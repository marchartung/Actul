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
//    Marc Hartung
////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDE_LOGGER_LOCKSETLOGGER_H_
#define INCLUDE_LOGGER_LOCKSETLOGGER_H_

#include <Util/Defines.h>
#include <Util/LibcAllocator.h>
#include <Util/SingletonClass.h>
#include <Logger/EpochLogger.h>
#include <Logger/LockSet.h>
#include <Scheduler/ThreadInfo.h>
#include <Scheduler/ProcessInfo.h>
#include <vector>
#include <array>
#include <algorithm>

namespace Actul
{

struct LockSetLoggerMemorySpec
{
   size_type historySize;
};

class LockSetLogger : public SingletonClass<LockSetLogger>
{
   friend class SingletonClass<LockSetLogger>;

   LockSetLogger(LockSetLogger&&) = delete;
   LockSetLogger(const LockSetLogger&) = delete;

 public:


   const LockSet & getLockSet(const size_type & lsId) const;
   const size_type & getCurLockSetId() const;
   const size_type & getLockSetId(const tid_type & tid) const;
   const LockSet & getCurLockSet() const;


   template<bool isWriteLock>
   void addLock(const event_id& id)
   {
      size_type & thrLockId = _curLockSets[ThreadInfo::getThreadId()];
      thrLockId = createNewLocksSet(thrLockId);
      if(isWriteLock)
         _lockSetHistory[thrLockId].addWriteLock(id);
      else
         _lockSetHistory[thrLockId].addReadLock(id);
      _epLogger.updateLockSet(ThreadInfo::getThreadId(),thrLockId);
   }

   void removeLock(const event_id & id);

 private:
   EpochLogger & _epLogger;
   SizeArray _curLockSets;
   LockSetVec _lockSetHistory;

   LockSetLogger();
   ~LockSetLogger();

   size_type createNewLocksSet(const size_type & from);
};

} /* namespace Actul */

#endif /* INCLUDE_LOGGER_LOCKSETLOGGER_H_ */
