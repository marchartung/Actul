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
#ifndef INCLUDE_LOGGER_YIELDEVENT_H_
#define INCLUDE_LOGGER_YIELDEVENT_H_

#include <Logger/DataRace.h>
namespace Actul
{

struct YieldEvent
{
   EventType type;
   bool wasUsed;
   uint8_t width;
   tid_type tid;
   event_id id;
   size_type instanceNum;
   AddressType addr;
   StackArray stack;

   bool operator==(const YieldEvent & in) const
   {
      return type == in.type && id == in.id && width == in.width && tid == in.tid && instanceNum == in.instanceNum && addr == in.addr && stack == in.stack;
   }

   bool operator!=(const YieldEvent & in) const
   {
      return !(*this == in);
   }

   YieldEvent()
         : type(EventType::UNDEFINED),
           wasUsed(false),
           width(0),
           tid(invalid<tid_type>()),
           id(invalid<event_id>()),
           instanceNum(invalid<size_type>()),
           addr(nullptr)
   {
   }

   YieldEvent(const MemoryAccess & mem, const bool & simpleYield = false)
         : type(makeAccessType(mem.isWrite, mem.isAtomic)),
           wasUsed(false),
           width(mem.width),
           tid(mem.tid),
           id(0),
           instanceNum((simpleYield) ? invalid<size_type>() : mem.instanceNum),
           addr(mem.addr),
           stack(mem.stack)
   {

   }

   void setUsed()
   {
      wasUsed = true;
   }

   void invalidate()
   {
      type = EventType::UNDEFINED;
   }

   bool isValid() const
   {
      return type != EventType::UNDEFINED;
   }

   bool isSimpleYield() const
   {
      return instanceNum == invalid<size_type>();
   }

};

typedef Vector<YieldEvent> YieldEventVec;
}

#endif /* INCLUDE_LOGGER_YIELDEVENT_H_ */
