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
#ifndef INCLUDE_ONLINESCHEDULER_HPP_
#define INCLUDE_ONLINESCHEDULER_HPP_

#include <Util/Defines.h>
#include <Scheduler/TestReport.h>
#include <Logger/ConstrainLogger.h>
#include <Logger/YieldLogger.h>
#include <vector>
#include <random>

namespace Actul
{

class ThreadScheduler
{
   ThreadScheduler() = delete;
   ThreadScheduler(const ThreadScheduler &) = delete;
   ThreadScheduler(ThreadScheduler&&) = delete;
 public:

   ThreadScheduler(const TestSettings & settings, ConstrainLogger & conLogger);
   ~ThreadScheduler();

   void setParameters(const size_type & workerId, const size_type & seed, TestReport & report);

   void treatExitThread();

   void print();

   void abortOnAssertion();

   template<EventType et>
   void call()
   {
      ProcessInfo::getTestReport().getTestInfo().setProgress();
      //if(ProcessInfo::getRuntime() - _lastStatPrint > _settings.printInterval)
      //{
      //   printStats();
      //   _lastStatPrint = ProcessInfo::getRuntime();
      //}
      if (shouldSchedule<et>())
         internalCall();
   }

 private:

   /* Variables for Functionality */
   bool _isReplaying;
   bool _hasReadViolation;
   tid_type _lastReleasedTid;
   size_type _numSuccReleases;
   double _lastStatPrint;
   std::default_random_engine _randomEngine;
   const TestSettings & _settings;

   ConstrainLogger & _conLogger;
   EventLogger & _evLogger;
   YieldLogger & _yieLogger;

   struct ReleaseInfo
   {
      bool readViolated;
      bool releaseViolated;
      bool constrainViolated;
      TidVec curFreeTids;

      void clear()
      {
         readViolated = false;
         releaseViolated = false;
         constrainViolated = false;
         curFreeTids.clear();
      }
   };
   ReleaseInfo _releaseInfo;

   template<EventType et>
   bool shouldSchedule()
   {
      bool res = false;
      if (isDefaultYieldEvent<et>())
         res = true;
      else
      {
         res = _conLogger.shouldCurrentThreadYield();
      }
      return res;
   }

   void internalCall();

   tid_type getNextReleaseThread();

   tid_type getReplayRelease();

   tid_type getRandomRelease();

   bool shouldBeScheduledYield() const;

   bool hasTooManyReleases(const tid_type & tid) const;

   void decreaseSuccReleases(const tid_type & tid);

   /*
    * Releases the thread [tid] and blocks till it returns.
    * The function returns true, if the exectuted epoch was already in the OfflineGraph, false otherwise.
    */
   void releaseThread(const tid_type & tid, const bool & isExitRelease);

   void logRelease(const tid_type & tid);

   size_type getRandomNum(const size_type & numMax);

   void setSeed(const size_type & testRun, const size_type workerId, const size_type & offset);

   void setScheduleAbleThreads();

   void setCurFreeThreads();

   void setCurReadViolatable();

   void setCurReleaseViolatable();

   void setCurConstrainViolatable();

   tid_type getRandomFreeThread();

   void getReplayToken(const tid_type & tid, ReplayToken & r) const;

};

} /* namespace Actul */

#endif /* INCLUDE_ONLINESCHEDULER_HPP_ */
