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
#ifndef INCLUDE_LOGGER_ACTULLOGGER_H_
#define INCLUDE_LOGGER_ACTULLOGGER_H_

#include <Logger/Event.h>
#include <Logger/ConstrainLogger.h>
#include <Logger/DataRaceLogger.h>
#include <Logger/EventLogger.h>
#include <Scheduler/TestScheduler.h>
#include <Util/TypeDefs.h>
#include <utility>

namespace Actul
{

class ActulLogger
{
 public:
   /**
    * Used for memory access, function access/exits, and heap allocations/free
    */
   template<EventType ET>
   static void notifyAccessEvent(const unsigned long & width, const AddressType & addr)
   {
      static_assert(isAccessEvent<ET>(),"notifyAccessEvent can only be called for direct memory accesses");
      if (TestScheduler::isAllocated() && !ThreadInfo::isInScheduler())  // only activates, when multiple threads are running
      {
         ThreadInfo::increaceNumAccesses();
         TestScheduler & ts = *TestScheduler::peekInstance();
         ConstrainLogger & cl = ts.getConstrainLogger();
         DataRaceLogger & dl = ts.getDataRaceLogger();
         EventLogger & el = *EventLogger::getInstance();
         // log memory access and get event id
         const event_id & eId = el.notifyEventPre<ET>(width, addr);  // careful: event_ids depend on ET, so different ETs can have the same id

         cl.notifyEventPre<ET>(eId);
         // 4. check if scheduler should be called
         callScheduler<ET>(false, eId);

         el.addInstance<ET>();
         dl.findDataNewDataRaces(eId, addr);

         cl.notifyEventPost<ET>(eId);
         el.notifyEventPost<ET>(addr, eId);
      }
   }

   template<EventType ET>
   static const event_id & notifySyncEventPre(const AddressType & addr)
   {
      EventLogger & el = *EventLogger::getInstance();
      const event_id & res = el.notifyEventPre<ET>(0, addr);  // careful: event_ids depend on ET, so different ETs can have the same id
      actulAssert(res != invalid<event_id>(), "EventLogger: Sync event has no id");
      if (TestScheduler::isAllocated())  // only activates, when multiple threads are running
      {
         TestScheduler & ts = *TestScheduler::peekInstance();
         ConstrainLogger & cl = ts.getConstrainLogger();
         cl.notifyEventPre<ET>(res);
      }

      return res;
   }

   template<EventType ET>
   static void callScheduler(const bool & isBlocked, const event_id & eid)
   {
      // 0. check if scheduler is active i.e. pthread_create was called
      if (TestScheduler::isAllocated())  // only activates, when multiple threads are running
      {
         TestScheduler & ts = *TestScheduler::peekInstance();
         EventLogger & el = *EventLogger::getInstance();
         actulAssert(el.getCurEventId() == eid, "EventLogger: Current event_id wasn't set");
         actulAssert(el.getCurEventType() == ET, "EventLogger: Current EventType wasn't set");

         ts.getThreadScheduler().call<ET>();
      }
   }

   static void callExit()
   {
      if (TestScheduler::isAllocated())  // only activates, when multiple threads are running
      {
         TestScheduler & ts = *TestScheduler::peekInstance();
         ts.getThreadScheduler().treatExitThread();
      }
      else
         actulAssert(false, "Cannot call exit when Testscheduler was never initialized");
   }

   template<EventType ET>
   static void notifySyncEventPost(const AddressType & addr, const event_id & eid)
   {
      EventLogger & eil = *EventLogger::getInstance();
      // 0. check if scheduler is active i.e. pthread_create was called
      if (TestScheduler::isAllocated())  // only activates, when multiple threads are running
      {
         TestScheduler & ts = *TestScheduler::peekInstance();
         ConstrainLogger & cl = ts.getConstrainLogger();
         EpochLogger & ep = *EpochLogger::getInstance();
         cl.notifyEventPost<ET>(eid);
         ep.curUpdateRelease(); // sets num releases in epoch
      }
      eil.notifyEventPost<ET>(addr, eid);
   }
};

} /* namespace Actul */

#endif /* INCLUDE_LOGGER_ACTULLOGGER_H_ */
