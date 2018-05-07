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
//    Marc Hartung
////////////////////////////////////////////////////////////////////////////////
#include <Scheduler/DataRaceAttribute.h>

namespace Actul
{

template<>
String to_string<DataRaceAttribute>(const DataRaceAttribute & drA)
{
   String res("");
   if (drA.falsePositive)
      res += "false positive ";
   else
   {
      res = to_string(((drA.harmful) ? "harmful " : "")) + to_string(((drA.introducing) ? "introducing " : "")) + to_string(((drA.violating) ? "violating " : ""))
            + to_string(((drA.orderDependent) ? "order dependent " : ""));
      //res += (drA.introducing) ? "introducing " : "";
      //res += (drA.violating) ? "violating " : "";
      //res += (drA.orderDependent) ? "order dependent " : "";
   }
   return res;
}

String DataRaceAttribute::getString(bool newClassification) const
{
   String res("");
   if (newClassification)
   {
      if (harmful)
         res = "harmful ";
      else if (violating && orderDependent)
         res = "violating ";
      else if(orderDependent)
         res = "order dependent ";
      else if(!falsePositive)
         res = "benign ";
      else if(isTested)
         res = "false positive ";
      else
         res = "not tested ";

   } else
      res = to_string(*this);
   return res;
}

DataRaceAttribute::DataRaceAttribute()
      : isTested(false),
        falsePositive(true),
        harmful(false),
        introducing(false),
        violating(false),
        orderDependent(false)
{

}

size_type DataRaceAttribute::getScore() const
{
   size_type res = 0;
   if (!falsePositive)
   {
      res += (harmful) ? 16 : 0;
      res += (introducing) ? 8 : 0;
      res += (violating) ? 4 : 0;
      res += (orderDependent) ? 2 : 0;
      res += (!falsePositive) ? 1 : 0;  //i.e. permutable
   }
   return res;
}

size_type DataRaceAttribute::getMaxScore()
{
   DataRaceAttribute attr;
   attr.falsePositive = false;
   attr.harmful = true;
   attr.introducing = true;
   attr.violating = true;
   attr.orderDependent = true;
   return attr.getScore();
}

} /* namespace Actul */
