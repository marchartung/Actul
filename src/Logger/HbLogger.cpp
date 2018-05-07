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
#include <Logger/HbLogger.h>
#include <pthread.h>
#include <Util/DebugRoutines.h>
#include <Scheduler/ThreadInfo.h>

namespace Actul
{

template<>
HbLogger * SingletonClass<HbLogger>::_instance = nullptr;

HbLogger::HbLogger()
: _epLogger(*EpochLogger::getInstance())
{
   _clockHistory.push_back(Clock());
   for (size_type i = 0; i < _threadStates.size(); ++i)
   {
      _threadStates[i].curClockId = 0;
   }
}


HbLogger::~HbLogger()
{
   _clockHistory.clear();
}

const Clock& HbLogger::getClock(const size_type& cId) const
{
   actulAssert(cId < _clockHistory.size(), "HbLogger: Invalid clock id passed");
   return _clockHistory[cId];
}

const size_type & HbLogger::getClockId(const tid_type& tid) const
{
   actulAssert(tid < _threadStates.size(), "HbLogger: Invalid clock id passed");
   return _threadStates[tid].curClockId;
}

const size_type& HbLogger::getCurClockId() const
{
   return getClockId(ThreadInfo::getThreadId());
}

const Clock& HbLogger::getCurClock() const
{
   return getClock(getCurClockId());
}

size_type HbLogger::createNewClock(const size_type & clockId)
{
   size_type res = _clockHistory.size();
   _clockHistory.push_back(_clockHistory[clockId]);
   return res;
}

void HbLogger::threadHappendAfterClock(const tid_type & tid, const size_type & clockId)
{
   actulAssert(clockId < _clockHistory.size(), "HbLogger: Passed invalid clock id");
   ThreadHbState & ths = _threadStates[tid];
   ths.curClockId = createNewClock(ths.curClockId);
   Clock & c = _clockHistory[ths.curClockId];
   c.tick(tid);
   c.max(_clockHistory[clockId]);
   _epLogger.updateClock(tid,ths.curClockId);
}

void HbLogger::curThreadHappendAfterClock(const size_type & clockId)
{
   threadHappendAfterClock(ThreadInfo::getThreadId(),clockId);
}

size_type HbLogger::curThreadTickClock()
{
   const tid_type & tid = ThreadInfo::getThreadId();
   ThreadHbState & ths = _threadStates[tid];

   ths.curClockId = createNewClock(ths.curClockId);
   _clockHistory[ths.curClockId].tick(tid);
   _epLogger.updateClock(tid,ths.curClockId);
   return ths.curClockId;
}

void HbLogger::barrierOnThreads(const TidArray & tids)
{
   actulAssert(tids[0] != invalid<tid_type>(), "HbLogger: Threads not initialized in barrier");
   size_type newClockId = createNewClock(_threadStates[tids[0]].curClockId);
   Clock & newClock = _clockHistory[newClockId];
   for (size_type i = 0; i < tids.size() && tids[i] != invalid<tid_type>(); ++i)
   {
      ThreadHbState & ths = _threadStates[tids[i]];
      newClock.max(getClock(ths.curClockId));
      newClock.tick(tids[i]);
      ths.curClockId = newClockId;
      _epLogger.updateClock(tids[i],newClockId);
   }
}

}
