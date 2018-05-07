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
#include <Logger/LockSetLogger.h>

#include <pthread.h>

namespace Actul
{

template<>
LockSetLogger * SingletonClass<LockSetLogger>::_instance = nullptr;

LockSetLogger::LockSetLogger()
      : _epLogger(*EpochLogger::getInstance()),
        _lockSetHistory(1)
{
   for (size_type i = 0; i < _curLockSets.size(); ++i)
   {
      _curLockSets[i] = 0;
   }
}

LockSetLogger::~LockSetLogger()
{
   _lockSetHistory.clear();
}

size_type LockSetLogger::createNewLocksSet(const size_type & from)
{
   actulAssert(from < _lockSetHistory.size(), "LockSetLogger: Cannot create lock set from invalid id");
   size_type res = _lockSetHistory.size();
   _lockSetHistory.push_back(_lockSetHistory[from]);
   return res;
}

const LockSet & LockSetLogger::getLockSet(const size_type & lsId) const
{
   actulAssert(lsId < _lockSetHistory.size(), "LockSetLogger: Invalid access to lock set from history");
   return _lockSetHistory[lsId];
}

const size_type & LockSetLogger::getCurLockSetId() const
{
   return _curLockSets[ThreadInfo::getThreadId()];
}

const size_type& LockSetLogger::getLockSetId(const tid_type& tid) const
{
   actulAssert(tid != invalid<tid_type>(), "LockSetLogger: Invalid thread id passed");
   return _curLockSets[tid];
}

const LockSet & LockSetLogger::getCurLockSet() const
{
   return getLockSet(getCurLockSetId());
}

void LockSetLogger::removeLock(const event_id& id)
{
   size_type & thrLockId = _curLockSets[ThreadInfo::getThreadId()];
   thrLockId = createNewLocksSet(thrLockId);
   _lockSetHistory[thrLockId].removeLock(id);
   _epLogger.updateLockSet(ThreadInfo::getThreadId(),thrLockId);
}

} /* namespace Actul */
