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
#ifndef INCLUDE_UTIL_ADDRESSMAP_H_
#define INCLUDE_UTIL_ADDRESSMAP_H_

#include <Util/Defines.h>
#include <Util/Container/Vector.h>
#include <Util/Utility.h>
#include <Logger/Event.h>
#include <Logger/EpochLogger.h>
#include <Implementation/FunctionStackImpl.h>
#include <Scheduler/ThreadInfo.h>
#include <Scheduler/ProcessInfo.h>

#include <unordered_map>

namespace Actul
{

class AddressCluster
{
 public:

   static AddressType getBaseAddress(const AddressType & addr);

   static uint8_t getAddrOffset(const AddressType & addr);

   static uint8_t getAddrWidth(const unsigned long & width, const AddressType & addr);

   static bool isAlignedAccess(const unsigned long & width, const AddressType & addr);

   static AddressType calcRealAddress(const AddressType & baseAddr, const uint8_t & offset);

 private:
   static constexpr unsigned _bitShift = 3;
   static constexpr unsigned _maxWidth = CEPow<unsigned,_bitShift>(2);
   static constexpr uintptr_t _baseAddrMask = (~0U << _bitShift);
   static constexpr uintptr_t _offsetMask = ~(~0U << _bitShift);

   static_assert(_maxWidth <=256, "AddressCluster: shift config is invalid");
};

struct EventInstance
{
   const size_type epochId;
   const size_type funcStackPos;
   const size_type numThreadAccesses;
   const size_type numTotalReleases;

   EventInstance()
         : epochId(invalid<size_type>()),
           funcStackPos(invalid<size_type>()),
           numThreadAccesses(invalid<size_type>()),
           numTotalReleases(invalid<size_type>())
   {
   }

   EventInstance(const size_type epochId, const size_type funcStackPos)
         : epochId(epochId),
           funcStackPos(funcStackPos),
           numThreadAccesses(ThreadInfo::getNumAccesses()),
           numTotalReleases(ProcessInfo::getNumReleases())
   {
   }

   bool operator==(const EventInstance & e) const
   {
      return epochId == e.epochId && funcStackPos == e.funcStackPos && numTotalReleases == e.numTotalReleases;
   }

   bool operator!=(const EventInstance & e) const
   {
      return !(*this == e);
   }

};

typedef Vector<EventInstance> EventInstanceVec;
typedef Vector<EventInstanceVec> EventInstanceVecVec;

class SimpleAddressMap
{
   typedef std::unordered_map<AddressType, event_id, std::hash<AddressType>, std::equal_to<AddressType>,
         LibcAllocator<std::pair<AddressType, event_id>>> InternalAddrMap;
public:

   SimpleAddressMap(const event_id & start = 0);

   AddressType getCurAddress(const tid_type & tid) const;

   const event_id & get(const AddressType & addr);

   //const uint32_t & getEventCounter(const event_id & id) const;

private:
   int _n;
   event_id _numEvents;
   //Uint32Vec _counter;
   InternalAddrMap _map;
   Array<AddressType,ACTUL_MAX_NUM_THREADS> _cachedAddress;

};

typedef uint32_t event_id;
struct AddressMemoryEventKey
{
   const bool write;
   const bool atomic;
   const uint8_t addrOffset;
   const uint8_t width;
   const event_id eventId;

   AddressMemoryEventKey()
         : write(false),
           atomic(false),
           addrOffset(0),
           width(0),
           eventId(invalid<event_id>())
   {
   }

   AddressMemoryEventKey(const bool & write, const bool & atomic, const uint8_t & addrOffset, const uint8_t & width, const event_id & eventId)
         : write(write),
           atomic(atomic),
           addrOffset(addrOffset),
           width(width),
           eventId(eventId)
   {
   }

   bool operator==(const AddressMemoryEventKey & in) const
   {
      return write == in.write && atomic == in.atomic && addrOffset == in.addrOffset && width == in.width;
   }

   bool operator!=(const AddressMemoryEventKey & in) const
   {
      return !(*this == in);
   }

   bool isWrite() const
   {
      return write;
   }

   bool overlaps(const AddressMemoryEventKey & in) const
   {
      return (addrOffset < in.addrOffset ? (addrOffset + width > in.addrOffset) : in.addrOffset + in.width > addrOffset);
   }
};

typedef Vector<AddressMemoryEventKey> AddressMemoryEventKeyVec;

typedef std::unordered_map<AddressType, AddressMemoryEventKeyVec, std::hash<AddressType>, std::equal_to<AddressType>,
      LibcAllocator<std::pair<AddressType, AddressMemoryEventKeyVec>>> UnorderedEventMap;
typedef UnorderedEventMap::iterator UnorderedEventMapIt;

class MemoryAddressMap
{
 public:
   struct CachedLookUp
   {
      size_type lookUpPos;
      size_type instancePos;
      AddressType addr;
      UnorderedEventMapIt it;
   };

   MemoryAddressMap(const event_id & eStart, const EpochLogger & epLogger, const FunctionStackImpl & fsLogger);

   template<EventType ET>
   const event_id & get(const unsigned long & width, const AddressType & addr)
   {
      actulAssert(width <= 8, "MemoryAddressMap: Only supports memory access events with a width of equal or less of 8 byte");  // No reason, just to safe a loop
      AddressMemoryEventKey key(isWrite<ET>(), isAtomic<ET>(), AddressCluster::getAddrOffset(addr), AddressCluster::getAddrWidth(width, addr), _numEvents);
      size_type pos = 0;
      bool newEvent = false;
      // find create key of base address
      AddressType bAddr = AddressCluster::getBaseAddress(addr);
      auto it = _map.find(bAddr);
      if (it == _map.end())
      {
         auto insIt = _map.insert(std::make_pair(bAddr, AddressMemoryEventKeyVec(1, key)));
         actulAssert(insIt.second, "MemoryAddressMap: Couldn't create new memory address key");
         //actulAssert(_counter.size() == _numEvents,"MemoryAddressMap: Event counter is inconsistent");
         it = insIt.first;
         pos = 0;
         newEvent = true;
      } else
      {
         AddressMemoryEventKeyVec & keys = it->second;
         for (; pos < keys.size() && keys[pos] != key; ++pos)
            ;
         if (pos == keys.size())
         {
            keys.push_back(key);
            newEvent = true;
         }
      }

      if (newEvent)
      {
         _eventInstances.push_back(Array<Vector<EventInstance>, ACTUL_MAX_NUM_THREADS>());
         ++_numEvents;
      }

      const tid_type & tid = ThreadInfo::getThreadId();
      _cachedAccesses[tid].addr = addr;
      _cachedAccesses[tid].lookUpPos = pos;
      _cachedAccesses[tid].instancePos = invalid<size_type>();
      _cachedAccesses[tid].it = it;
      return it->second[pos].eventId;
   }

   template<EventType ET>
   void addInstance()
   {
      const tid_type & tid = ThreadInfo::getThreadId();
      const event_id & eid = _cachedAccesses[tid].it->second[_cachedAccesses[tid].lookUpPos].eventId;
      _cachedAccesses[tid].instancePos = _eventInstances[eid][tid].size();
      EventInstance ei(_epLogger.getCurEpochId(), _fsLogger.getCurStackPos());
      if (_eventInstances[eid][tid].empty() || _eventInstances[eid][tid].back() != ei)
         _eventInstances[eid][tid].push_back(ei);  // TODO push all?
   }

   event_id add(const EventType & et, const unsigned long & width, const AddressType & addr)
   {
      actulAssert(width <= 8, "MemoryAddressMap: Only supports memory access events with a width of equal or less of 8 byte");  // No reason, just to safe a loop
      bool newEvent = false;
      AddressMemoryEventKey key(isWrite(et), isAtomic(et), AddressCluster::getAddrOffset(addr), AddressCluster::getAddrWidth(width, addr), _numEvents);
      size_type pos = 0;
      auto it = _map.find(AddressCluster::getBaseAddress(addr));
      if (it == _map.end())
      {
         auto insIt = _map.insert(std::make_pair(AddressCluster::getBaseAddress(addr), AddressMemoryEventKeyVec(1, key)));
         actulAssert(insIt.second, "MemoryAddressMap: Couldn't create new memory address key");
         newEvent = true;
         actulAssert(_eventInstances.size() == _numEvents, "MemoryAddressMap: Event instances are inconsistent");
         it = insIt.first;
         pos = 0;
      } else
      {
         AddressMemoryEventKeyVec & keys = it->second;
         for (; pos < keys.size() && keys[pos] != key; ++pos)
            ;
         if (pos == keys.size())
         {
            keys.push_back(key);
            newEvent = true;
         }
      }
      if (newEvent)
      {
         _eventInstances.push_back(Array<Vector<EventInstance>, ACTUL_MAX_NUM_THREADS>());
         ++_numEvents;
         actulAssert(_eventInstances.size() == _numEvents, "MemoryAddressMap: Event instances are inconsistent");
      }
      return it->second[pos].eventId;
   }

   const AddressMemoryEventKeyVec & getCurCachedMemoryEvents(const AddressType & addr) const;

   const AddressMemoryEventKey & getCurAccess(const AddressType & addr) const;

   AddressType getCurAddress(const tid_type & tid) const;

   const EventInstance & getCurCachedInstance(const AddressType & addr) const;

   const EventInstanceVec & getInstances(const event_id & id, const tid_type & tid) const
   {
      actulAssert(id < _eventInstances.size(), "MemoryAddressMap: Try to access invalid event instances");
      return _eventInstances[id][tid];
   }

   size_type getNumCurInstances(const tid_type & tid) const;

   size_type getNumInstances(const event_id & id, const tid_type & tid, const size_type & instanceNum = invalid<size_type>()) const;

   //const uint32_t & getEventCounter(const event_id & id) const;

 private:

   event_id _numEvents;
   const EpochLogger & _epLogger;
   const FunctionStackImpl & _fsLogger;
   Array<CachedLookUp, ACTUL_MAX_NUM_THREADS> _cachedAccesses;
   UnorderedEventMap _map;
   //EventInstanceVecVec _eventInstances;
   Vector<Array<Vector<EventInstance>, ACTUL_MAX_NUM_THREADS>> _eventInstances;
   //Uint32Vec _counter;
};

} /* namespace Actul */

#endif /* INCLUDE_UTIL_ADDRESSMAP_H_ */
