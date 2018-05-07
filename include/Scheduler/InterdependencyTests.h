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
#ifndef INCLUDE_UTIL_INTERDEPENDENCYTESTS_H_
#define INCLUDE_UTIL_INTERDEPENDENCYTESTS_H_

#include <Util/Defines.h>
#include <Util/CycleFinder.h>
#include <Scheduler/TestDatabase.h>

namespace Actul
{

class InterdependencyTests
   {
    public:

      InterdependencyTests(const TestDatabase & tdb, const TestSettings & settings);

      SizeVec getCurTestRaceIds() const;

      const BoolVec & getCurTestRaceOrders() const;

      bool hasTest();

      bool next();

      size_type numTests() const;

    private:
      const TestSettings & _settings;
      const TestDatabase & _tdb;
      bool _currentlyNotValidated;
      CycleFinder _cycleFinder;
      BoolVec _curPermutation;
      SizeVec _curTestRaces;

      bool tryPermuteOrder();

      bool trySetNextRaces();

      bool isValidOrder() const;

      void validate();

      bool hasTest_unsafe() const;

      bool isSenseLess() const;
};

} /* namespace Actul */

#endif /* INCLUDE_UTIL_INTERDEPENDENCYTESTS_H_ */
