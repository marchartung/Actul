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
#ifndef INCLUDE_STATICRACESCHEDULER_HPP_
#define INCLUDE_STATICRACESCHEDULER_HPP_

#include <Util/Defines.h>
#include <Logger/DataRace.h>
#include <Scheduler/DataRaceAttribute.h>
#include <Scheduler/PermutationTests.h>
#include <Scheduler/InterdependencyTests.h>
#include <Scheduler/TestReport.h>
#include <Util/TypeDefs.h>

namespace Actul
{

class DataraceScheduler
{

 public:

   DataraceScheduler(const TestSettings & settings);

   virtual ~DataraceScheduler();

   void updateOn(TestReport & report);

   bool hasTest();

   void addDataRacesAsYields(TestReport& report);

   void setTestDataRacesAndReplay(TestReport & report);

   const TestDatabase & getTestDatabase() const;

   size_type getNumNeededTests() const;

   const Vector<YieldEvent> & getYieldEvents();

 private:
   size_type _permTestPos;
   const TestSettings & _settings;
   TestDatabase _testDb;
   PermutationTests _permState;
   InterdependencyTests _interdependState;
   Vector<YieldEvent> _yieldEvents;

   DataRaceAttributeVec && getDataRaceAttributes() const;
   void checkIfTestWasSuccess( const DataRaceShmVec& inDrs,  const DataRaceShmVec& outDrs);

   void updateYields(const size_type & prevNumRaces);
};

} /* namespace Actul */

#endif /* INCLUDE_STATICRACESCHEDULER_HPP_ */
