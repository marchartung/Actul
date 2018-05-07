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
#ifndef INCLUDE_IMPLEMENTATION_PTHREADIMPL_H_
#define INCLUDE_IMPLEMENTATION_PTHREADIMPL_H_

#include <Util/Defines.h>
#include <Util/SingletonClass.h>
#include <Logger/ActulLogger.h>
#include <Logger/EventLogger.h>
#include <Logger/YieldLogger.h>
#include <Scheduler/ProcessInfo.h>
#include <Scheduler/ThreadInfo.h>

#include <Util/Container/Vector.h>
#include <Util/Container/Array.h>
#include <Util/Utility.h>

#include <pthread.h>

namespace Actul
{

/**
 * FIXME Implement Semaphores
 */

/* TODO: In Barrier: states are also handled outside of BarrierState, not good
 *       In Cond: HbLogger calls should be moved to CondState
 *       In Locks: LockSetLogger calls should be moved to LockState
 */

class PthreadImpl : public SingletonClass<PthreadImpl>
{
   friend class SingletonClass<PthreadImpl> ;

   class ThreadState;
   typedef Array<ThreadState, ACTUL_MAX_NUM_THREADS> ThreadStateArray;

   class ThreadState
   {
      enum ThreadStateType
      {
         TST_NOT_CREATED,
         TST_NOT_JOINABLE,
         TST_JOINABLE,
         TST_JOINABLE_EXITED
      };
    public:
      ThreadState();

      const size_type & getSyncId() const;
      const EventType & getSyncType() const;

      const pthread_t & getPid() const;

      bool isBlocked(const EventType & et, const tid_type & tid) const;

      bool isUnused() const;

      bool isJoinable() const;

      bool hasExited() const;

      bool hasWaiter() const;

      bool isWaiting() const;

      void create(const pthread_t & pid, const event_id &id, const bool & joinable);
      const event_id & getEventId() const;

      void goingToJoin(ThreadStateArray & tsa);

      void * getReturnVal() const;

      void exit(ThreadStateArray & tsa, const size_type & clockId, void * returnVal);

      const size_type & getClockId() const;

      void join();

      void waitsForThreads(const TidVec & tids);

      void waitsForThread(const tid_type & tid);

    private:
      bool _isJoinabled;
      pthread_t _pid;
      tid_type _waiter;
      event_id _eid;
      size_type _clockId;
      void * _returnVal;
      TidArray _waitsFor;
   };

   class LockState
   {
      enum LockType
      {
         LT_RWLOCK,
         LT_MUTEX
      };

      enum CurrentLockState
      {
         CLS_NOT_INITIALIIZED_LOCK,
         CLS_INITIALIZED_LOCK,
         CLS_WRITE_OWNED_LOCK,
         CLS_READ_OWNED_LOCK
      };
      enum CurrentThreadState
      {
         CTS_NONE,
         CTS_READ,
         CTS_WRITE,
         CTS_TRYREAD,
         CTS_TRYWRITE
      };
      typedef std::array<CurrentThreadState, ACTUL_MAX_NUM_THREADS> CurrentThreadStateArray;
    public:
      LockState();

      bool isInitialized() const;

      template<bool isWriteLock>
      bool isFree() const
      {
         actulAssert(_state != CurrentLockState::CLS_NOT_INITIALIIZED_LOCK, "LockState: Cannot determine if an uninitialized lock is free or not");
         return _state == CurrentLockState::CLS_INITIALIZED_LOCK || ((!isWriteLock) ? _state == CurrentLockState::CLS_READ_OWNED_LOCK : false);
      }

      bool canBeDestroyed() const;

      bool isBlocked(const EventType & et, const tid_type & tid) const;

      bool isOwner() const;

      void initialize(const unsigned & initVal);

      void deinitialize();

      template<bool isWriteLock, bool isTryLock>
      void addWaiter()
      {
         actulAssert(_state != CurrentLockState::CLS_NOT_INITIALIIZED_LOCK, "LockState: Cannot add waiter to uninitialized lock");
         size_type numWs = getNumWaiters();
         const tid_type & tid = ThreadInfo::getThreadId();
         _waiters[numWs] = tid;
         _waiterStates[numWs] = (
               (isTryLock) ?
                     ((isWriteLock) ? CurrentThreadState::CTS_TRYWRITE : CurrentThreadState::CTS_TRYREAD) :
                     ((isWriteLock) ? CurrentThreadState::CTS_WRITE : CurrentThreadState::CTS_READ));
      }

      template<bool isWriteLock>
      void lock()
      {
         actulAssert(_state != CurrentLockState::CLS_NOT_INITIALIIZED_LOCK, "LockState: Cannot lock a uninitialized lock");
         actulAssert(isFree<isWriteLock>(), "LockState: Cannot lock an locked lock");
         actulAssert(!isOwner(), "LockState: Cannot lock again");

         const tid_type & tid = ThreadInfo::getThreadId();
         size_type pos = getWaiter(tid);
         bool isWriteDebug = _waiterStates[pos] == CurrentThreadState::CTS_TRYWRITE || _waiterStates[pos] == CurrentThreadState::CTS_WRITE;
         actulAssert(isWriteLock == isWriteDebug, "LockState: Inconsistent lock state");

         removeWaiter();

         size_type numOs = getNumOwners();
         _owners[numOs] = tid;
         _state = (isWriteLock) ? CurrentLockState::CLS_WRITE_OWNED_LOCK : CurrentLockState::CLS_READ_OWNED_LOCK;
      }

      void unlock();

      void removeWaiter();

    private:
      CurrentLockState _state;
      TidArray _waiters;
      CurrentThreadStateArray _waiterStates;
      TidArray _owners;

      template<bool isWrite>
      bool isBlockedInternal(const size_type & i)
      {
         bool res = !(_waiterStates[i] == CurrentThreadState::CTS_TRYREAD || _waiterStates[i] == CurrentThreadState::CTS_TRYWRITE);
         if (res && !isWrite)  // when its not try and won't be write lock:
         {
            res = (_waiterStates[i] != CurrentThreadState::CTS_READ);  // check if the waiter is a read lock
         }
         return res;
      }

      int getNumWaiters() const;

      int getNumOwners() const;

      size_type getWaiter(const tid_type & tid) const;

      size_type findOwner(const tid_type & tid) const;
   };
   typedef Vector<LockState> LockStateVec;

   struct LockRef
   {
      size_type lId;
      const LockStateVec * vec;

      LockRef()
            : lId(invalid<size_type>()),
              vec(nullptr)
      {
      }

      LockRef(const size_type & lid, const LockStateVec & vec)
            : lId(lid),
              vec(&vec)
      {
      }

      const LockState & getLockState() const
      {
         return (*vec)[lId];
      }
   };

   class CondState
   {

    public:

      CondState();

      bool isBlocked(const EventType & et, const tid_type & tid) const;

      bool isInitialized() const;

      bool canBeDestroyed() const;

      void initialize(const unsigned & initVal);

      void deinitialize();

      void wait(const LockRef & lref, const bool & timed);

      void signal(const size_type & clockId);

      void bcast(const size_type & clockId);

      size_type removeWaiter();

    private:

      struct WaiterInfo
      {
         bool isTryWait;
         tid_type tid;
         size_type clockStamp;
         LockRef lref;
      };

      struct ClockStamps
      {
         bool isBcast;
         size_type stamp;
         size_type clockId;
      };
      bool _init;
      size_type _stampCounter;
      Vector<WaiterInfo> _waiters;
      Vector<ClockStamps> _clockStamps;

      size_type getWaiter(const tid_type & tid) const;

      size_type findClock(const size_type & clock) const;

   };
   typedef Vector<CondState> CondStateVec;

   class SemState
   {

    public:

      bool isInitialized() const
      {
         actulAssert(false, "SemState: not implemented");
         return false;
      }

      bool isBlocked(const EventType & et, const tid_type & tid) const
      {
         actulAssert(false, "SemState: not implemented");
         return false;
      }

      bool canBeDestroyed() const
      {
         actulAssert(false, "SemState: not implemented");
         return true;
      }

      void initialize(const unsigned & initVal)
      {
         actulAssert(false, "SemState: not implemented");
      }

      void deinitialize()
      {
         actulAssert(false, "SemState: not implemented");
      }

    private:

   };
   typedef Vector<SemState> SemStateVec;

   class BarrierState
   {
    public:
      BarrierState();

      bool isInitialized() const;

      bool isFree() const;

      bool isBlocked(const EventType & et, const tid_type & tid) const;

      void initialize(const unsigned & initVal);

      void deinitialize();

      int getNumWaiters() const;

      bool isWaiter(const tid_type & tid) const;

      bool tryWait();

      const TidArray & getWaiters() const;

      void resetWaiters();

    private:
      int _init;
      TidArray _waiters;

   };
   typedef Vector<BarrierState> BarrierStateVec;

   ////////////////////////////////////////
   ////////////////////////////////////////
   //// Implementation of PthreadImpl ////
   ////////////////////////////////////////
   ////////////////////////////////////////

 public:

   bool isBlocked(const tid_type & tid) const;

   bool isCurBlocked() const;

   const TidVec & getRunningThreads() const;

   const TidVec & getThreads() const;

   template<EventType et>
   int initialize(AddressType addr, const unsigned & initVal)
   {
      const event_id & id = ActulLogger::notifySyncEventPre<et>(addr);
      int res = EBUSY;
      // TODO use some template magic to get rid of repetition:
      if (isBarrierEvent<et>())
      {
         adjustSize(_barriers, id + 1);
         auto & s = _barriers[id];
         ProcessInfo::checkProgramProperty(!s.isInitialized(), FailedProperty::DOUBLE_SYNC_INITIALIZE);
         if (!s.isInitialized())
         {
            s.initialize(initVal);
            res = 0;
         }
      } else if (isConditionalEvent<et>())
      {
         adjustSize(_conds, id + 1);
         auto & s = _conds[id];
         ProcessInfo::checkProgramProperty(!s.isInitialized(), FailedProperty::DOUBLE_SYNC_INITIALIZE);
         if (!s.isInitialized())
         {
            s.initialize(initVal);
            res = 0;
         }
      } else if (isLockEvent<et>())
      {
         adjustSize(_locks, id + 1);
         auto & s = _locks[id];
         ProcessInfo::checkProgramProperty(!s.isInitialized(), FailedProperty::DOUBLE_SYNC_INITIALIZE);
         if (!s.isInitialized())
         {
            s.initialize(initVal);
            res = 0;
         }
      } else if (isSemaphoreEvent<et>())
      {
         adjustSize(_sems, id + 1);
         auto & s = _sems[id];
         ProcessInfo::checkProgramProperty(!s.isInitialized(), FailedProperty::DOUBLE_SYNC_INITIALIZE);
         if (!s.isInitialized())
         {
            s.initialize(initVal);
            res = 0;
         }
      }
      ActulLogger::notifySyncEventPost<et>(addr, id);
      return res;
   }

   template<EventType et>
   int deinitialize(AddressType addr)
   {
      const event_id & id = ActulLogger::notifySyncEventPre<et>(addr);
      int res = EINVAL;
      // TODO use some template magic to get rid of repetition:
      if (isBarrierEvent<et>())
      {
         adjustSize(_barriers, id + 1);
         auto & s = _barriers[id];
         ProcessInfo::checkProgramProperty(s.isInitialized(), FailedProperty::DOUBLE_SYNC_INITIALIZE);
         if (s.isInitialized())
         {
            if (ProcessInfo::checkProgramProperty(s.isFree(), FailedProperty::INVALID_SYNC_DESTROY))
            {
               s.deinitialize();
               res = 0;
            } else
               res = EBUSY;
         }
      } else if (isConditionalEvent<et>())
      {
         adjustSize(_conds, id + 1);
         auto & s = _conds[id];
         ProcessInfo::checkProgramProperty(s.isInitialized(), FailedProperty::DOUBLE_SYNC_INITIALIZE);
         if (s.isInitialized())
         {
            if (ProcessInfo::checkProgramProperty(s.canBeDestroyed(), FailedProperty::INVALID_SYNC_DESTROY))
            {
               s.deinitialize();
               res = 0;
            } else
               res = EBUSY;
         }
      } else if (isLockEvent<et>())
      {
         adjustSize(_locks, id + 1);
         auto & s = _locks[id];
         ProcessInfo::checkProgramProperty(s.isInitialized(), FailedProperty::DOUBLE_SYNC_INITIALIZE);
         if (s.isInitialized())
         {
            if (ProcessInfo::checkProgramProperty(s.canBeDestroyed(), FailedProperty::INVALID_SYNC_DESTROY))
            {
               s.deinitialize();
               res = 0;
            } else
               res = EBUSY;
         }
      } else if (isSemaphoreEvent<et>())
      {
         adjustSize(_sems, id + 1);
         auto & s = _sems[id];
         ProcessInfo::checkProgramProperty(s.isInitialized(), FailedProperty::DOUBLE_SYNC_INITIALIZE);
         if (s.isInitialized())
         {
            if (ProcessInfo::checkProgramProperty(s.canBeDestroyed(), FailedProperty::INVALID_SYNC_DESTROY))
            {
               s.deinitialize();
               res = 0;
            } else
               res = EBUSY;
         }
      }
      ActulLogger::notifySyncEventPost<et>(addr, id);
      return res;
   }

   void* threadStart(void* arg);

   template<EventType et, typename ReturnType, typename ArgumentType>
   ReturnType act(AddressType addr, ArgumentType arg)
   {
      const event_id & id = ActulLogger::notifySyncEventPre<et>(addr);
      ReturnType res = internalAct<et, ReturnType, ArgumentType>(id, addr, arg);
      ActulLogger::notifySyncEventPost<et>(addr, id);
      return res;
   }

 private:
   LockSetLogger & _lsLogger;
   HbLogger & _hbLogger;
   EventLogger & _eiLogger;
   YieldLogger & _yieLogger;
   TidVec _runningThreads;
   TidVec _ranThreads;
   ThreadStateArray _threads;
   BarrierStateVec _barriers;
   CondStateVec _conds;
   LockStateVec _locks;
   SemStateVec _sems;

   PthreadImpl();

   template<typename T>
   void adjustSize(Vector<T> & vec, const size_type & sz)
   {
      if (sz >= vec.size())
         vec.resize(sz);
   }

   template<typename T>
   bool isInitialized(const event_id & id)
   {
      return false;
   }

   template<EventType et, typename ReturnType, typename ArgumentType>
   ReturnType internalAct(const event_id & id, AddressType addr, ArgumentType arg)
   {
      actulAssert(false, "PthreadImpl: Event not captured");
      return 0;
   }

   template<EventType et, bool isWriteLock, bool isTrylock>
   int pthread_lock(const event_id & id)
   {
      int res = EINVAL;
      bool blocked;
      if (!isInitialized<LockState>(id))
      {
         AddressType addr = _eiLogger.getCurAddress<et>();

         if (isMutexLock<et>())
         {
            pthread_mutex_t tmp = PTHREAD_MUTEX_INITIALIZER;
            if (ByteEqual(&tmp, reinterpret_cast<const pthread_mutex_t*>(addr), sizeof(pthread_mutex_t)))
            {
               adjustSize(_locks, id + 1);
               auto & s = _locks[id];
               s.initialize(0);
            }
         }

      }
      if (ProcessInfo::checkProgramProperty(isInitialized<LockState>(id), FailedProperty::ACCESS_UNINITIALIZED_LOCK))
      {
         LockState & ls = _locks[id];
         if (ProcessInfo::checkProgramProperty(!ls.isOwner(), FailedProperty::ACCESS_UNINITIALIZED_LOCK))
         {
            ls.addWaiter<isWriteLock, isTrylock>();
            blocked = (isTrylock || ls.isFree<isWriteLock>()) ? false : true;
            ActulLogger::callScheduler<et>(blocked, id);

            if (ProcessInfo::checkProgramProperty(isInitialized<LockState>(id), FailedProperty::ACCESS_UNINITIALIZED_LOCK))
            {
               LockState & ls = _locks[id];
               if (ls.isFree<isWriteLock>())
               {
                  ls.lock<isWriteLock>();
                  _lsLogger.addLock<isWriteLock>(id);
                  res = 0;
               } else
               {
                  actulAssert(isTrylock, "PthreadImpl: Scheduler released thread on locked lock");
                  ls.removeWaiter();
                  res = EBUSY;
               }
            }
         } else
            res = EDEADLK;

      }
      return res;
   }

   template<EventType et>
   int pthread_unlock(const event_id & id)
   {
      int res = EINVAL;

      if (ProcessInfo::checkProgramProperty(isInitialized<LockState>(id), FailedProperty::ACCESS_UNINITIALIZED_LOCK))
      {
         LockState & ls = _locks[id];
         if (ProcessInfo::checkProgramProperty(ls.isOwner(), FailedProperty::ACCESS_UNINITIALIZED_LOCK))
         {

            ActulLogger::callScheduler<et>(false, id);

            if (ProcessInfo::checkProgramProperty(ls.isOwner(), FailedProperty::ACCESS_UNINITIALIZED_LOCK))
            {
               res = 0;
               ls.unlock();
               _lsLogger.removeLock(id);
            }
         } else
            res = EPERM;

      }
      return res;
   }

   tid_type findUnusedThreadId() const;

   tid_type findTidOfPid(const pthread_t & pid) const;
}
;

template<>
bool PthreadImpl::isInitialized<PthreadImpl::BarrierState>(const event_id & id);
template<>
bool PthreadImpl::isInitialized<PthreadImpl::CondState>(const event_id & id);
template<>
bool PthreadImpl::isInitialized<PthreadImpl::LockState>(const event_id & id);
template<>
bool PthreadImpl::isInitialized<PthreadImpl::SemStateVec>(const event_id & id);

template<>
int PthreadImpl::internalAct<EventType::BARRIER_WAIT, int, void*>(const event_id & id, AddressType addr, void * dummy);

template<>
int PthreadImpl::internalAct<EventType::COND_WAIT, int, pthread_mutex_t*>(const event_id & id, AddressType addr, pthread_mutex_t * m);
template<>
int PthreadImpl::internalAct<EventType::COND_TIMEDWAIT, int, pthread_mutex_t*>(const event_id & id, AddressType addr, pthread_mutex_t * m);
template<>
int PthreadImpl::internalAct<EventType::COND_SIGNAL, int, void*>(const event_id & id, AddressType addr, void * dummy);
template<>
int PthreadImpl::internalAct<EventType::COND_BCAST, int, void*>(const event_id & id, AddressType addr, void * dummy);

template<>
int PthreadImpl::internalAct<EventType::MUTEXLOCK, int, void*>(const event_id & id, AddressType addr, void * dummy);
template<>
int PthreadImpl::internalAct<EventType::MUTEXTRYLOCK, int, void*>(const event_id & id, AddressType addr, void * dummy);
template<>
int PthreadImpl::internalAct<EventType::RWLOCK_WRITE, int, void*>(const event_id & id, AddressType addr, void * dummy);
template<>
int PthreadImpl::internalAct<EventType::RWTRYLOCK_WRITE, int, void*>(const event_id & id, AddressType addr, void * dummy);
template<>
int PthreadImpl::internalAct<EventType::RWTRYLOCK_READ, int, void*>(const event_id & id, AddressType addr, void * dummy);
template<>
int PthreadImpl::internalAct<EventType::RWLOCK_READ, int, void*>(const event_id & id, AddressType addr, void * dummy);
template<>
int PthreadImpl::internalAct<EventType::MUTEXUNLOCK, int, void*>(const event_id & id, AddressType addr, void * dummy);
template<>
int PthreadImpl::internalAct<EventType::RWUNLOCK, int, void*>(const event_id & id, AddressType addr, void * dummy);

template<>
int PthreadImpl::act<EventType::THREAD_CREATE, int, std::tuple<pthread_t*, const pthread_attr_t*, Actul::ExecuteFunction, void*>>(
      AddressType addr, std::tuple<pthread_t*, const pthread_attr_t*, Actul::ExecuteFunction, void*> args);
template<>
int PthreadImpl::act<EventType::THREAD_JOIN, int, std::tuple<pthread_t, void **>>(AddressType addr, std::tuple<pthread_t, void **> args);
template<>
void PthreadImpl::act<EventType::THREAD_EXIT, void, void*>(AddressType addr, void * arg);
template<>
void PthreadImpl::act<EventType::THREAD_MAIN_EXIT, void, void*>(AddressType addr, void * arg);

} /* namespace Actul */

#endif /* INCLUDE_IMPLEMENTATION_PTHREADIMPL_H_ */
