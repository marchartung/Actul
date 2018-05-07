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
#include <Util/Defines.h>
#include <Util/Utility.h>
#include <Implementation/PthreadImpl.h>
#include <Scheduler/TestScheduler.h>

#include <tuple>
#include <signal.h>

namespace Actul
{
template<>
PthreadImpl * SingletonClass<PthreadImpl>::_instance = nullptr;

bool PthreadImpl::isBlocked(const tid_type & tid) const
{
   actulAssert(tid < _threads.size(), "PthreadImpl: Invalid tid id passed");
   const EventType & et = _eiLogger.getEventType(tid);
   const event_id & sid = _eiLogger.getEventId(tid);
   bool res = false;
   if (isThreadEvent(et))
   {
      for (size_type i = 0; i < _threads.size(); ++i)
         if (_threads[i].getEventId() == sid)
         {
            res = _threads[i].isBlocked(et, tid);  // TODO here is an extra mapping needed, because event_id and threadid can be different
            break;
         }
   } else if (isLockEvent(et))
   {
      res = _locks[sid].isBlocked(et, tid);
   } else if (isBarrierEvent(et))
   {
      res = _barriers[sid].isBlocked(et, tid);
   } else if (isConditionalEvent(et))
   {
      res = _conds[sid].isBlocked(et, tid);
   } else if (isSemaphoreEvent(et))
   {
      res = _sems[sid].isBlocked(et, tid);
   }
   return res;
}

bool PthreadImpl::isCurBlocked() const
{
   return isBlocked(ThreadInfo::getThreadId());
}

const TidVec & PthreadImpl::getRunningThreads() const
{
   return _runningThreads;
}

const TidVec & PthreadImpl::getThreads() const
{
   return _ranThreads;
}

PthreadImpl::BarrierState::BarrierState()
      : _init(-1)
{
   for (size_type i = 0; i < _waiters.size(); ++i)
      _waiters[i] = invalid<tid_type>();
}

bool PthreadImpl::BarrierState::isInitialized() const
{
   return _init > -1;
}

bool PthreadImpl::BarrierState::isFree() const
{
   return !isInitialized() || getNumWaiters() == 0 || getNumWaiters() == _init;
}

bool PthreadImpl::BarrierState::isBlocked(const EventType & et, const tid_type & tid) const
{
   return isInitialized() && isWaiter(tid);
}

void PthreadImpl::BarrierState::initialize(const unsigned & initVal)
{
   _init = initVal;
}

void PthreadImpl::BarrierState::deinitialize()
{
   actulAssert(_init != -1, "BarrierState: Barrier is inconsistent.");
   _init = -1;
}

bool PthreadImpl::BarrierState::isWaiter(const tid_type & tid) const
{
   bool res = false;
   for (size_type i = 0; i < _waiters.size() && _waiters[i] != invalid<tid_type>(); ++i)
      if (_waiters[i] == tid)
      {
         res = true;
         break;
      }

   return res;
}

int PthreadImpl::BarrierState::getNumWaiters() const
{
   int res = 0, end = _waiters.size();
   for (; res < end && _waiters[res] != invalid<tid_type>(); ++res)
      ;
   return res;
}

bool PthreadImpl::BarrierState::tryWait()
{
   size_type curNum = getNumWaiters();
   _waiters[curNum] = ThreadInfo::getThreadId();
   ++curNum;
   return curNum == UAbs(_init);
}

const TidArray & PthreadImpl::BarrierState::getWaiters() const
{
   return _waiters;
}

void PthreadImpl::BarrierState::resetWaiters()
{
   actulAssert(isFree(), "BarrierState: Try to clean a blocked barrier");
   for (size_type i = 0; i < _waiters.size() && _waiters[i] != invalid<tid_type>(); ++i)
      _waiters[i] = invalid<tid_type>();
}

PthreadImpl::CondState::CondState()
      : _init(false),
        _stampCounter(0)
{
}

bool PthreadImpl::CondState::isInitialized() const
{
   return _init;
}

bool PthreadImpl::CondState::canBeDestroyed() const
{
   return _waiters.empty();
}

void PthreadImpl::CondState::initialize(const unsigned & initVal)
{
   actulAssert(!_init, "CondState: Cannot deinitialize uninitialized conditional");
   _init = true;
}

void PthreadImpl::CondState::deinitialize()
{
   actulAssert(_init, "CondState: Cannot deinitialize uninitialized conditional");
   _init = false;
}

void PthreadImpl::CondState::wait(const LockRef & lref, const bool & timed)
{
   actulAssert(_init, "CondState: Cannot wait uninitialized conditional");
   WaiterInfo wi;
   wi.tid = ThreadInfo::getThreadId();
   wi.clockStamp = _stampCounter;
   wi.isTryWait = timed;
   wi.lref = lref;
   _waiters.push_back(wi);
}

bool PthreadImpl::CondState::isBlocked(const EventType & et, const tid_type & tid) const
{
   return (et == EventType::COND_WAIT && (_waiters[getWaiter(tid)].clockStamp >= _stampCounter || _waiters[getWaiter(tid)].lref.getLockState().isBlocked(EventType::MUTEXLOCK, tid)))
         || (et == EventType::COND_TIMEDWAIT && _waiters[getWaiter(tid)].lref.getLockState().isBlocked(EventType::MUTEXLOCK, tid));
}

void PthreadImpl::CondState::signal(const size_type & clockId)
{

   ClockStamps cs;
   cs.isBcast = false;
   cs.clockId = clockId;
   cs.stamp = ++_stampCounter;
   _clockStamps.push_back(cs);
}

void PthreadImpl::CondState::bcast(const size_type & clockId)
{
   ClockStamps cs;
   cs.isBcast = true;
   cs.clockId = clockId;
   cs.stamp = ++_stampCounter;
   _clockStamps.push_back(cs);
}

size_type PthreadImpl::CondState::findClock(const size_type & stamp) const
{
   size_type res = invalid<size_type>();
   for (size_type i = 0; i < _clockStamps.size(); ++i)
      if (_clockStamps[i].stamp > stamp)
      {
         res = i;
         break;
      }

   return res;
}

size_type PthreadImpl::CondState::removeWaiter()
{
   actulAssert(_init, "CondState: Cannot remove waiter from uninitialized conditional");
   const tid_type & tid = ThreadInfo::getThreadId();
   size_type res = invalid<size_type>();

   // find current thread and the end
   size_type wId = getWaiter(tid);
   actulAssert(wId != invalid<size_type>(), "CondState: Couldn't find waiter for remove");
   const size_type & stamp = _waiters[wId].clockStamp;
   actulAssert(stamp < _stampCounter || _waiters[wId].isTryWait, "CondState: Waiter falsely released");
   size_type cId = findClock(stamp);
   if (cId != invalid<size_type>())
   {
      res = _clockStamps[cId].clockId;
      size_type rStamp = _clockStamps[cId].stamp;
      bool isBcast = _clockStamps[cId].isBcast;
      if (!isBcast)
         _clockStamps.remove(cId);
      _waiters.unordered_remove(wId);

      size_type minStamp = rStamp;
      for (size_type i = 0; i < _waiters.size(); ++i)
      {
         auto & w = _waiters[i];
         if (!isBcast && w.clockStamp == rStamp - 1)
            w.clockStamp = rStamp;
         actulAssert(isBcast || w.clockStamp >= rStamp, "CondState: Inconsistent conditional");
         minStamp = Min(w.clockStamp, minStamp);
      }
      for (size_type i = 0; i < _clockStamps.size();)
         if (_clockStamps[i].stamp <= minStamp)
            _clockStamps.remove(i);
         else
            ++i;
   } else
   {
      actulAssert(_waiters[wId].isTryWait, "CondState: Trying to release non timed cond without signal");
      _waiters.unordered_remove(wId);
   }
   return res;
}

size_type PthreadImpl::CondState::getWaiter(const tid_type & tid) const
{
   size_type i = 0;
   for (; i < _waiters.size(); ++i)
      if (_waiters[i].tid == tid)
         break;
   actulAssert(_waiters.size() > i, "CondState: Couldn't find thread in waiters");
   return i;
}

PthreadImpl::LockState::LockState()
      : _state(CLS_NOT_INITIALIIZED_LOCK)
{
   for (size_type i = 0; i < _waiters.size(); ++i)
   {
      _waiters[i] = invalid<tid_type>();
      _owners[i] = invalid<tid_type>();
      _waiterStates[i] = CurrentThreadState::CTS_NONE;
   }
}

bool PthreadImpl::LockState::isBlocked(const EventType & et, const tid_type & tid) const
{
   return _state != CurrentLockState::CLS_NOT_INITIALIIZED_LOCK && isAcquireLockEvent(et)
         && (_state == CurrentLockState::CLS_WRITE_OWNED_LOCK || (_state == CurrentLockState::CLS_READ_OWNED_LOCK && et == EventType::RWTRYLOCK_WRITE));
}

bool PthreadImpl::LockState::isInitialized() const
{
   return _state != CurrentLockState::CLS_NOT_INITIALIIZED_LOCK;
}

bool PthreadImpl::LockState::canBeDestroyed() const
{
   return isFree<true>();
}

bool PthreadImpl::LockState::isOwner() const
{
   actulAssert(_state != CurrentLockState::CLS_NOT_INITIALIIZED_LOCK, "LockState: Cannot determine if an uninitialized lock has an owner");
   return findOwner(ThreadInfo::getThreadId()) != invalid<size_type>();
}

void PthreadImpl::LockState::initialize(const unsigned & initVal)
{
   actulAssert(_state != CurrentLockState::CLS_INITIALIZED_LOCK, "LockState: Cannot initialize a already initialized lock");
   _state = CurrentLockState::CLS_INITIALIZED_LOCK;
}

void PthreadImpl::LockState::deinitialize()
{
   actulAssert(_state == CurrentLockState::CLS_INITIALIZED_LOCK, "LockState: Tried to deinitialize locked lock");
   _state = CurrentLockState::CLS_NOT_INITIALIIZED_LOCK;
}

void PthreadImpl::LockState::unlock()
{
   actulAssert(_state != CurrentLockState::CLS_NOT_INITIALIIZED_LOCK, "LockState: Cannot unlock a uninitialized lock");
   actulAssert(isOwner(), "LockState: Cannot unlock not owned lock");

   const tid_type & tid = ThreadInfo::getThreadId();
   size_type delPos = findOwner(tid), iend = getNumOwners() - 1;
   std::swap(_owners[delPos], _owners[iend]);
   _owners[iend] = invalid<tid_type>();
   if (iend == 0)
   {
      _state = CurrentLockState::CLS_INITIALIZED_LOCK;
   }
}

void PthreadImpl::LockState::removeWaiter()
{
   const tid_type & tid = ThreadInfo::getThreadId();
   size_type delPos = getWaiter(tid), iend = getNumWaiters() - 1;

   std::swap(_waiters[delPos], _waiters[iend]);
   std::swap(_waiterStates[delPos], _waiterStates[iend]);
   _waiters[iend] = invalid<tid_type>();
   _waiterStates[iend] = CurrentThreadState::CTS_NONE;
}

int PthreadImpl::LockState::getNumWaiters() const
{
   int res = 0, end = _waiters.size();
   for (; res < end && _waiters[res] != invalid<tid_type>(); ++res)
      ;
   return res;
}

int PthreadImpl::LockState::getNumOwners() const
{
   int res = 0, end = _owners.size();
   for (; res < end && _owners[res] != invalid<tid_type>(); ++res)
      ;
   return res;
}

size_type PthreadImpl::LockState::getWaiter(const tid_type & tid) const
{
   size_type i = 0;
   for (; i < _waiters.size() && _waiters[i] != invalid<tid_type>(); ++i)
      if (_waiters[i] == tid)
      {
         break;
      }
   actulAssert(_waiters[i] == tid, "CondState: Couldn't find thread in waiters");
   return i;
}

size_type PthreadImpl::LockState::findOwner(const tid_type & tid) const
{
   size_type res = invalid<size_type>();
   for (size_type i = 0; i < _owners.size() && _owners[i] != invalid<tid_type>(); ++i)
      if (_owners[i] == tid)
      {
         res = i;
         break;
      }
   return res;
}

PthreadImpl::ThreadState::ThreadState()
      : _isJoinabled(false),
        _pid(invalid<pthread_t>()),
        _waiter(invalid<tid_type>()),
        _eid(invalid<event_id>()),
        _clockId(invalid<size_type>()),
        _returnVal(NULL),
        _waitsFor(invalid<tid_type>())
{
}

bool PthreadImpl::ThreadState::isBlocked(const EventType & et, const tid_type & tid) const
{
   return (et == EventType::THREAD_MAIN_EXIT || et == EventType::THREAD_JOIN) && _waitsFor[0] != invalid<tid_type>();
}

bool PthreadImpl::ThreadState::isJoinable() const
{
   return _isJoinabled;
}

const pthread_t & PthreadImpl::ThreadState::getPid() const
{
   return _pid;
}

bool PthreadImpl::ThreadState::hasWaiter() const
{
   return _waiter != invalid<tid_type>();
}


bool PthreadImpl::ThreadState::isWaiting() const
{
   return _waitsFor[0] != invalid<tid_type>();
}

bool PthreadImpl::ThreadState::isUnused() const
{
   return _pid == invalid<pthread_t>();
}

const event_id & PthreadImpl::ThreadState::getEventId() const
{
   return _eid;
}

void PthreadImpl::ThreadState::create(const pthread_t & pid, const event_id & id, const bool & joinable)
{
   actulAssert(isUnused(), "Cannot create thread on the same id twice at the same time");
   _isJoinabled = joinable;
   _eid = id;
   _pid = pid;
}

void PthreadImpl::ThreadState::goingToJoin(ThreadStateArray & tsa)
{
   actulAssert(isJoinable(), "ThreadState: Cannot wait for a not joinable thread");
   actulAssert(!hasWaiter(), "ThreadState: Two threads cannot join with the same thread");
   const tid_type & tid = ThreadInfo::getThreadId();
   _waiter = tid;
}

void * PthreadImpl::ThreadState::getReturnVal() const
{
   return _returnVal;
}
const size_type & PthreadImpl::ThreadState::getClockId() const
{
   actulAssert(_clockId != invalid<size_type>(), "ThreadState: Returning invalid clock ID");
   return _clockId;
}

bool PthreadImpl::ThreadState::hasExited() const
{
   return _clockId != invalid<size_type>();
}

void PthreadImpl::ThreadState::exit(ThreadStateArray & tsa, const size_type & clockId, void * returnVal)
{
   const tid_type & tid = ThreadInfo::getThreadId();
   for (size_type i = 0; i < tsa.size(); ++i)
   {
      ThreadState & t = tsa[i];
      if (t._waitsFor[0] != invalid<tid_type>())
      {
         TidArray newWait(invalid<tid_type>());
         TidArray & old = t._waitsFor;
         for (size_type j = 0, k = 0; j < old.size(); ++j)
         {
            if (tid != old[j])
            {
               newWait[k] = old[j];
               k++;
            }
            if (old[j] == invalid<size_type>())
               break;
         }
         old = newWait;
      }
   }

   _clockId = clockId;
   if (isJoinable())
   {
      _returnVal = returnVal;
   }
}

void PthreadImpl::ThreadState::join()
{
   actulAssert(_waiter == ThreadInfo::getThreadId(), "ThreadState: Different waiter assigned");
   actulAssert(hasExited(), "ThreadState: Cannot join if thread has not exited");
   // set ThreadState back to default:
   _isJoinabled = false;
   _pid = invalid<pthread_t>();
   _waiter = invalid<tid_type>();
   _clockId = invalid<size_type>();
}

void PthreadImpl::ThreadState::waitsForThreads(const TidVec & tids)
{
   actulAssert(tids.size() > 0, "ThreadState: Cannot wait for no thread");
   actulAssert(_waitsFor[0] == invalid<tid_type>(), "ThreadState: Already waits for threads to finish");
   for (size_type i = 0, j = 0; i < tids.size(); ++i)
      if (tids[i] != ThreadInfo::getThreadId())
         _waitsFor[j++] = tids[i];
}
void PthreadImpl::ThreadState::waitsForThread(const tid_type & tid)
{
   actulAssert(tid != invalid<size_type>(), "ThreadState: Cannot wait for no thread");
   actulAssert(_waitsFor[0] == invalid<tid_type>(), "ThreadState: Already waits for threads to finish");
   _waitsFor[0] = tid;
}


PthreadImpl::PthreadImpl()
      : _lsLogger(*LockSetLogger::getInstance()),
        _hbLogger(*HbLogger::getInstance()),
        _eiLogger(*EventLogger::getInstance()),
        _yieLogger(*YieldLogger::getInstance())
{
   const event_id & eid = ActulLogger::notifySyncEventPre<EventType::THREAD_CREATE>(reinterpret_cast<const AddressType>(&ThreadInfo::getThreadId()));
   actulAssert(eid == 0, "PthreadImpl: maybe an error :-/, better caught than missed");
   _threads[0].create(0, eid, false);
   _runningThreads.push_back(0);
   _ranThreads.push_back(0);
   ActulLogger::notifySyncEventPost<EventType::THREAD_CREATE>(reinterpret_cast<const AddressType>(&ThreadInfo::getThreadId()),eid);
}

tid_type PthreadImpl::findTidOfPid(const pthread_t & pid) const
{
   tid_type res = invalid<tid_type>();
   for (tid_type i = 0; i < _threads.size(); ++i)
      if (_threads[i].getPid() == pid)
      {
         res = i;
         break;
      }
   return res;
}

tid_type PthreadImpl::findUnusedThreadId() const
{
   tid_type res = invalid<tid_type>();
   for (tid_type i = 0; i < _threads.size(); ++i)
      if (_threads[i].isUnused())
      {
         res = i;
         break;
      }
   return res;
}

struct WrapperData
{
   ExecuteFunction callback;
   void* arg;
   tid_type tid;
   tid_type tidCreator;
   size_type clockId;
   WrapperData(ExecuteFunction f, void * arg, const tid_type & tid, const tid_type & creator, const size_type & clockId)
         : callback(f),
           arg(arg),
           tid(tid),
           tidCreator(creator),
           clockId(clockId)
   {
   }
};

void * pthread_starte_func(void * data)
{
   return PthreadImpl::getInstance()->threadStart(data);
}

void* PthreadImpl::threadStart(void* arg)
{
   WrapperData & wrap = *reinterpret_cast<WrapperData*>(arg);
   ThreadInfo::setThreadId(wrap.tid);
   const event_id & eid = ActulLogger::notifySyncEventPre<EventType::THREAD_START>(&ThreadInfo::getThreadId());
   _hbLogger.curThreadHappendAfterClock(wrap.clockId);
   _yieLogger.init();
   _yieLogger.yield();
   ActulLogger::notifySyncEventPost<EventType::THREAD_START>(&(ThreadInfo::getThreadId()), eid);

   void* stdCall = (wrap.callback)(wrap.arg);
   ThreadInfo::setInScheduler(true);
   deleteInstance<WrapperData>(&wrap);
   ThreadInfo::setInScheduler(false);
   act<Actul::EventType::THREAD_EXIT, void, void*>(NULL, nullptr);
   return stdCall;
}

template<>
int PthreadImpl::act<EventType::THREAD_CREATE, int, std::tuple<pthread_t*, const pthread_attr_t*, Actul::ExecuteFunction, void*>>(
      AddressType addr, std::tuple<pthread_t*, const pthread_attr_t*, Actul::ExecuteFunction, void*> args)
{
   if (!TestScheduler::isAllocated())
   {
      const event_id & id = ActulLogger::notifySyncEventPre<EventType::THREAD_MAIN_START>(&ThreadInfo::getThreadId());
      TestScheduler::allocateInstance();
      ActulLogger::notifySyncEventPost<EventType::THREAD_MAIN_START>(&ThreadInfo::getThreadId(), id);
   }
   pthread_t * & thread = std::get<0>(args);
   const pthread_attr_t * attr = std::get<1>(args);
   ExecuteFunction & start_routine = std::get<2>(args);
   int res = EAGAIN;
   tid_type newThreadId = findUnusedThreadId();
   if (ProcessInfo::checkProgramProperty(newThreadId != invalid<tid_type>(), FailedProperty::NUM_THREADS_SUCCEEDED))
   {
      res = EINVAL;
      bool isJoinable = true;
      int detachMode = 0;
      if (attr != 0)
      {
         if (InternalPthread_attr_getdetachstate(attr, &detachMode) == 0)
         {
            isJoinable = (detachMode == PTHREAD_CREATE_JOINABLE);
            detachMode = 0;
         } else
            detachMode = invalid<int>();
      }

      if (detachMode != invalid<int>())
      {  // everything is ok and thread will be created

         const event_id & id = ActulLogger::notifySyncEventPre<EventType::THREAD_CREATE>((AddressType) (&ThreadInfo::getThreadId()));
         const tid_type & tid = ThreadInfo::getThreadId();
         WrapperData * wData = newInstance<WrapperData>(start_routine, std::get<3>(args), newThreadId, tid, _hbLogger.getCurClockId());
         _hbLogger.curThreadTickClock();
         res = InternalPthread_create(thread, attr, pthread_starte_func, reinterpret_cast<void *>(wData));
         actulAssert(res == 0, "PthreadImpl: Could not create thread");
         _runningThreads.push_back(newThreadId);
         if (_ranThreads.find(newThreadId) == invalid<size_type>())
            _ranThreads.push_back(newThreadId);
         _yieLogger.waitForThreadToYield(newThreadId);  // ensure that created thread is locked
         ThreadState & newTs = _threads[newThreadId];
         newTs.create(*thread, id, isJoinable);

         //actulAssert(id == newThreadId, "PthreadImpl: EventId of new thread is not the thread id");
         ActulLogger::callScheduler<EventType::THREAD_CREATE>(false, id);
         ActulLogger::notifySyncEventPost<EventType::THREAD_CREATE>(&ThreadInfo::getThreadId(), id);
      }
   }
   return res;
}

template<>
int PthreadImpl::act<EventType::THREAD_JOIN, int, std::tuple<pthread_t, void **>>(AddressType addr, std::tuple<pthread_t, void **> args)
{
   pthread_t pid = std::get<0>(args);
   void ** retval = std::get<1>(args);
   int res = ESRCH;
   tid_type jTid = findTidOfPid(pid);
   const event_id & id = ActulLogger::notifySyncEventPre<EventType::THREAD_JOIN>(&ThreadInfo::getThreadId());
   const tid_type & tid = ThreadInfo::getThreadId();
   if (ProcessInfo::checkProgramProperty(jTid != invalid<tid_type>(), FailedProperty::NO_SUCH_PID))
   {
      ThreadState & otherTs = _threads[jTid];
      ThreadState & thisThread = _threads[tid];
      if (ProcessInfo::checkProgramProperty(_threads[jTid].isJoinable(), FailedProperty::THREAD_NOT_JOINABLE))
      {
         otherTs.goingToJoin(_threads);
         if(!otherTs.hasExited())
            thisThread.waitsForThread(jTid);
         ActulLogger::callScheduler<EventType::THREAD_JOIN>(thisThread.isBlocked(EventType::THREAD_JOIN, tid), id);
         actulAssert(!thisThread.isWaiting(), "PthreadImpl: Thread is still waiting");
         actulAssert(otherTs.hasExited(), "PthreadImpl: Cannot join not-exited thread");
         _hbLogger.curThreadHappendAfterClock(otherTs.getClockId());
         pthread_join(pid, retval);
         if (retval != NULL)
            *retval = otherTs.getReturnVal();
         otherTs.join();

         res = 0;
      } else
         res = EINVAL;
   }
   ActulLogger::notifySyncEventPost<EventType::THREAD_JOIN>(&ThreadInfo::getThreadId(), id);
   return res;
}

template<>
void PthreadImpl::act<EventType::THREAD_EXIT, void, void*>(AddressType addr, void * arg)
{
   const tid_type & tid = ThreadInfo::getThreadId();
   ThreadState & ts = _threads[tid];
   const event_id & id = ActulLogger::notifySyncEventPre<EventType::THREAD_EXIT>(&ThreadInfo::getThreadId());
   //actulAssert(tid == id, "PthreadImpl: inconsistent event id during thread exit");
   const size_type & clockId = _hbLogger.getCurClockId();
   _hbLogger.curThreadTickClock();

   ts.exit(_threads, clockId, arg);
   for (size_type i = 0; i < _runningThreads.size(); ++i)
      if (_runningThreads[i] == tid)
      {
         _runningThreads.unordered_remove(i);
         break;
      }
   ActulLogger::callExit();
   ActulLogger::notifySyncEventPost<EventType::THREAD_EXIT>(&ThreadInfo::getThreadId(), id);
   _yieLogger.deinit();
   actulAssert(!ThreadInfo::isInScheduler(), "Wrong exit");
   pthread_exit(arg);
}

template<>
void PthreadImpl::act<EventType::THREAD_MAIN_EXIT, void, void*>(AddressType addr, void * arg)
{

   if (getRunningThreads().size() > 1 && !ProcessInfo::hasAssertion())
   {
      const tid_type & tid = ThreadInfo::getThreadId();
      ThreadState & ts = _threads[tid];
      const event_id & id = ActulLogger::notifySyncEventPre<EventType::THREAD_MAIN_EXIT>(&ThreadInfo::getThreadId());
      ts.waitsForThreads(getRunningThreads());
      ActulLogger::callScheduler<EventType::THREAD_MAIN_EXIT>(true, id);
      actulAssert(!ts.isBlocked(EventType::THREAD_MAIN_EXIT, id), "Cannot release blocked thread");
      ActulLogger::notifySyncEventPost<EventType::THREAD_MAIN_EXIT>(&ThreadInfo::getThreadId(), id);
   }
}

template<>
bool PthreadImpl::isInitialized<PthreadImpl::BarrierState>(const event_id & id)
{
   return _barriers.size() > id && _barriers[id].isInitialized();
}

template<>
bool PthreadImpl::isInitialized<PthreadImpl::CondState>(const event_id & id)
{
   return _conds.size() > id && _conds[id].isInitialized();
}

template<>
bool PthreadImpl::isInitialized<PthreadImpl::LockState>(const event_id & id)
{
   return _locks.size() > id && _locks[id].isInitialized();
}

template<>
bool PthreadImpl::isInitialized<PthreadImpl::SemStateVec>(const event_id & id)
{
   return _sems.size() > id && _sems[id].isInitialized();
}

template<>
int PthreadImpl::internalAct<EventType::BARRIER_WAIT, int, void*>(const event_id & id, AddressType addr, void * dummy)
{
   int res = EINVAL;
   if (ProcessInfo::checkProgramProperty(isInitialized<BarrierState>(id), FailedProperty::ACCESS_UNINITIALIZED_BARRIER))
   {

      bool wasLastWait = _barriers[id].tryWait();
      if (wasLastWait)
      {
         const TidArray & ws = _barriers[id].getWaiters();
         _hbLogger.barrierOnThreads(ws);
         _barriers[id].resetWaiters();
         res = PTHREAD_BARRIER_SERIAL_THREAD;
      } else
         res = 0;
      ActulLogger::callScheduler<EventType::BARRIER_WAIT>(!wasLastWait, id);
      actulAssert(!isBlocked(ThreadInfo::getThreadId()), "PthreadImpl: Scheduler release blocked thread.");
   }
   return res;
}

template<>
int PthreadImpl::internalAct<EventType::COND_WAIT, int, pthread_mutex_t*>(const event_id & id, AddressType addr, pthread_mutex_t * m)
{
   int res = EINVAL;
   const event_id & mId = _eiLogger.getEventId<EventType::MUTEXLOCK>(0, m);
   bool isCorrect = ProcessInfo::checkProgramProperty(isInitialized<CondState>(id), FailedProperty::ACCESS_UNINITIALIZED_CONDITIONAL);
   isCorrect &= ProcessInfo::checkProgramProperty(isInitialized<LockState>(mId), FailedProperty::ACCESS_UNINITIALIZED_LOCK);
   bool lockOwned = ProcessInfo::checkProgramProperty(_locks[mId].isOwner(), FailedProperty::NOT_OWNED_COND_LOCK);

   if (isCorrect && lockOwned)
   {
      CondState & cs = _conds[id];
      LockState & ls = _locks[mId];
      res = 0;
      cs.wait(LockRef(mId, _locks), false);
      ls.unlock();
      _lsLogger.removeLock(mId);
      ActulLogger::callScheduler<EventType::COND_WAIT>(true, id);
      actulAssert(!isBlocked(ThreadInfo::getThreadId()) && _locks[mId].isFree<true>(), "PthreadImpl: Thread releases blocked thread");

      size_type clockId = cs.removeWaiter();
      if (clockId != invalid<size_type>())
         _hbLogger.curThreadHappendAfterClock(clockId);
      else
         actulAssert(false, "PthreadImpl: Impl assumed timed conditional, but it wasn't");
      ls.addWaiter<true, false>();
      _locks[mId].lock<true>();
      _lsLogger.addLock<true>(mId);
   } else if (!lockOwned)
   {
      res = EPERM;
   }
   return res;
}

template<>
int PthreadImpl::internalAct<EventType::COND_TIMEDWAIT, int, pthread_mutex_t*>(const event_id & id, AddressType addr, pthread_mutex_t * m)
{
   int res = EINVAL;
   const event_id & mId = _eiLogger.getEventId<EventType::MUTEXLOCK>(0, m);
   bool isCorrect = ProcessInfo::checkProgramProperty(isInitialized<CondState>(id), FailedProperty::ACCESS_UNINITIALIZED_CONDITIONAL);
   isCorrect &= ProcessInfo::checkProgramProperty(isInitialized<LockState>(mId), FailedProperty::ACCESS_UNINITIALIZED_LOCK);
   bool lockOwned = ProcessInfo::checkProgramProperty(_locks[mId].isOwner(), FailedProperty::NOT_OWNED_COND_LOCK);

   if (isCorrect && lockOwned)
   {
      res = 0;
      _conds[id].wait(LockRef(mId, _locks), true);
      _locks[mId].unlock();
      _lsLogger.removeLock(mId);
      ActulLogger::callScheduler<EventType::COND_TIMEDWAIT>(true, id);
      if (ProcessInfo::checkProgramProperty(isInitialized<CondState>(id), FailedProperty::ACCESS_UNINITIALIZED_CONDITIONAL)
            && ProcessInfo::checkProgramProperty(isInitialized<LockState>(mId), FailedProperty::ACCESS_UNINITIALIZED_LOCK))
      {
         res = ETIMEDOUT;
         actulAssert(!isBlocked(ThreadInfo::getThreadId()), "PthreadImpl: Thread releases blocked thread");
         size_type clockId = _conds[id].removeWaiter();
         if (clockId != invalid<size_type>())
         {
            _hbLogger.curThreadHappendAfterClock(clockId);
            res = 0;
         }
         _locks[mId].addWaiter<true, false>();
         _locks[mId].lock<true>();
         _lsLogger.addLock<true>(mId);
      }
   } else if (!lockOwned)
   {
      res = EPERM;
   }
   return res;
}

template<>
int PthreadImpl::internalAct<EventType::COND_SIGNAL, int, void*>(const event_id & id, AddressType addr, void * dummy)
{
   int res = EINVAL;
   if (ProcessInfo::checkProgramProperty(isInitialized<CondState>(id), FailedProperty::ACCESS_UNINITIALIZED_CONDITIONAL))
   {
      res = 0;
      ActulLogger::callScheduler<EventType::COND_SIGNAL>(false, id);

      size_type clockId = _hbLogger.getCurClockId();
      _hbLogger.curThreadTickClock();
      _conds[id].signal(clockId);
   }
   return res;
}

template<>
int PthreadImpl::internalAct<EventType::COND_BCAST, int, void*>(const event_id & id, AddressType addr, void * dummy)
{
   int res = EINVAL;
   if (ProcessInfo::checkProgramProperty(isInitialized<CondState>(id), FailedProperty::ACCESS_UNINITIALIZED_CONDITIONAL))
   {
      res = 0;
      ActulLogger::callScheduler<EventType::COND_BCAST>(false, id);

      size_type clockId = _hbLogger.getCurClockId();
      _hbLogger.curThreadTickClock();
      _conds[id].bcast(clockId);
   }
   return res;
}

template<>
int PthreadImpl::internalAct<EventType::MUTEXLOCK, int, void*>(const event_id & id, AddressType addr, void * dummy)
{
   return pthread_lock<EventType::MUTEXLOCK, true, false>(id);
}
template<>
int PthreadImpl::internalAct<EventType::MUTEXTRYLOCK, int, void*>(const event_id & id, AddressType addr, void * dummy)
{
   return pthread_lock<EventType::MUTEXTRYLOCK, true, true>(id);
}
template<>
int PthreadImpl::internalAct<EventType::RWLOCK_WRITE, int, void*>(const event_id & id, AddressType addr, void * dummy)
{
   return pthread_lock<EventType::RWLOCK_WRITE, true, false>(id);
}
template<>
int PthreadImpl::internalAct<EventType::RWTRYLOCK_WRITE, int, void*>(const event_id & id, AddressType addr, void * dummy)
{
   return pthread_lock<EventType::RWTRYLOCK_WRITE, true, true>(id);
}
template<>
int PthreadImpl::internalAct<EventType::RWTRYLOCK_READ, int, void*>(const event_id & id, AddressType addr, void * dummy)
{
   return pthread_lock<EventType::RWTRYLOCK_READ, false, true>(id);
}
template<>
int PthreadImpl::internalAct<EventType::RWLOCK_READ, int, void*>(const event_id & id, AddressType addr, void * dummy)
{
   return pthread_lock<EventType::RWLOCK_READ, false, false>(id);
}
template<>
int PthreadImpl::internalAct<EventType::MUTEXUNLOCK, int, void*>(const event_id & id, AddressType addr, void * dummy)
{
   return pthread_unlock<EventType::MUTEXUNLOCK>(id);
}
template<>
int PthreadImpl::internalAct<EventType::RWUNLOCK, int, void*>(const event_id & id, AddressType addr, void * dummy)
{
   return pthread_unlock<EventType::RWUNLOCK>(id);
}

} /* namespace Actul */
