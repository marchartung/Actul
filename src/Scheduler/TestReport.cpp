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
#include <Scheduler/TestReport.h>
#include <Util/Container/SHM/SHMManager.h>

namespace Actul
{
/**
 *
 SHMManager _manager;
 EpochTrace _trace;
 SHMArray<PossibleRace> * _dataRaces;
 SHMArray<DeadLockThreadInfo> * _deadlockInfos;
 SHMAtomicCounter * _isRunning;
 */

TestReport::TestReport()
      : _manager(SHMManager::getInstance()),
        _testInfo(_manager->getSHMWrapper<TestInfo>()),
        _dataRaces(_manager->getSHMArray<DataRace>()),
        _testDataRaces(_manager->getSHMArray<DataRace>()),
        _doneYields(_manager->getSHMArray<YieldEvent>()),
        _replay(_manager->getSHMArray<ReplayToken>()),
        _addRaces(_manager->getSHMArray<AddRaceStruct>())
{
   check();
}

TestReport::~TestReport()
{
}

void TestReport::check()
{
   actulAssert(_dataRaces->capacity() > 0, "TestReport: Check on _dataRaces failed");
   actulAssert(_testDataRaces->capacity() > 0, "TestReport: Check _testDataRaces failed");
   actulAssert(_doneYields->capacity() > 0, "TestReport: Check _doneYields failed");
   actulAssert(_replay->capacity() > 0, "TestReport: Check _replay failed");
   actulAssert(_addRaces->capacity() > 0, "TestReport: Check _addRaces failed");
}

TestInfo & TestReport::getTestInfo()
{
   return _testInfo->getElem();
}

const TestInfo & TestReport::getTestInfo() const
{
   return _testInfo->getElem();
}

bool TestReport::isFinished() const
{
   return !_testInfo->getElem().isRunning;
}

bool TestReport::hasDataRace() const
{
   return _dataRaces->size() > 0;
}

const SHMArray<DataRace>& TestReport::getDataRaces() const
{
   return *_dataRaces;
}

SHMArray<DataRace>& TestReport::getDataRaces()
{
   return *_dataRaces;
}

const SHMArray<DataRace>& TestReport::getTestDataRaces() const
{
   return *_testDataRaces;
}

SHMArray<DataRace>& TestReport::getTestDataRaces()
{
   return *_testDataRaces;
}

const SHMArray<ReplayToken> & TestReport::getReplayTokenArray() const
{
   return *_replay;
}

SHMArray<ReplayToken> & TestReport::getReplayTokenArray()
{
   return *_replay;
}

YieldEventShmVec & TestReport::getYieldEvents()
{
   return *_doneYields;
}

const YieldEventShmVec & TestReport::getYieldEvents() const
{
   return *_doneYields;
}

AddRaceStructShmVec& Actul::TestReport::getAddRaceStruct()
{
   return *_addRaces;
}

const AddRaceStructShmVec& Actul::TestReport::getAddRaceStruct() const
{
   return *_addRaces;
}

void TestReport::clear()
{
   _dataRaces->clear();
   _testDataRaces->clear();
   _replay->clear();
   _doneYields->clear();
   _addRaces->clear();
   getTestInfo() = TestInfo();
}

void TestReport::print()
{
   printStr("TestReport:\n");
   const TestInfo & info = getTestInfo();
   if (info.returnValue != 207)
   {
      if (info.hasViolation())
         printStr("Test had violation\n");
      else
         printStr("Test had no violation\n");
      printStr("Time: " + to_string(info.runtime) + "\n");
      printStr("Num r-tokens: " + to_string(getReplayTokenArray().size()) + "\n");
      printStr("Num races found: " + to_string(getDataRaces().size()) + "\n");
      printStr("Num races used: " + to_string(getTestDataRaces().size()) + "\n");
      printStr("Num yields used: " + to_string(getYieldEvents().size()) + "\n");
   } else
      printStr("Test failed\n");
}

void TestReport::reattach()
{
   _manager->detachObservables();
   _manager->attachObservables();
}
} /* namespace Actul */
