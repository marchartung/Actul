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
#ifndef INCLUDE_SCHEDULER_PERMUTATIONTESTS_H_
#define INCLUDE_SCHEDULER_PERMUTATIONTESTS_H_

#include <Util/Defines.h>
#include <Scheduler/TestDatabase.h>

namespace Actul
{

class PermutationTests
{
 public:
   PermutationTests(const TestDatabase & tdb, const TestSettings & settings);

   bool hasTest() const;

   const SizeVec & getCurTestRaceIds() const;

   const BoolVec & getCurTestRaceOrders() const;

   void next();

   size_type numTests() const;

 private:
   const TestDatabase & _tdb;
   const TestSettings & _settings;
   SizeVec _curDr;
   const BoolVec _orders;
};

} /* namespace Actul */

#endif /* INCLUDE_SCHEDULER_PERMUTATIONTESTS_H_ */
