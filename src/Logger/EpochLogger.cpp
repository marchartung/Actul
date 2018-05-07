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
#include <Logger/EpochLogger.h>
#include <Scheduler/ThreadInfo.h>
#include <Scheduler/ProcessInfo.h>

namespace Actul
{
template<>
EpochLogger * SingletonClass<EpochLogger>::_instance = nullptr;

EpochLogger::EpochLogger()
{
   _epochs.push_back(Epoch(0, 0, 0, 0));
   _curEpochs[0] = 0;
   for (size_type i = 1; i < _curEpochs.size(); ++i)
   {
      _curEpochs[i] = invalid<size_type>();
   }
}

size_type EpochLogger::getCurEpochId() const
{
   return _curEpochs[ThreadInfo::getThreadId()];
}

const Epoch& EpochLogger::getEpoch(const size_type& epochId) const
{
   return _epochs[epochId];
}


void EpochLogger::curUpdateRelease()
{
   size_type & curEpId = _curEpochs[ThreadInfo::getThreadId()];
   actulAssert(curEpId != invalid<size_type>(), "EpochLogger: Invalid release. Epoch wasn't defined yet");
}


void EpochLogger::updateClock(const tid_type & tid, const size_type& clockId)
{
   size_type & curE = _curEpochs[tid];
   if (curE != invalid<size_type>())
      _epochs.push_back(_epochs[curE]);
   else
   {
      _epochs.push_back(_epochs[0]);
      _epochs.back().tid = tid;
   }
   curE = _epochs.size() - 1;
   actulAssert(_epochs[curE].clockId < clockId, "EpochLogger: Update a clock id which is smaller than the current. Bug?");
   _epochs[curE].clockId = clockId;
}

void EpochLogger::updateLockSet(const tid_type & tid, const size_type& lockSetId)
{
   size_type & curE = _curEpochs[tid];
   if (curE != invalid<size_type>())
      _epochs.push_back(_epochs[curE]);
   else
   {
      _epochs.push_back(_epochs[0]);
      _epochs.back().tid = tid;
   }
   curE = _epochs.size() - 1;
   _epochs[curE].lockSetId = lockSetId;
}

} /* namespace Actul */

