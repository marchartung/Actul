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
#include <Scheduler/ThreadScheduler.h>
#include <Scheduler/ThreadInfo.h>
#include <Implementation/PthreadImpl.h>

#include <cstdlib>
#include <ctime>
#include <utility>
#include <algorithm>
#include <random>

namespace Actul
{

ThreadScheduler::ThreadScheduler(const TestSettings & settings, ConstrainLogger & conLogger)
      : _isReplaying(false),
        _hasReadViolation(false),
        _lastReleasedTid(0),
        _numSuccReleases(0),
        _lastStatPrint(ProcessInfo::getRuntime()),
        _settings(settings),
        _conLogger(conLogger),
        _evLogger(*EventLogger::getInstance()),
        _yieLogger(*YieldLogger::getInstance())
{
}


ThreadScheduler::~ThreadScheduler()
{
}

void ThreadScheduler::internalCall()
{
   if (PthreadImpl::peekInstance()->getRunningThreads().size() > 1)
   {
      tid_type release = getNextReleaseThread();

      actulAssert(release != invalid<tid_type>(), "ThreadScheduler: Cannot release a thread");
      if (ThreadInfo::getThreadId() != release)
         releaseThread(release, false);
      if (_evLogger.getCurEventType() == EventType::THREAD_EXIT)
         treatExitThread();
   }
}

void ThreadScheduler::treatExitThread()
{
   // this thread needs to release other thread and leave scheduler

   tid_type release = getNextReleaseThread();
   if (release != invalid<tid_type>())
   {
      actulAssert(release != ThreadInfo::getThreadId(), "ThreadScheduler: Exit thread and released thread are the same");
      releaseThread(release, true);
   }
}
void ThreadScheduler::print()
{
   printStr("Worker: " + to_string(ProcessInfo::getTestInfo().workerId) + "\n");
   printStr("Num Races: " + to_string(ProcessInfo::getTestReport().getDataRaces().size()) + "\n");
}

tid_type ThreadScheduler::getNextReleaseThread()
{
   tid_type release = invalid<tid_type>();
   setScheduleAbleThreads();
   ProcessInfo::checkProgramProperty(!_releaseInfo.curFreeTids.empty(), FailedProperty::DEADLOCK);
   if (_isReplaying)
   {
      release = getReplayRelease();
      if (release == invalid<tid_type>())
      {
         actulAssert(!_isReplaying, "ThreadScheduler: Trying to random schedule, but replaying is still active");
         release = getRandomRelease();
      }
   } else
      release = getRandomRelease();
   actulAssert(release != invalid<tid_type>() && !hasTooManyReleases(release), "ThreadScheduler: Try to release blocked thread.");
   actulAssert(release != invalid<tid_type>() && !_conLogger.isThreadBlocked(release), "ThreadScheduler: Try to release blocked thread.");
   actulAssert(release != invalid<tid_type>() && !PthreadImpl::peekInstance()->isBlocked(release), "ThreadScheduler: Try to release blocked thread.");

   logRelease(release);
   return release;
}

void ThreadScheduler::setScheduleAbleThreads()
{
   _releaseInfo.clear();
   setCurFreeThreads();
   if (_releaseInfo.curFreeTids.empty())
   {
      setCurReadViolatable();
      if (_releaseInfo.curFreeTids.empty())
      {
         setCurReleaseViolatable();
         if (_releaseInfo.curFreeTids.empty())
         {
            setCurConstrainViolatable();
         }
      }
   }
}

void ThreadScheduler::setCurFreeThreads()
{
   _releaseInfo.curFreeTids.clear();
   const TidVec & runTids = PthreadImpl::peekInstance()->getRunningThreads();
   for (size_type i = 0; i < runTids.size(); ++i)
   {
      const tid_type & tid = runTids[i];
      if (!(_conLogger.isThreadBlocked(tid) || hasTooManyReleases(tid) || PthreadImpl::peekInstance()->isBlocked(tid)))
         _releaseInfo.curFreeTids.push_back(tid);
   }
}

void ThreadScheduler::setCurReadViolatable()
{
   const TidVec & runTids = PthreadImpl::peekInstance()->getRunningThreads();
   for (size_type i = 0; i < runTids.size(); ++i)
   {
      const tid_type & tid = runTids[i];
      if (_conLogger.isThreadReadBlocked(tid) && !hasTooManyReleases(tid) && !_conLogger.isThreadConstrained(tid) && !PthreadImpl::peekInstance()->isBlocked(tid))
      {
         _releaseInfo.curFreeTids.push_back(tid);
      }
   }

   for (size_type i = 0; i < _releaseInfo.curFreeTids.size(); ++i)
   {
      _conLogger.violateReadConstrain(_releaseInfo.curFreeTids[i]);
      _releaseInfo.readViolated = true;
   }
}

void ThreadScheduler::setCurReleaseViolatable()
{
   const TidVec & runTids = PthreadImpl::peekInstance()->getRunningThreads();
   for (size_type i = 0; i < runTids.size(); ++i)
   {
      const tid_type & tid = runTids[i];
      if (!hasTooManyReleases(tid) && !_conLogger.isThreadConstrained(tid) && !PthreadImpl::peekInstance()->isBlocked(tid))
      {
         _releaseInfo.curFreeTids.push_back(tid);
      }
   }

   for (size_type i = 0; i < _releaseInfo.curFreeTids.size(); ++i)
   {
      _releaseInfo.releaseViolated = true;
      decreaseSuccReleases(_releaseInfo.curFreeTids[i]);
      if (_conLogger.isThreadReadBlocked(_releaseInfo.curFreeTids[i]))
      {
         _conLogger.violateReadConstrain(_releaseInfo.curFreeTids[i]);
         _releaseInfo.readViolated = true;
      }
   }
}

void ThreadScheduler::setCurConstrainViolatable()
{
   const TidVec & runTids = PthreadImpl::peekInstance()->getRunningThreads();
   for (size_type i = 0; i < runTids.size(); ++i)
   {
      const tid_type & tid = runTids[i];
      if (_conLogger.isThreadConstrained(tid) && !PthreadImpl::peekInstance()->isBlocked(tid))
         _releaseInfo.curFreeTids.push_back(tid);
   }

   for (size_type i = 0; i < _releaseInfo.curFreeTids.size(); ++i)
   {
      _releaseInfo.constrainViolated = true;
      _conLogger.removeConstrainsFromThread(_releaseInfo.curFreeTids[i]);
      if (hasTooManyReleases(_releaseInfo.curFreeTids[i]))
      {
         decreaseSuccReleases(_releaseInfo.curFreeTids[i]);
         _releaseInfo.releaseViolated = true;

      }
      if (_conLogger.isThreadReadBlocked(_releaseInfo.curFreeTids[i]))
      {
         _conLogger.violateReadConstrain(_releaseInfo.curFreeTids[i]);
         _releaseInfo.readViolated = true;
      }
   }

}

bool ThreadScheduler::shouldBeScheduledYield() const
{
   return _conLogger.shouldCurrentThreadYield() || PthreadImpl::peekInstance()->isCurBlocked();
}

bool ThreadScheduler::hasTooManyReleases(const tid_type & tid) const
{
   return (_lastReleasedTid == tid && _settings.maxSuccessiveReleases <= _numSuccReleases);
}

void ThreadScheduler::decreaseSuccReleases(const tid_type & tid)
{
   actulAssert(hasTooManyReleases(tid), "ThreadScheduler: Cannot decrease releases if thread is not bounded by releases");
   --_numSuccReleases;
}

void ThreadScheduler::getReplayToken(const tid_type & tid, ReplayToken & out) const
{
   out.readViolated = _releaseInfo.readViolated;
   out.releaseViolated = _releaseInfo.releaseViolated;
   out.constrainViolated = _releaseInfo.constrainViolated;
   out.et = _evLogger.getEventType(tid);
   out.tid = tid;
   out.numAccesses = ThreadInfo::getNumAccesses();
}

tid_type ThreadScheduler::getReplayRelease()
{
   actulAssert(_isReplaying, "ThreadScheduler: Inconsistent getReplayRelease function");
   tid_type res = invalid<tid_type>();
   TestReport & rep = ProcessInfo::getTestReport();
   _isReplaying = false;
   const size_type & numReleases = ProcessInfo::getNumReleases();
   if (rep.getReplayTokenArray().size() > numReleases)
   {
      const ReplayToken & replayToken = rep.getReplayTokenArray()[numReleases];
      ReplayToken realToken;
      getReplayToken(replayToken.tid, realToken);
      if (_releaseInfo.curFreeTids.find(replayToken.tid) == invalid<size_type>())
      {
         if (replayToken.readViolated && _conLogger.isThreadReadBlocked(replayToken.tid))
         {
            _releaseInfo.readViolated = true;
            _conLogger.violateReadConstrain(replayToken.tid);
         }
         if (replayToken.releaseViolated && hasTooManyReleases(replayToken.tid))
         {
            _releaseInfo.releaseViolated = true;
            decreaseSuccReleases(replayToken.tid);
         }
      }

      if (_releaseInfo.curFreeTids.find(replayToken.tid) != invalid<size_type>() && realToken == replayToken)
      {
         res = replayToken.tid;
         _isReplaying = true;
      } else
      {
         rep.getReplayTokenArray().resize(ProcessInfo::getNumReleases());
      }

   }
   return res;
}

tid_type ThreadScheduler::getRandomRelease()
{
   tid_type res = invalid<tid_type>();
   // search for threads which are blocked on too many successive reads

   res = getRandomFreeThread();

   return res;
}

void ThreadScheduler::releaseThread(const tid_type& tid, const bool & isExitRelease)
{

   if (isExitRelease)
      _yieLogger.exitReleaseThread(tid,ThreadInfo::getThreadId());
   else
      _yieLogger.yieldAndReleaseThread(tid);
}

void ThreadScheduler::logRelease(const tid_type & tid)
{
   //printStr("Release: " + to_string(tid) + "\n");
   TestReport & rep = ProcessInfo::getTestReport();
   if (_lastReleasedTid == tid)
      ++_numSuccReleases;
   else
   {
      _lastReleasedTid = tid;
      _numSuccReleases = 0;
   }
   if (!_isReplaying)
   {
      rep.getReplayTokenArray().insert(ReplayToken());
      getReplayToken(tid, rep.getReplayTokenArray().back());
   }
   ProcessInfo::increaseNumReleases();
}

void ThreadScheduler::setParameters(const size_type & workerId, const size_type & seed, TestReport & report)
{
   //long int seed = ((long int) testRun + offset) << (sizeof(size_type) * 8);
   //seed |= (long int) workerId;
   //printStr("seed: " + to_string(seed) + "\n");
   ProcessInfo::setFork(true);
   ProcessInfo::setTestReport(report);
   ProcessInfo::getTestInfo().runtime = 0.0;
   ProcessInfo::getTestInfo().isInScheduler[0] = true;
   ProcessInfo::resetProgramStartTime();
   ProcessInfo::getTestInfo().hasViolation();
   _randomEngine.seed(seed);
   if (report.getReplayTokenArray().size() > 0)
      _isReplaying = true;
   else
      _isReplaying = false;

   DataRaceShmVec & drs = report.getTestDataRaces();
   actulAssert(_conLogger.numConstrainer() == 0, "ThreadScheduler: Parameters cannot be set in a not empty ConstrainLogger");
   for (size_type i = 0; i < drs.size(); ++i)
      _conLogger.addDataRaceEvent(drs[i]);
   YieldEventShmVec & yields = report.getYieldEvents();
   for (size_type i = 0; i < yields.size(); ++i)
      _conLogger.addYieldEvent(yields[i]);

   report.getYieldEvents().clear();  // this container is used two ways, not so good
   report.getTestInfo().workerId = workerId;

}

size_type ThreadScheduler::getRandomNum(const size_type& numMax)
{
   size_type res = 0;

   std::uniform_int_distribution<size_type> dist(0, numMax - 1);
   res = dist(_randomEngine);

//printStr("random: " + to_string(res) + "/" + to_string(numMax) + "\n");
   return res;
}

tid_type ThreadScheduler::getRandomFreeThread()
{
   return _releaseInfo.curFreeTids[getRandomNum(_releaseInfo.curFreeTids.size())];
}

} /* namespace Actul */
