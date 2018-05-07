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
#include <Util/EnvironmentDependencies.h>
#include <Scheduler/ThreadInfo.h>
#include <Util/ThreadLockGate.h>
#include <Util/DebugRoutines.h>

#include <unistd.h>

namespace Actul
{

ThreadLockGate::ThreadLockGate()
      : _releaserExits(false)
{
}

ThreadLockGate::~ThreadLockGate()
{
}

void ThreadLockGate::init(const tid_type & releaseTid)
{
   _sems[releaseTid].init();
}
void ThreadLockGate::deinit(const tid_type & releaseTid)
{
   _sems[releaseTid].deinit();
}

size_type ThreadLockGate::size() const
{
   return _sems.size();
}

void ThreadLockGate::releaseBlocking(const tid_type& releaseTid)
{
   actulAssert(releaseTid < ACTUL_MAX_NUM_THREADS, "ThreadLockGate: To many threads running. Please rebuild with -DMAX_NUM_THREADS=[desiredMaxThreadCount]");
   _sems[releaseTid].post(ThreadInfo::getThreadId());  // TODO or set to ThreadLogger::getInvalidTid()
}

void ThreadLockGate::releaseNoWait(const tid_type& releaseTid)
{
   actulAssert(releaseTid < ACTUL_MAX_NUM_THREADS, "ThreadLockGate: To many threads running. Please rebuild with -DMAX_NUM_THREADS=[desiredMaxThreadCount]");
   _sems[releaseTid].post(invalid<tid_type>());  // TODO or set to ThreadLogger::getInvalidTid()
}

void ThreadLockGate::exitRelease(const tid_type & toRelease, const tid_type & toExit)
{
   _releaserExits = true;
   _sems[toRelease].post(toExit);
}

void ThreadLockGate::yieldAndRelease(const tid_type& releaseTid)
{
   actulAssert(releaseTid < ACTUL_MAX_NUM_THREADS, "ThreadLockGate: To many threads running. Please rebuild with -DMAX_NUM_THREADS=[desiredMaxThreadCount]");
   releaseBlocking(releaseTid);
   yield();
}

void ThreadLockGate::yield()
{
   _sems[ThreadInfo::getThreadId()].wait();
   // checking if releasing thread yielded:
   tid_type releaser = _sems[ThreadInfo::getThreadId()].getLastPostThreadId();
   if (releaser != invalid<tid_type>())
   {
      if (_releaserExits)
         waitForThreadToDeinit(releaser);
      else
         waitForThreadToYield(releaser);
   }
}

void ThreadLockGate::waitForThreadToYield(const tid_type& tid) const
{
   actulAssert(tid < ACTUL_MAX_NUM_THREADS, "ThreadLockGate: To many threads running. Please rebuild with -DMAX_NUM_THREADS=[desiredMaxThreadCount]");

   while (!_sems[tid].isThreadWaiting())
   {
      usleep(5);
   }  // checks if the other thread is already waiting
}

void ThreadLockGate::waitForThreadToDeinit(const tid_type& tid)
{
   actulAssert(tid < ACTUL_MAX_NUM_THREADS, "ThreadLockGate: To many threads running. Please rebuild with -DMAX_NUM_THREADS=[desiredMaxThreadCount]");

   while (_sems[tid].isInitialized())
   {
      usleep(5);
   }  // checks if the other thread is still running
   _releaserExits = false;
}

}

