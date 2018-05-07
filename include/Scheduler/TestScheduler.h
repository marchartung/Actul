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
#ifndef OFFLINESCHEDULER_HPP_
#define OFFLINESCHEDULER_HPP_

#include <Scheduler/DataraceScheduler.h>
#include <Scheduler/TestSettings.h>
#include <Scheduler/ReportPrinter.h>
#include <Scheduler/ThreadScheduler.h>
#include <Scheduler/ScheduleCreator.h>
#include <Logger/ConstrainLogger.h>
#include <Logger/DataRaceLogger.h>
#include <Util/SingletonClass.h>
#include <Util/Defines.h>
#include <Util/Container/Vector.h>
#include <Util/Container/SHM/SHMManager.h>

namespace Actul
{

struct PCounter
{
   double val;
   double tmp;

   PCounter(const double & val)
         : val(0),
           tmp(0)
   {
   }
};

struct PerformanceCounters
{
   PCounter startWorker;
   PCounter cleanUpWorker;
   PCounter idleWorker;
   PCounter resourceVio;
   PCounter updateOn;

   PerformanceCounters()
         : startWorker(0),
           cleanUpWorker(0),
           idleWorker(0),
           resourceVio(0),
           updateOn(0)
   {

   }

   void start(PCounter & in)
   {
      in.tmp = ProcessInfo::getRuntime();
   }

   void end(PCounter & in)
   {
      in.val += ProcessInfo::getRuntime() - in.tmp;
   }

   double getSecs(const double & val) const
   {
      return val;
   }

   void print() const
   {
      printStr("Performance:\n");
      printStr("Time: " + to_string(ProcessInfo::getRuntime()) + "\n");
      printStr("StartWorker: " + to_string(getSecs(startWorker.val)) + "\n");
      printStr("CleanWorker: " + to_string(getSecs(cleanUpWorker.val)) + "\n");
      printStr("IDLE_Master: " + to_string(getSecs(idleWorker.val)) + "\n");
      printStr("MBCa_Master: " + to_string(getSecs(resourceVio.val)) + "\n");
      printStr("Update DB: " + to_string(getSecs(updateOn.val)) + "\n");
   }
};

class TestScheduler : public SingletonClass<TestScheduler>
{
   friend class SingletonClass<TestScheduler> ;

   TestScheduler(const TestScheduler&) = delete;
   TestScheduler(TestScheduler&&) = delete;

 public:

   void start();

   void abortOnAssertion();

   ConstrainLogger & getConstrainLogger();
   DataRaceLogger & getDataRaceLogger();
   ThreadScheduler & getThreadScheduler();

   void printResults();
   void printBrief() const;
   void printRunningStates() const;
   void printCurrentDataRaceClassification() const;
   void printCurrentDataRaceClassificationHead() const;

 private:
   char _lastTestMode;
   typedef Vector<TestReport *> TestReportPtrVec;
   typedef SizeVec WorkerStack;
   static TestScheduler * _instance;

   size_type _maxNumPossibleSchedules;
   size_type _numDoneTests;
   size_type _numStartedTests;
   size_type _numCrashes;
   size_type _currentWorkerId;
   double _lastPrintTime;

   TestSettings & _settings;
   size_type _startSeed;
   PidVec _pids;
   WorkerStack _freeWorkerIds;

   mutable PerformanceCounters _pc;

   /* independent Logger*/
   YieldLogger & _yieLogger;

   /* Logger created by test scheduler*/
   ConstrainLogger _conLogger;
   DataRaceLogger _drLogger;

   /* Scheduler*/
   ThreadScheduler _thrScheduler;
   DataraceScheduler _drScheduler;

   // SHARED MEM:
   SHMManager * _shmManager;
   TestReportPtrVec _testReports;
   SizeVec _progressCounters;
   SizeVec _progressLookups;

   TestScheduler();
   ~TestScheduler();

   bool runDataRaceTesting();

   void cleanUpTest(const size_type& workerId);

   size_type getFinishedWorkerId();

   bool tryStartNewTest();

   bool tryCleanUpWorker();

   bool hasResourceViolation();

   /**
    * Starts new test based on current state of the offline graph and the static scheduler
    * Returns true, if its the offline scheduler, false when it is the test process
    */
   bool startTestWorker(const size_type & workerId, const bool & fetchTest);
   bool initializeWorker(const size_type & workerId);

   size_type mBytesAllocated() const;

   bool hasRunningWorkers() const;

   void deinitialize();

   size_type findWorkerId(const pid_t & pid);

   void killWorker(const size_type & workerId);

   void abortTesting();

   void checkSlaveProgress(const size_type & workerId);
   void checkSlaveProgress();

};

} /* namespace Actul */

#endif /* OFFLINESCHEDULER_HPP_ */
