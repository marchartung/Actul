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
#include <Logger/Event.h>

namespace Actul
{

template<>
String to_string(const EventType & et)
{
   String res;
   switch (et)
   {
      case EventType::READ:
         res = "READ";
         break;
      case EventType::WRITE:
         res = "WRITE";
         break;
      case EventType::ATOMIC_READ:
         res = "ATOMIC_READ";
         break;
      case EventType::ATOMIC_WRITE:
         res = "ATOMIC_WRITE";
         break;
      case EventType::FUNCTION_ENTRY:
         res = "FUNCTION_ENTRY";
         break;
      case EventType::FUNCTION_EXIT:
         res = "FUNCTION_EXIT";
         break;
      case EventType::START:
         res = "START";
         break;
      case EventType::END:
         res = "END";
         break;
      case EventType::UNDEFINED:
         res = "UNDEFINED";
         break;
      case EventType::MUTEXLOCK_INIT:
         res = "MUTEXLOCK_INIT";
         break;
      case EventType::MUTEXLOCK_DESTROY:
         res = "MUTEXLOCK_DESTROY";
         break;
      case EventType::RWLOCK_INIT:
         res = "RWLOCK_INIT";
         break;
      case EventType::RWLOCK_DESTROY:
         res = "RWLOCK_DESTROY";
         break;
      case EventType::SEM_INIT:
         res = "SEM_INIT";
         break;
      case EventType::SEM_DESTROY:
         res = "SEM_DESTROY";
         break;
      case EventType::COND_INIT:
         res = "COND_INIT";
         break;
      case EventType::COND_DESTROY:
         res = "COND_DESTROY";
         break;
      case EventType::BARRIER_INIT:
         res = "BARRIER_INIT";
         break;
      case EventType::BARRIER_DESTROY:
         res = "BARRIER_DESTROY";
         break;
      case EventType::COND_TIMEDWAIT:
         res = "COND_TIMEDWAIT_FAIL";
         break;
      case EventType::SEM_TRYWAIT:
         res = "SEM_TRYWAIT_SUCC";
         break;
      case EventType::RWTRYLOCK_READ:
         res = "RWTRYLOCK_READ_SUCC";
         break;
      case EventType::RWTRYLOCK_WRITE:
         res = "RWTRYLOCK_WRITE_SUCC";
         break;
      case EventType::MUTEXTRYLOCK:
         res = "MUTEXTRYLOCK";
         break;
      case EventType::MUTEXLOCK:
         res = "MUTEXLOCK";
         break;
      case EventType::MUTEXUNLOCK:
         res = "MUTEXUNLOCK";
         break;
      case EventType::RWLOCK_READ:
         res = "RWLOCK_READ";
         break;
      case EventType::RWLOCK_WRITE:
         res = "RWLOCK_WRITE";
         break;
      case EventType::RWUNLOCK:
         res = "RWUNLOCK";
         break;
      case EventType::SEM_POST:
         res = "SEM_POST";
         break;
      case EventType::SEM_WAIT:
         res = "SEM_WAIT";
         break;
      case EventType::COND_SIGNAL:
         res = "COND_SIGNAL";
         break;
      case EventType::COND_BCAST:
         res = "COND_BCAST";
         break;
      case EventType::COND_WAIT:
         res = "COND_WAIT";
         break;
      case EventType::BARRIER_WAIT:
         res = "BARRIER_WAIT";
         break;
      case EventType::THREAD_CREATE:
         res = "THREAD_CREATE";
         break;
      case EventType::THREAD_JOIN:
         res = "THREAD_JOIN";
         break;
      case EventType::THREAD_EXIT:
         res = "THREAD_EXIT";
         break;
      case EventType::THREAD_START:
         res = "THREAD_START";
         break;
      default:
         res = "UNKNOWN ID=" + to_string(static_cast<int>(et));
   }
   return res;
}

EventType makeAccessType(const bool & isWrite, const bool & isAtomic)
{
   return (isWrite) ? ((isAtomic) ? EventType::ATOMIC_WRITE : EventType::WRITE) : ((isAtomic) ? EventType::ATOMIC_READ : EventType::READ);
}

bool isWrite(const EventType & ET)
{
   return ET == EventType::WRITE || ET == EventType::ATOMIC_WRITE;
}

bool isAtomic(const EventType & ET)
{
   return ET == EventType::ATOMIC_READ || ET == EventType::ATOMIC_WRITE;
}


bool isAcquireLockEvent(const EventType & et)
{
   return et == EventType::MUTEXLOCK || et == EventType::RWLOCK_READ || et ==EventType::RWTRYLOCK_WRITE;
}

bool isLockEvent(const EventType & et)
{
   return static_cast<unsigned short>(et) >= static_cast<unsigned short>(EventType::MUTEXLOCK_INIT)
         && static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::RWUNLOCK);
}

bool isBarrierEvent(const EventType & et)
{
   return static_cast<unsigned short>(et) >= static_cast<unsigned short>(EventType::BARRIER_INIT)
         && static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::BARRIER_WAIT);
}

bool isSemaphoreEvent(const EventType & et)
{
   return static_cast<unsigned short>(et) >= static_cast<unsigned short>(EventType::SEM_INIT) && static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::SEM_WAIT);
}

bool isConditionalEvent(const EventType & et)
{
   return static_cast<unsigned short>(et) >= static_cast<unsigned short>(EventType::COND_INIT)
         && static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::COND_WAIT);
}

bool isAccessEvent(const EventType & et)
{
   return static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::ATOMIC_WRITE);
}

bool isThreadEvent(const EventType & et)
{
   return static_cast<unsigned short>(et) >= static_cast<unsigned short>(EventType::THREAD_CREATE)
         && static_cast<unsigned short>(et) <= static_cast<unsigned short>(EventType::THREAD_START);
}

}
