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
#ifndef INCLUDE_LOGGER_EVENTLOGGER_H_
#define INCLUDE_LOGGER_EVENTLOGGER_H_

#include <Util/TypeDefs.h>
#include <Util/Defines.h>
#include <Util/LibcAllocator.h>
#include <Util/SingletonClass.h>
#include <Util/Container/AddressMap.h>
#include <Util/Container/Array.h>
#include <Logger/Event.h>
#include <Implementation/FunctionStackImpl.h>

#include <unordered_map>

namespace Actul
{

class EventLogger : public SingletonClass<EventLogger>
{
   friend class SingletonClass<EventLogger> ;

   EventLogger(EventLogger&&) = delete;
   EventLogger(const EventLogger&) = delete;

   struct ThreadEventEntry
   {
      EventType type;
      event_id id;
      uint32_t counter;
   };
   typedef Array<ThreadEventEntry, ACTUL_MAX_NUM_THREADS> ThreadEventEntryArray;
 public:

   template<EventType ET>
   const event_id & notifyEventPre(const unsigned long & width, const AddressType & addr)
   {
      ThreadInfo::setInScheduler(true);
      const tid_type & tid = ThreadInfo::getThreadId();
      event_id & res = _threadEntries[tid].id;

      if (isThreadEvent<ET>())
         res = tid;
      else
         res = getEventId<ET>(width, addr);

      _threadEntries[tid].type = ET;
      return res;
   }

   template<EventType ET>
   void addInstance()
   {
      if (isAccessEvent<ET>())
         _memoryMap.addInstance<ET>();
   }

   template<EventType ET>
   const event_id & getEventId(const unsigned long & width, const AddressType & addr)
   {
      if (isAccessEvent<ET>())
         return _memoryMap.get<ET>(width, addr);
      if (isFunctionEvent<ET>())
         return invalid<event_id>();
      else if (isBarrierEvent<ET>())
         return _barrierMap.get(addr);
      else if (isConditionalEvent<ET>())
         return _condMap.get(addr);
      else if (isLockEvent<ET>())
         return _lockMap.get(addr);
      else if (isSemaphoreEvent<ET>())
         return _semMap.get(addr);
      else if (isHeapEvent<ET>())
         return _heapMap.get(addr);
      else
      {
         actulAssert(false, "EventIdLogger: Unknown event to retrieve id.");
         return invalid<event_id>();
      }
   }

   template<EventType ET>
   void notifyEventPost(const AddressType & addr, const event_id & eid)
   {
      const tid_type & tid = ThreadInfo::getThreadId();
      actulAssert(_threadEntries[tid].id == eid && _threadEntries[tid].type == ET, "EventIdLogger: event id or event type changed between pre and post");
      _threadEntries[tid].id = invalid<event_id>();
      _threadEntries[tid].type = EventType::NO_EVENT;
      ThreadInfo::setInScheduler(false);
   }

   template<EventType ET>
   const EventInstanceVec & getEventInstances(const event_id & id, const tid_type & tid) const
   {

      actulAssert(isAccessEvent<ET>(), "EventLogger: getEventInstances only supported for memory events");

      return _memoryMap.getInstances(id, tid);
   }

   template<EventType ET>
   size_type getCurInstanceNum() const
   {
      return getCurInstanceNum<ET>(ThreadInfo::getThreadId());
   }

   template<EventType ET>
   size_type getCurInstanceNum(const tid_type & tid) const
   {
      if (isAccessEvent<ET>())
         return _memoryMap.getNumCurInstances(tid);
      else
         actulAssert(false, "EventLogger: getCurInstanceNum only supported for memory events");
      return invalid<size_type>();
   }

   template<EventType ET>
   AddressType getCurAddress() const
   {
      if (isAccessEvent<ET>())
         return _memoryMap.getCurAddress(ThreadInfo::getThreadId());
      else if (isLockEvent<ET>())
         return _lockMap.getCurAddress(ThreadInfo::getThreadId());
      actulAssert(false, "EventLogger: getCurInstanceNum only supported for memory events");
      return NULL;
   }

   template<EventType ET>
   size_type getNumInstances(const event_id & id, const tid_type & tid, const size_type & instanceNum = invalid<size_type>()) const
   {
      if (isAccessEvent<ET>())
         return _memoryMap.getNumInstances(id, tid, instanceNum);
      else
         actulAssert(false, "EventLogger: getCurInstanceNum only supported for memory events");
      return invalid<size_type>();
   }

   event_id addEvent(const EventType & et, const unsigned long & width, const AddressType & addr)
   {
      if (isAccessEvent(et))
         return _memoryMap.add(et, width, addr);
      else
      {
         actulAssert(false, "EventLogger: Adding non access events before test run start is not implemented");
         return invalid<event_id>();
      }
   }

   const event_id & getEventId(const tid_type & tid) const
   {
      return _threadEntries[tid].id;
   }

   const event_id & getCurEventId() const
   {
      return getEventId(ThreadInfo::getThreadId());
   }

   const EventType & getEventType(const tid_type & tid) const
   {
      return _threadEntries[tid].type;
   }

   const EventType & getCurEventType() const
   {
      return getEventType(ThreadInfo::getThreadId());
   }

   const AddressMemoryEventKeyVec & getCachedMemoryEvents(const AddressType & addr) const
   {
      return _memoryMap.getCurCachedMemoryEvents(addr);
   }

   const AddressMemoryEventKey & getCachedAccess(const AddressType & addr) const
   {
      return _memoryMap.getCurAccess(addr);
   }
   const EventInstance & getCachedInstance(const AddressType & addr) const
   {
      return _memoryMap.getCurCachedInstance(addr);
   }

 private:
   ThreadEventEntryArray _threadEntries;
   MemoryAddressMap _memoryMap;
   SimpleAddressMap _barrierMap;
   SimpleAddressMap _condMap;
   SimpleAddressMap _heapMap;
   SimpleAddressMap _lockMap;
   SimpleAddressMap _semMap;
   SimpleAddressMap _threadMap;

   EventLogger();

};

}
/* namespace Actul */

#endif /* INCLUDE_LOGGER_EVENTLOGGER_H_ */
