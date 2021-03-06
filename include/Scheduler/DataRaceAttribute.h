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
#ifndef INCLUDE_SCHEDULER_DATARACEATTRIBUTE_H_
#define INCLUDE_SCHEDULER_DATARACEATTRIBUTE_H_

#include <Util/Container/Vector.h>
#include <Util/String.h>

namespace Actul
{


struct DataRaceAttribute
{
   bool isTested;
   bool falsePositive;
   bool harmful;
   bool introducing;
   bool violating;
   bool orderDependent;

   DataRaceAttribute();

   size_type getScore() const;

   static size_type getMaxScore();

   String getString(bool newClassification) const;

};
typedef Vector<DataRaceAttribute> DataRaceAttributeVec;

template<>
String to_string<DataRaceAttribute>(const DataRaceAttribute & drA);

} /* namespace Actul */

#endif /* INCLUDE_SCHEDULER_DATARACEATTRIBUTE_H_ */
