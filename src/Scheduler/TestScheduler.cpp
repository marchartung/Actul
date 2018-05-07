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
#include <Scheduler/ReportPrinter.h>
#include <Scheduler/TestScheduler.h>
#include <Util/VectorHelper.h>

#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <Util/DebugRoutines.h>
#include <Util/EnvironmentDependencies.h>
#include <Util/String.h>

#include <Implementation/PthreadImpl.h>
namespace Actul
{
template<>
TestScheduler * SingletonClass<TestScheduler>::_instance = nullptr;

TestScheduler::TestScheduler()
      : _lastTestMode('r'),
        _maxNumPossibleSchedules(0),
        _numDoneTests(0),
        _numStartedTests(0),
        _numCrashes(0),
        _currentWorkerId(0),
        _settings(ProcessInfo::getSettings()),
        _startSeed(_settings.seedOffset),
        _yieLogger(*YieldLogger::getInstance()),
        _conLogger(_settings),
        _drLogger(),
        _thrScheduler(_settings, _conLogger),
        _drScheduler(_settings),
        _shmManager(_shmManager->getInstance()),
        _progressCounters(_settings.maxNumWorkers),
        _progressLookups(_settings.maxNumWorkers)
{
   ThreadInfo::setThreadId(0);
   for (size_type i = 0; i < _settings.maxNumWorkers; ++i)
   {
      _pids.push_back(invalid<int>());
      _freeWorkerIds.push_back(i);
      _testReports.push_back(newInstance<TestReport>());
   }

   if (_settings.debugOutput > 0)
   {
      printCurrentDataRaceClassificationHead();
   }
   start();
}

TestScheduler::~TestScheduler()
{
   deinitialize();
}

void TestScheduler::printResults()
{
   ReportPrinter printer(_drScheduler.getTestDatabase(),_settings);
   if (_settings.verbosity > 0)
      printer.print();
   printer.printBrief();

   printStr("crashes/done/max tests: " + to_string(_numCrashes) + "/" + to_string(_numDoneTests) + "/" + to_string(_settings.maxNumTests) + "\n");
   printStr("time: " + to_string(ProcessInfo::getRuntime()) + "\n");
   printStr("Actul test environment finishes\n");
   if (_settings.debugOutput > 1)
      _pc.print();
}

void TestScheduler::printBrief() const
{
   printStr("Test scheduler current stats:\n");
   printStr("Num done tests: " + to_string(_numDoneTests) + " (crashes: " + to_string(_numCrashes) + ")\n");
   printStr("Memory: " + to_string(mBytesAllocated()) + "MB\n");
   printStr("Num neeeded tests: " + to_string(_drScheduler.getNumNeededTests()) + "\n");
   printStr("Num race candidates: " + to_string(_drScheduler.getTestDatabase().numDataRaceCandidates()) + "\n");
   printStr("Num races: " + to_string(_drScheduler.getTestDatabase().numDataRaces()) + "\n\n");
}

void TestScheduler::printRunningStates() const
{
   printStr("#Tests: " + to_string(_numDoneTests) + " (#crashing: " + to_string(_numCrashes) + ") Memory: " + to_string(mBytesAllocated()) + "MB ");
   printStr("#DataRaceCandidates: " + to_string(_drScheduler.getTestDatabase().numDataRaceCandidates()) + "\n");
}

void TestScheduler::printCurrentDataRaceClassificationHead() const
{
   ReportPrinter printer(_drScheduler.getTestDatabase(),_settings);
   printStr("CURclassMarker,numTestsEstimated,doneTests,numCrashes,numCandidates,numRaces,time,memory,testMode,");
   printer.printClassificationDistributionHead();
   printStr("\n");
}

void TestScheduler::printCurrentDataRaceClassification() const
{
   const TestDatabase & db = _drScheduler.getTestDatabase();
   ReportPrinter printer(db,_settings);
   printStr(
         "CURclassMarker," + to_string(_drScheduler.getNumNeededTests()) + "," + to_string(_numDoneTests) + "," + to_string(_numCrashes) + ","
               + to_string(db.numDataRaceCandidates()) + "," + to_string(db.numDataRaces()) + "," + to_string(ProcessInfo::getRuntime()) + "," + to_string(mBytesAllocated()) + ","
               + to_string(_lastTestMode) + ",");
   printer.printClassificationDistribution();
   printStr("\n");
}

void TestScheduler::deinitialize()
{
   if (!ProcessInfo::isFork())
   {
      printResults();
      _shmManager->destroyAll();
   } else
   {
      for (size_type i = 0; i < ProcessInfo::getTestInfo().isInScheduler.size(); ++i)
         ProcessInfo::getTestInfo().isInScheduler[i] = false;
      _drScheduler.getTestDatabase().getAddedDataRaces(ProcessInfo::getTestReport().getAddRaceStruct(), ProcessInfo::getTestReport().getDataRaces());
      ProcessInfo::getTestInfo().runtime = ProcessInfo::getRuntime();
      _shmManager->detachAll();
   }
}

ConstrainLogger & TestScheduler::getConstrainLogger()
{
   return _conLogger;
}

DataRaceLogger & TestScheduler::getDataRaceLogger()
{
   return _drLogger;
}

ThreadScheduler & TestScheduler::getThreadScheduler()
{
   return _thrScheduler;
}

void TestScheduler::start()
{
   // first run, check for num workers and first data races

   printStr("Running Actul test environment\n");
   runDataRaceTesting();
   if (!ProcessInfo::isFork())
   {
      InternalExit(0);
   }

}

bool TestScheduler::tryCleanUpWorker()
{
   _pc.start(_pc.cleanUpWorker);
   bool res = hasRunningWorkers();
   if (res)
   {
      size_type curWorkerId = getFinishedWorkerId();
      if (curWorkerId != invalid<size_type>())
      {
         cleanUpTest(curWorkerId);
         _freeWorkerIds.push_back(curWorkerId);
         if(_settings.testOnlyRandom && _drScheduler.getTestDatabase().numDataRaceCandidates() > 0)
            _settings.minNumTests = Max(_drScheduler.getTestDatabase().numDataRaceCandidates()*_drScheduler.getTestDatabase().numDataRaceCandidates(),_settings.minNumTests);
      } else
         res = false;
   }
   _pc.end(_pc.cleanUpWorker);
   return res;
}

size_type TestScheduler::findWorkerId(const pid_t & pid)
{
   size_type res = invalid<size_type>();
   for (size_type i = 0; i < _pids.size(); ++i)
      if (_pids[i] == pid)
      {
         res = i;
         break;
      }
   return res;
}

size_type TestScheduler::getFinishedWorkerId()
{

   int retPid, statusDetailed;
   size_type res = invalid<size_type>();
   for (size_type i = 0; i < _pids.size(); ++i)
   {
      if (_pids[i] != invalid<int>())
      {
         retPid = InternalWaitpid(_pids[i], &statusDetailed, WNOHANG);
         if (retPid > 0)
         {
            actulAssert(_pids[i] == retPid, "TestScheduler: Test wait was invalid");
            res = findWorkerId(retPid);
            //printStr("RetPid:" + to_string(retPid) + " => worker id " + to_string(res) + "\n");
            actulAssert(res != invalid<size_type>(), "TestScheduler: Wait for fork failed.");
            _testReports[res]->reattach();
            if (WIFSIGNALED(statusDetailed))
            {
               _testReports[res]->getTestInfo().returnValue = WTERMSIG(statusDetailed);
               //printStr("Signal: " + to_string(WTERMSIG(statusDetailed)) + " ");
            } else if (WIFEXITED(statusDetailed))
            {
               _testReports[res]->getTestInfo().returnValue = WEXITSTATUS(statusDetailed);
               //printStr("Return Val: " + to_string(WEXITSTATUS(statusDetailed)) + "\n");
            }

            _pids[res] = invalid<int>();
            _progressCounters[i] = 0;
            _progressLookups[i] = 0;
            break;
         }
      }
   }
   return res;
}

void TestScheduler::checkSlaveProgress(const size_type & workerId)
{
   actulAssert(_pids[workerId] != invalid<int>(), "TestScheduler: Cannot check progress of not running slave");
   if (_testReports[workerId]->getTestInfo().getProgressCounter() != _progressCounters[workerId])
   {
      _progressCounters[workerId] = _testReports[workerId]->getTestInfo().getProgressCounter();
      _progressLookups[workerId] = 0;
   } else if (_progressLookups[workerId] > _settings.maxIdleLoops)
   {
      printStr("Warning: Killing worker " + to_string(workerId) + " due to inactivity\n");
      killWorker(workerId);
      _progressLookups[workerId] = 0;
   } else
      ++_progressLookups[workerId];
}

void TestScheduler::checkSlaveProgress()
{
   for (size_type i = 0; i < _settings.maxNumWorkers; ++i)
      if (isValid(_pids[i]))
         checkSlaveProgress(i);
}

void TestScheduler::cleanUpTest(const size_type& workerId)
{
   _testReports[workerId]->reattach();
   if (_settings.debugOutput > 1)
      _testReports[workerId]->print();
   actulAssert(_testReports[workerId]->getTestInfo().returnValue != 207, "TestScheduler: Internal error. Shutting down");
   const BoolArray & bTest = _testReports[workerId]->getTestInfo().isInScheduler;
   actulAssert(bTest == BoolArray(false), "TestScheduler: Actul test crashed in RTL");
   if (_testReports[workerId]->getTestInfo().hasViolation())
      ++_numCrashes;
   if(_testReports[workerId]->getTestDataRaces().size() == 0)
      _lastTestMode = 'r';
   else if(_testReports[workerId]->getTestDataRaces().size() == 1)
      _lastTestMode = 'p';
   else
      _lastTestMode = 'i';
   //_testReports[workerId]->print();
   _pc.start(_pc.updateOn);
   _drScheduler.updateOn(*_testReports[workerId]);
   _pc.end(_pc.updateOn);
   _testReports[workerId]->clear();
   _testReports[workerId]->check();
   ++_numDoneTests;
   if (_settings.debugOutput > 0)
      printCurrentDataRaceClassification();
   if (_settings.debugOutput > 1)
      printBrief();
}

bool TestScheduler::hasRunningWorkers() const
{
   return _freeWorkerIds.size() < _settings.maxNumWorkers;
}

bool TestScheduler::tryStartNewTest()
{
   _pc.start(_pc.startWorker);
   bool res = _freeWorkerIds.size() > 0;
   if (res)
   {
      size_type workerId = _freeWorkerIds.back();
      _testReports[workerId]->reattach();
      bool fetchTest = false;
      if (((_drScheduler.hasTest() && !_settings.testOnlyRandom) || _numStartedTests < _settings.minNumTests) && _numStartedTests < _settings.maxNumTests)
      {
         if (_drScheduler.hasTest() && !_settings.testOnlyRandom)
            fetchTest = true;
         else
            ++_startSeed;
         _freeWorkerIds.pop_back();
         res = startTestWorker(workerId, fetchTest);
      } else
         res = false;
   }
   _pc.end(_pc.startWorker);
   return res;
}

bool TestScheduler::hasResourceViolation()
{
   _pc.start(_pc.resourceVio);
   bool res = false;
   size_type mem;
   if (ProcessInfo::getRuntime() >= _settings.maxSecondsRuntime || (mem = mBytesAllocated()) > _settings.maxMemoryMB)
   {
      res = true;
      printStr("Abort testing: \nruntime: " + to_string(ProcessInfo::getRuntime()) + "s (allowed " + to_string(_settings.maxSecondsRuntime) + ")\n");
      printStr("memory: " + to_string(mem) + "MB (allowed " + to_string(_settings.maxMemoryMB) + ")\n");
      abortTesting();
   }
   _pc.end(_pc.resourceVio);
   return res;
}

bool TestScheduler::runDataRaceTesting()
{
   bool workToDo = true, shouldIdl;
   double lastPrintTime;
   actulAssert(_freeWorkerIds.size() > 0, "TestScheduler: No worker allocation possible.");

   while (workToDo)
   {
      if (!(workToDo = tryStartNewTest()))  // start new test first
      {
         if (ProcessInfo::isFork())  // worker return from tryStartNewTest, than loop has to be left
            break;
         if (!(workToDo = tryCleanUpWorker()))  // try clean up when there is no test to start
            if ((workToDo = hasRunningWorkers()))  // check if master has to wait for tests or can finish
               shouldIdl = true;  // idle since there is nothing to do except waiting
      }

      workToDo &= !hasResourceViolation();

      if (shouldIdl)
      {
         if (_settings.numIdlMicroSeconds > 0)
         {
            _pc.start(_pc.idleWorker);
            shouldIdl = false;
            checkSlaveProgress();
            usleep(_settings.numIdlMicroSeconds);
            _pc.end(_pc.idleWorker);
         }
      }
      if (ProcessInfo::getRuntime() - lastPrintTime > _settings.printInterval)
      {
         lastPrintTime = ProcessInfo::getRuntime();
         printRunningStates();
      }
   }
   return workToDo;
}

void TestScheduler::killWorker(const size_type & workerId)
{
   actulAssert(_pids[workerId] != invalid<int>(), "Cannot kill not running worker");
   kill(_pids[workerId], SIGKILL);
}

void TestScheduler::abortTesting()
{
   for (size_type i = 0; i < _pids.size(); ++i)
      if (_pids[i] != invalid<int>())
         killWorker(i);
}

bool TestScheduler::initializeWorker(const size_type & workerId)
{
   _thrScheduler.setParameters(workerId, _startSeed + _settings.seedOffset, *_testReports[workerId]);
   return true;
}

bool TestScheduler::startTestWorker(const size_type & workerId, const bool & fetchTest)
{
   bool res = false;
   actulAssert(workerId < _settings.maxNumWorkers, "TestScheduler: Couldn't start worker.");
   _testReports[workerId]->check();
   if (fetchTest)
   {
      _drScheduler.setTestDataRacesAndReplay(*_testReports[workerId]);
      ScheduleCreator::createSchedule(_testReports[workerId]->getReplayTokenArray(), _drScheduler.getTestDatabase().getReplay(_testReports[workerId]->getTestInfo().replayId),
                                      _testReports[workerId]->getTestDataRaces());
   }
   else // random scheduling needs as many yields as possible, so add all data races
      _drScheduler.addDataRacesAsYields(*_testReports[workerId]);
   _currentWorkerId = workerId;
   ++_numStartedTests;
   fflush(stdout);

   _pids[workerId] = InternalFork();
   if (_pids[workerId] != 0)
   {
      res = true;
   } else
   {
      initializeWorker(workerId);
   }

   return res;
}

size_type TestScheduler::mBytesAllocated() const
{
   size_type used = 0;
   for (size_type i = 0; i < _settings.maxNumWorkers; ++i)
   {
      if (_pids[i] != invalid<int>())
         used += numAllocatedMB(_pids[i]);
   }
   used += numAllocatedMB(getpid());
   used += (_shmManager->bytesAllocated() / (1024 * 1024)) + 1;
//printStr("MB: " + to_string(used) + "\n");
   return used;
}

} /* namespace Actul */
