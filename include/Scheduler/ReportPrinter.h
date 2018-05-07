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
#ifndef INCLUDE_ANALYSE_PRINTREPORT_HPP_
#define INCLUDE_ANALYSE_PRINTREPORT_HPP_

#include <Scheduler/DataraceScheduler.h>
#include <Util/SymbolDb.h>

namespace Actul
{

class ReportPrinter
{
 public:
   ReportPrinter(const TestDatabase & td, const TestSettings & settings);

   void printBrief();

   void print();

   void printClassificationDistributionHead();

   void printClassificationDistribution();

   static void printStack(const StackArray & s)
   {
      String res("Stack:\n");
      SymbolDb symDb;
      for (size_type i = 0; i < s.size(); ++i)
         if (s[i] != nullptr)
         {
            AddressType tmp = s[i];
            FunctionSymbol fs = symDb.getFunctionSymbol(tmp);
            res += "#" + to_string(i) + " " + fs.info + "\n";
         } else
            break;
      printStr(res + "\n");
   }

 private:
   const TestSettings & _settings;
   const TestDatabase & _tb;
   mutable SymbolDb _symDb;

   DataRaceAttributeVec getAttributes() const;

   SizeVec getToPrintRaces(const DataRaceAttributeVec & attrs) const;

   void getSimilarDataRaces(SizeVecVec & res, const DataRaceAttributeVec & attrs) const;

   void sortDataRaces(SizeVec & v, const DataRaceAttributeVec & attributes) const;

   void printDataRace(const DataRaceAttributeVec & attrs, const SizeVec & similar);

   String getStackStr(const MemoryAccess & s) const;
}
;

} /* namespace Actul */

#endif /* INCLUDE_ANALYSE_PRINTREPORT_HPP_ */
