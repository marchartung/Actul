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
#ifndef INCLUDE_UTIL_CYCLEFINDER_H_
#define INCLUDE_UTIL_CYCLEFINDER_H_

#include <Util/Defines.h>
#include <Scheduler/TestDatabase.h>

namespace Actul
{

class CycleFinder
{
 public:

   CycleFinder(const TestDatabase & tDb);

   bool hasCycles(const DataRaceVec & vec) const;

   bool hasCylces(const SizeVec & ids, const BoolVec & orders) const;

 private:
   const TestDatabase & _tDb;
};

}
#endif /* INCLUDE_UTIL_CYCLEFINDER_H_ */
