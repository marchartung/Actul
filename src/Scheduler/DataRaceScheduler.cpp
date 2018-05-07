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
#include <Scheduler/ProcessInfo.h>
#include <Scheduler/DataraceScheduler.h>

#include <array>

namespace Actul
{

DataraceScheduler::~DataraceScheduler()
{

}

DataraceScheduler::DataraceScheduler(const TestSettings & settings)
      : _permTestPos(0),
        _settings(settings),
        _testDb(settings),
        _permState(_testDb, _settings),
        _interdependState(_testDb, _settings)
{

}
void DataraceScheduler::checkIfTestWasSuccess(const DataRaceShmVec& inDrs, const DataRaceShmVec& outDrs)
{
   bool found;
   for (size_type i = 0; i < inDrs.size(); ++i)
   {
      found = false;
      const DataRace & in = inDrs[i];
      for (size_type j = 0; j < outDrs.size(); ++j)
      {
         const DataRace & out = outDrs[j];
         if (in.isSameInstance(out))
         {
            found = true;
            if (in.order())
               found = in.isSameInstanceReversed(out);
            if (found)
               break;
         }
      }
      if (!found && inDrs.size() > 1 && _settings.debugOutput > 1)
         printStr("Warning: Tested data race was not correctly reordered.\n\n");
      if(found)
      {
         _testDb.setTested(in);
      }
   }
}

void DataraceScheduler::updateOn(TestReport& report)
{
   size_type numDataRaces = _testDb.numDataRaceCandidates();
   const auto & races = report.getDataRaces();
   const auto & replay = report.getReplayTokenArray();
   const auto & yields = report.getYieldEvents();

   AddRaceStructShmVec & addRaces = report.getAddRaceStruct();
   _testDb.addTestData(report.getTestInfo().hasViolation(), races, replay, yields, addRaces);
   checkIfTestWasSuccess(report.getTestDataRaces(), report.getDataRaces());
   if (numDataRaces < _testDb.numDataRaceCandidates() && _settings.testOnlyRandom)
      updateYields(numDataRaces);
}

const Vector<YieldEvent> & DataraceScheduler::getYieldEvents()
{
   if(_yieldEvents.empty() && _testDb.numDataRaceCandidates() > 0)
      updateYields(0);
   return _yieldEvents;
}

void DataraceScheduler::updateYields(const size_type & prevNumRaces)
{
   YieldEvent tmpYield;
   for (size_type i = prevNumRaces; i < _testDb.numDataRaceCandidates(); ++i)
   {
      for (size_type i = 0; i < 2; ++i)
      {
         tmpYield = YieldEvent(_testDb.getDataRace(i).getAccess(i),true);
         if (invalid(_yieldEvents.find(tmpYield)))
            _yieldEvents.push_back(tmpYield);
      }
   }
}

bool DataraceScheduler::hasTest()
{
   bool res = _permState.hasTest();
   if (!res)
   {
      res = _interdependState.hasTest();
   }
   return res;
   //return (_permState.hasTest() || _interdependState.hasTest());
}

const TestDatabase & DataraceScheduler::getTestDatabase() const
{
   return _testDb;
}

size_type DataraceScheduler::getNumNeededTests() const
{
   return _permState.numTests() + _interdependState.numTests() + 1;
}

void setDataRacesInReport(TestReport & report, const TestDatabase & testDb, const SizeVec & ids, const BoolVec & orders)
{
   size_type replayId = testDb.getReplayWith(ids);
   actulAssert(replayId != invalid<size_type>(), "DataraceScheduler: Test cannot be created. No suitable replay found");

   for (size_type i = 0; i < ids.size(); ++i)
   {
      const DataRace & d = testDb.getDataRace(ids[i], replayId);
      report.getTestDataRaces().insert(d);
      report.getTestDataRaces().back().setOrder(Xor(report.getTestDataRaces().back().order(), orders[i]));
   }
   report.getTestInfo().replayId = replayId;

}

template<typename TestSetter>
void setValidTest(TestReport& report, TestSetter & testSetter, const TestDatabase & testDb)
{
   SizeVec drIdsToTest;
   BoolVec drOrdersToTest;

   actulAssert(testSetter.hasTest(), "DataraceScheduler: Invalid test setter call");

   drIdsToTest = testSetter.getCurTestRaceIds();
   drOrdersToTest = testSetter.getCurTestRaceOrders();

   setDataRacesInReport(report, testDb, drIdsToTest, drOrdersToTest);
   actulAssert(report.getTestDataRaces().size() > 0, "");
   testSetter.next();
}

void DataraceScheduler::addDataRacesAsYields(TestReport& report)
{
   auto & reportYields = report.getYieldEvents();
   reportYields.resize(_yieldEvents.size());
   for(size_type i=0;i<_yieldEvents.size();++i)
   {
      reportYields[i] = _yieldEvents[i];
   }
}

void DataraceScheduler::setTestDataRacesAndReplay(TestReport& report)
{
   actulAssert(hasTest(), "DataraceScheduler: Invalid replay call");
   if (_permState.hasTest())
      setValidTest(report, _permState, _testDb);  // try to set a permutation test
   else if (_interdependState.hasTest())
      setValidTest(report, _interdependState, _testDb);  // try to set a interdependency  test
   else
      actulAssert(false, "DataraceScheduler: Could not set test and replay");
   hasTest();
}
} /* namespace Actul */
