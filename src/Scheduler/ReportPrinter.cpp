////////////////////////////////////////////////////////////////////////////////
// Copyright 2017-2018 Zuse Institute Berlin
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
// 
//   http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations under
// the License.
// 
// Contributors:
// 	Marc Hartung
////////////////////////////////////////////////////////////////////////////////
#include <Scheduler/ReportPrinter.h>

namespace Actul
{

ReportPrinter::ReportPrinter(const TestDatabase& td, const TestSettings & settings)
      : _settings(settings),
        _tb(td)
{
}

size_type getClassNum(const DataRaceAttribute & attr)
{
   size_type res = 0;
   if (attr.harmful)
      res = 5;  // harmful
   else if (attr.violating && attr.orderDependent)
      res = 4;  // violating
   else if (attr.orderDependent)
      res = 3;  // order dep
   else if (!attr.falsePositive)
      res = 2;  // benign
   else if (attr.isTested)
      res = 1;  // false positive
   else
      res = 0;  // not tested
   return res;
}

void ReportPrinter::sortDataRaces(SizeVec & v, const DataRaceAttributeVec & attributes) const
{
   size_type debugSz = v.size();
   SizeVecVec tmp(DataRaceAttribute::getMaxScore(), SizeVec());
   size_type idx;
   for (size_type i = 0; i < v.size(); ++i)
   {
      idx = attributes[v[i]].getScore();
      tmp[idx].push_back(v[i]);
   }
   v.clear();
   for (size_type i = 0; i < tmp.size(); ++i)
   {
      for (size_type j = 0; j < tmp[i].size(); ++j)
         v.push_back(tmp[i][j]);
   }
   actulAssert(debugSz == v.size(), "ReportPrinter: function sortDataRaces is inconsistent");
}

void ReportPrinter::printBrief()
{
   DataRaceAttributeVec attributes = getAttributes();
   SizeVecVec similar;
   getSimilarDataRaces(similar, attributes);
   SizeVec realRaceClass(similar.size(),0);
   for(size_type i=0;i<similar.size();++i)
   {
      for(size_type j=0;j<similar[i].size();++j)
         if(getClassNum(attributes[similar[i][j]]) > realRaceClass[i])
            realRaceClass[i] = getClassNum(attributes[similar[i][j]]);
   }
   SizeVec numClasses(6,0);
   for(size_type i=0;i<realRaceClass.size();++i)
   {
      actulAssert(realRaceClass[i] < numClasses.size(), "ReportPrinter: classes are inconsistent");
      ++numClasses[realRaceClass[i]];
   }

   printStr("COLLAPSED: not_tested: " + to_string(numClasses[0]) + "\n");
   printStr("COLLAPSED: false_pos : " + to_string(numClasses[1]) + "\n");
   printStr("COLLAPSED: benign    : " + to_string(numClasses[2]) + "\n");
   printStr("COLLAPSED: order_dep : " + to_string(numClasses[3]) + "\n");
   printStr("COLLAPSED: violate   : " + to_string(numClasses[4]) + "\n");
   printStr("COLLAPSED: harmful   : " + to_string(numClasses[5]) + "\n");
}

void ReportPrinter::print()
{
   DataRaceAttributeVec attributes = getAttributes();
   SizeVecVec similars;
   getSimilarDataRaces(similars, attributes);
   for (size_type i = 0; i < similars.size(); ++i)
   {
      printDataRace(attributes, similars[i]);
   }
}

void ReportPrinter::printClassificationDistributionHead()
{
   if (_settings.useNewClassification)
      printStr("not_tested, false_positive, benign, order-dependent, violating, harmful");
   else
   {
      for (size_type i = 0; i < DataRaceAttribute::getMaxScore() + 1; ++i)
      {
         if (i > 0)
            printStr(",");
         printStr("s" + to_string(i));
      }
   }
}

void ReportPrinter::printClassificationDistribution()
{
   DataRaceAttributeVec attributes = getAttributes();
   if (_settings.useNewClassification)
   {
      SizeVec numInClass(6, 0);
      for (size_type i = 0; i < attributes.size(); ++i)
         ++numInClass[getClassNum(attributes[i])];
      for (size_type i = 0; i < numInClass.size(); ++i)
      {
         if (i > 0)
            printStr(",");
         printStr(to_string(numInClass[i]));
      }
   } else
   {
      SizeVec classes(DataRaceAttribute::getMaxScore() + 1);
      for (size_type i = 0; i < attributes.size(); ++i)
         ++classes[attributes[i].getScore()];
      SizeVec overallScore(classes.size(), 0);
      for (size_type i = 0; i < classes.size(); ++i)
      {
         if (i > 0)
            printStr(",");
         //printStr("ScoreIdx" + to_string(i) + ":" + to_string(classes[i].size()) + "\n");
         printStr(to_string(classes[i]));
      }
   }
}

String ReportPrinter::getStackStr(const MemoryAccess & s) const
{
   VariableSymbol varSym = _symDb.getVariableSymbol(s.addr);
   String type = (s.isWrite) ? String("write") : String("read");
   String atomic = (s.isAtomic) ? String("atomic ") : String("");
   String res = String("Stack from variable ") + varSym.info + " (" + atomic + type + " on " + to_string(s.addr) + " width:" + to_string(s.width) + "):\n";
   for (size_type i = 0; i < s.stack.size(); ++i)
      if (s.stack[i] != nullptr)
      {
         AddressType tmp = s.stack[i];
         FunctionSymbol fs = _symDb.getFunctionSymbol(tmp);
         res += " #" + to_string(i) + " " + fs.info + "(" + to_string(const_cast<void*>(tmp)) + String(")\n");
      } else
         break;
   return std::move(res);
}

void ReportPrinter::printDataRace(const DataRaceAttributeVec & attrs, const SizeVec & similar)
{
   size_type id = similar[0];
   size_type minScore = attrs[id].getScore(), maxScore = minScore;
   for (size_type i = 1; i < similar.size(); ++i)
   {
      size_type tmp = attrs[similar[i]].getScore();
      if (tmp < minScore)
         minScore = tmp;
      if (tmp > maxScore)
      {
         maxScore = tmp;
         id = similar[i];
      }
   }
   const DataRace & d = _tb.getDataRace(id);
   String attrStr = attrs[id].getString(_settings.useNewClassification);
   String stackStr1 = getStackStr(d.getAccess(0));
   String stackStr2 = getStackStr(d.getAccess(1));
   String res = attrStr + String("data race detected (maxScore:") + to_string(maxScore) + String(",minScore:") + to_string(minScore) + ") " + to_string(similar.size())
         + " times on:\n" + stackStr1 + stackStr2 + String("\n");
   printStr(res);
}

SizeVec ReportPrinter::getToPrintRaces(const DataRaceAttributeVec & attrs) const
{
   SizeVec res;
   BoolVec took(attrs.size(), false);
   for (size_type i = 0; i < _tb.numDataRaceCandidates(); ++i)
   {
      if (!took[i])
      {
         took[i] = true;
         size_type to_take = i;
         for (size_type j = i + 1; j < _tb.numDataRaceCandidates(); ++j)
         {
            if (_tb.getDataRace(i).isSameCodeAccess(_tb.getDataRace(j)))
            {
               took[j] = true;
               if (attrs[j].getScore() > attrs[to_take].getScore())
                  to_take = j;
            }
         }
         res.push_back(to_take);
      }
   }
   return res;
}

void ReportPrinter::getSimilarDataRaces(SizeVecVec & res, const DataRaceAttributeVec & attrs) const
{
   BoolVec took(attrs.size(), false);
   for (size_type i = 0; i < _tb.numDataRaceCandidates(); ++i)
   {
      if (!took[i])
      {
         took[i] = true;
         res.push_back(SizeVec(1, i));
         for (size_type j = i + 1; j < _tb.numDataRaceCandidates(); ++j)
         {
            if (!took[j])
            {
               const DataRace & a = _tb.getDataRace(i), &b = _tb.getDataRace(j);
               if (a.isSameCodeAccess(b))
               {
                  took[j] = true;
                  res.back().push_back(j);
               }
            }
         }
      }
   }
}
DataRaceAttributeVec ReportPrinter::getAttributes() const
{
   DataRaceAttributeVec res;
   res.unsafe_resize(_tb.numDataRaceCandidates());
   for (size_type i = 0; i < res.size(); ++i)
   {
      res[i] = _tb.getAttribute(i);
   }
   return res;
}

/*
 void ReportPrinter::printDataraces(const CategoryDataraceVec & dataraces, const DataraceCategory& showBriefly, DataraceCategory& showCompletely)
 {
 for (short i = showBriefly; i <= showCompletely; ++i)
 {
 for (size_type j = 0; j < dataraces[i].size(); ++j)
 printDataraceBriefly(dataraces[i][j]);
 }

 for (short i = showCompletely; i < DataraceCategory::ENDMARKER_DATARACECATEGORY; ++i)
 {
 for (size_type j = 0; j < dataraces[i].size(); ++j)
 for (size_type j = 0; j < dataraces[i].size(); ++j)
 printDataraceCompletely(dataraces[i][j]);
 }
 }

 void ReportPrinter::printDataraceBriefly(const AnalysedDatarace& d)
 {
 printDataraceHead(d);
 printDataraceThreadInfo(d, 0);
 printStack(d.d, 0);  // TODO remove
 printDataraceThreadInfo(d, 1);
 printStack(d.d, 1);  // TODO remove
 }

 void ReportPrinter::printDataraceCompletely(const AnalysedDatarace& d)
 {
 printDataraceHead(d);
 printDataraceThreadInfo(d, 0);
 printStack(d.d, 0);
 printDataraceThreadInfo(d, 1);
 printStack(d.d, 1);
 }

 void ReportPrinter::printStack(const DataRace & d, const size_type & n)
 {
 const size_type num = sizeof(d.stack[n]) / sizeof(d.stack[n][0]);
 size_type i = 0;
 for (; i < num; ++i)
 if (d.stack[n][i] != 0)
 printStr("#" + to_string(i) + " " + SymbolDb::getInstance()->getFunctionSymbol(d.stack[n][i]).info + "\n");
 else
 break;
 if (i == 0)
 printStr("#  [No function stack available]\n");
 }

 string writeReadStr(const bool & isWrite, const bool & isAtomic)
 {
 string res("");
 if (isAtomic)
 res += "atomic ";
 if (isWrite)
 res += "write";
 else
 res += "read";
 return res;
 }

 double getCrashRate(const AnalysedDatarace & d)
 {
 double res = 0;
 res += d.stat.numFatalInOrder[0] + d.stat.numFatalInOrder[1];
 res /= d.stat.numAppearances;
 return res * 100.0;
 }

 void ReportPrinter::printDataraceHead(const AnalysedDatarace & d)
 {
 VariableSymbol varSym = SymbolDb::getInstance()->getVariableSymbol(d.d.addr[0]);
 string varName = varSym.info;
 if(varName == "")
 varName = to_string(reinterpret_cast<const void *>(d.d.addr[0]));
 else
 varName = " variable '" + varName + "'";
 printStr(
 "\n" + to_string(d.stat.getCategory()) + " data race on " + varName + " on threads " + to_string(d.d.tid[0]) + "("
 + writeReadStr(d.d.isWrite[0], d.d.isAtomic[0]) + ") and " + to_string(d.d.tid[1]) + "(" + writeReadStr(d.d.isWrite[1], d.d.isAtomic[1]) + ") appearances: "
 + to_string(d.stat.numAppearances) + " crash rate: " + to_string(getCrashRate(d)) + "%\n");
 }

 void ReportPrinter::printDataraceThreadInfo(const AnalysedDatarace & d, const size_type &n)
 {
 printStr("Function stack from thread " + to_string(d.d.tid[n]) + "\n");
 }
 */
} /* namespace Actul */
