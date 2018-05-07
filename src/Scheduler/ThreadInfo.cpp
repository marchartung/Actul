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
#include <Scheduler/ThreadInfo.h>
#include <Scheduler/ProcessInfo.h>

namespace Actul
{

thread_local volatile bool ThreadInfo::_inScheduler = false;
thread_local tid_type ThreadInfo::_tid = 0;
thread_local volatile size_type ThreadInfo::_epochId = 0;
thread_local size_type ThreadInfo::_numAccessesOnThread = 0;

bool ThreadInfo::isInScheduler()
{
   return _inScheduler;
}

void ThreadInfo::setInScheduler(const bool & isScheduler)
{
   actulAssert(isScheduler != _inScheduler, "ThreadInfo: Infinite recursion prevented. Actul tried to schedule itself");
   if(ProcessInfo::isTestReportSet() && !ProcessInfo::hasAssertion())
   {
      bool & inSched = ProcessInfo::getTestReport().getTestInfo().isInScheduler[_tid];
      actulAssert(_inScheduler == inSched, "ThreadInfo: Inconsistent schedule tracking");
      inSched = isScheduler;
   }
   _inScheduler = isScheduler;
}

const tid_type & ThreadInfo::getThreadId()
{
   actulAssert(_tid != invalid<tid_type>(), "ThreadInfo: Cannot return invalid thread id");
   return _tid;
}

void ThreadInfo::setThreadId(const tid_type & tid)
{
   _tid = tid;
}

const size_type & ThreadInfo::getNumAccesses()
{
   return _numAccessesOnThread;
}

void ThreadInfo::increaceNumAccesses()
{
   ++_numAccessesOnThread;
}

} /* namespace Actul */
