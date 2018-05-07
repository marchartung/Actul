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
#ifndef EVENTS_HPP_
#define EVENTS_HPP_

#include <Util/Defines.h>
#include <Util/String.h>
#include <Util/TypeDefs.h>
#include <Util/Container/Array.h>

namespace Actul
{

enum EventType
{
   READ,  //dont move
   WRITE,  //dont move
   ATOMIC_READ,  //dont move
   ATOMIC_WRITE,  //dont move
   HEAP_ALLOCATION,
   HEAP_FREE,

   FUNCTION_ENTRY,  //Only insert after if function event
   FUNCTION_SINGLE,
   FUNCTION_EXIT,  //Only insert before if function event

   START,  //Only insert after
   END,
   ASSERT,
   UNDEFINED,  //Only insert before

   MUTEXLOCK_INIT,  // only insert after
   MUTEXLOCK_DESTROY,
   RWLOCK_INIT,
   RWLOCK_DESTROY,
   RWTRYLOCK_READ,
   RWTRYLOCK_WRITE,
   MUTEXTRYLOCK,
   MUTEXLOCK,
   MUTEXUNLOCK,
   RWLOCK_READ,
   RWLOCK_WRITE,
   RWUNLOCK,  //Only insert before

   SEM_INIT,  //Only insert after
   SEM_DESTROY,
   SEM_TRYWAIT,
   SEM_POST,
   SEM_WAIT,  //Only insert before

   COND_INIT,  //Only insert after
   COND_DESTROY,
   COND_TIMEDWAIT,
   COND_SIGNAL,
   COND_BCAST,
   COND_WAIT,  //Only insert before

   BARRIER_INIT,  //Only insert after
   BARRIER_DESTROY,
   BARRIER_WAIT,  // only insert before

   THREAD_CREATE,  //Only insert after if thread event
   THREAD_JOIN,
   THREAD_EXIT,
   THREAD_MAIN_START,
   THREAD_MAIN_EXIT,
   THREAD_START,  //Only insert before if thread event

   NO_EVENT
};
typedef Array<EventType,ACTUL_MAX_NUM_THREADS> ThreadEventTypeArray;

EventType makeAccessType(const bool & isWrite, const bool & isAtomic);

template<bool isWrite, bool isAtomic>
constexpr EventType makeAccessType()
{
   return (isWrite) ? ((isAtomic) ? EventType::ATOMIC_WRITE : EventType::WRITE) : ((isAtomic) ? EventType::ATOMIC_READ : EventType::READ);
}

template<>
String to_string(const EventType & et);

template<EventType et>
constexpr bool eq(EventType in)
{
   return et == in;
}
template<EventType ET>
constexpr bool isWrite()
{
    return eq<ET>(EventType::WRITE) || eq<ET>(EventType::ATOMIC_WRITE);
}

template<EventType ET>
constexpr bool isRead()
{
    return eq<ET>(EventType::READ) || eq<ET>(EventType::ATOMIC_READ);
}

bool isWrite(const EventType & ET);

template<EventType ET>
constexpr bool isAtomic()
{
   return eq<ET>(EventType::ATOMIC_READ) || eq<ET>(EventType::ATOMIC_WRITE);
}

bool isAtomic(const EventType & et);

bool isAcquireLockEvent(const EventType & et);

template<EventType et>
constexpr bool isInitEvent()
{
   return eq<et>(EventType::MUTEXLOCK_INIT) || eq<et>(EventType::RWLOCK_INIT) || eq<et>(EventType::SEM_INIT) || eq<et>(EventType::COND_INIT) || eq<et>(EventType::BARRIER_INIT);
}
template<EventType et>
constexpr bool isLockEvent()
{
   return static_cast<unsigned short>(et) >= static_cast<unsigned short>(EventType::MUTEXLOCK_INIT)
         && static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::RWUNLOCK);
}

template<EventType et>
constexpr bool isMutexLock()
{
   return et == EventType::MUTEXLOCK;
}

bool isLockEvent(const EventType & et);

template<EventType et>
constexpr bool isBarrierEvent()
{
   return static_cast<unsigned short>(et) >= static_cast<unsigned short>(EventType::BARRIER_INIT)
         && static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::BARRIER_WAIT);
}

bool isBarrierEvent(const EventType & et);

template<EventType et>
constexpr bool isConditionalEvent()
{
   return static_cast<unsigned short>(et) >= static_cast<unsigned short>(EventType::COND_INIT)
         && static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::COND_WAIT);
}

bool isConditionalEvent(const EventType & et);

template<EventType et>
constexpr bool isSemaphoreEvent()
{
   return static_cast<unsigned short>(et) >= static_cast<unsigned short>(EventType::SEM_INIT) && static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::SEM_WAIT);
}

bool isSemaphoreEvent(const EventType & et);

template<EventType et>
constexpr bool isSyncEvent()
{
   return static_cast<unsigned short>(et) >= static_cast<unsigned short>(EventType::MUTEXLOCK_INIT)
         && static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::THREAD_START);
}

template<EventType et>
constexpr bool isThreadEvent()
{
   return static_cast<unsigned short>(et) >= static_cast<unsigned short>(EventType::THREAD_CREATE)
         && static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::THREAD_START);
}

bool isThreadEvent(const EventType & et);

template<EventType et>
constexpr bool isFunctionEvent()
{
   return static_cast<unsigned short>(et) >= static_cast<unsigned short>(EventType::FUNCTION_ENTRY)
         && static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::FUNCTION_EXIT);
}

template<EventType et>
constexpr bool isAccessEvent()
{
   return static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::ATOMIC_WRITE);
}

bool isAccessEvent(const EventType & et);
template<EventType et>
constexpr bool isDefaultYieldEvent()
{
   return (isSyncEvent<et>() && !isInitEvent<et>()) || isThreadEvent<et>();
}


template<EventType et>
constexpr bool isHeapEvent()
{
   return EventType::HEAP_ALLOCATION == et || EventType::HEAP_FREE == et;
}

}

#endif /* EVENTS_HPP_ */
