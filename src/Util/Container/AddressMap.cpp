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
#include <Util/Container/AddressMap.h>

namespace Actul
{

constexpr unsigned AddressCluster::_bitShift;
constexpr unsigned AddressCluster::_maxWidth;
constexpr uintptr_t AddressCluster::_baseAddrMask;
constexpr uintptr_t AddressCluster::_offsetMask;

AddressType AddressCluster::getBaseAddress(const AddressType & addr)
{
   AddressType res = (AddressType) (((uintptr_t) addr) & _baseAddrMask);
   return res;
}

uint8_t AddressCluster::getAddrOffset(const AddressType & addr)
{
   uint8_t res = (uint8_t) (((uintptr_t) addr) & _offsetMask);
   return res;
}

uint8_t AddressCluster::getAddrWidth(const unsigned long & width, const AddressType & addr)
{
   uint8_t off = getAddrOffset(addr);
   actulAssert(isAlignedAccess(width,addr),"AddressCluster: Unaligned access");
   return Min((unsigned) width + off, _maxWidth) - off;
}

bool AddressCluster::isAlignedAccess(const unsigned long & width, const AddressType & addr)
{
   return width + getAddrOffset(addr) <= _maxWidth;
}

AddressType AddressCluster::calcRealAddress(const AddressType & baseAddr, const uint8_t & offset)
{
   uintptr_t res = (uintptr_t) getBaseAddress(baseAddr);
   res += offset;
   return reinterpret_cast<AddressType>(res);
}





SimpleAddressMap::SimpleAddressMap(const event_id & eStart)
      : _n(0),
        _numEvents(eStart)
{

}

AddressType SimpleAddressMap::getCurAddress(const tid_type & tid) const
{
   return _cachedAddress[tid] ;
}

const event_id & SimpleAddressMap::get(const AddressType & addr)
{
   actulAssert(_n == 0, "ohoh");
   ++_n;
   actulAssert(_n == 1, "ohoh");
   _cachedAddress[ThreadInfo::getThreadId()] = addr;
   auto it = _map.find(addr);
   if (it == _map.end())
   {
      auto insIt = _map.insert(std::make_pair(addr, _numEvents));
      actulAssert(insIt.second, "SimpleAddressMap: Couldn't create new address entry");
      ++_numEvents;
      //_counter.push_back(0);
      //actulAssert(_counter.size() == _numEvents,"MemoryAddressMap: Event counter is inconsistent");
      it = insIt.first;
   }
   //++_counter[it->second];

   actulAssert(_n == 1, "ohoh");
   --_n;
   return it->second;
}

MemoryAddressMap::MemoryAddressMap(const event_id & eStart, const EpochLogger & epLogger, const FunctionStackImpl & fsLogger)
      : _numEvents(eStart),
        _epLogger(epLogger),
        _fsLogger(fsLogger)
{
   for (size_type i = 0; i < _cachedAccesses.size(); ++i)
      _cachedAccesses[i].it = _map.end();
}

/*const uint32_t & MemoryAddressMap::getEventCounter(const event_id & id) const
 {
 actulAssert(id<_counter.size(),"MemoryAddressMap: Accessing invalid event_id");
 return _counter[id];
 }*/

const AddressMemoryEventKeyVec & MemoryAddressMap::getCurCachedMemoryEvents(const AddressType & addr) const
{
   actulAssert(_cachedAccesses[ThreadInfo::getThreadId()].addr == addr, "MemoryAddressMap: Wrong cached access");
   return _cachedAccesses[ThreadInfo::getThreadId()].it->second;
}

const AddressMemoryEventKey & MemoryAddressMap::getCurAccess(const AddressType & addr) const
{
   actulAssert(_cachedAccesses[ThreadInfo::getThreadId()].addr == addr, "MemoryAddressMap: Wrong cached access");
   const CachedLookUp & c = _cachedAccesses[ThreadInfo::getThreadId()];
   return c.it->second[c.lookUpPos];
}

const EventInstance & MemoryAddressMap::getCurCachedInstance(const AddressType & addr) const
{
   const tid_type & tid = ThreadInfo::getThreadId();
   actulAssert(_cachedAccesses[tid].addr == addr, "MemoryAddressMap: Wrong cached access");
   const CachedLookUp & c = _cachedAccesses[tid];
   AddressMemoryEventKeyVec & eventKeys = c.it->second;

   actulAssert(eventKeys.size() > c.lookUpPos, "MemoryAddressMap: Invalid cached access");
   const event_id & id = eventKeys[c.lookUpPos].eventId;
   actulAssert(_eventInstances.size() > id, "MemoryAddressMap: Invalid cached access");
   const EventInstanceVec & evec = _eventInstances[id][tid];
   actulAssert(evec.size() > c.instancePos, "MemoryAddressMap: Invalid cached access");
   const EventInstance & res = evec[c.instancePos];
   return res;
}

AddressType MemoryAddressMap::getCurAddress(const tid_type & tid) const
{
   return _cachedAccesses[tid].addr;
}


size_type MemoryAddressMap::getNumCurInstances(const tid_type & tid) const
{
   const CachedLookUp & c = _cachedAccesses[ThreadInfo::getThreadId()];
   return getNumInstances(c.it->second[c.lookUpPos].eventId, tid);
}

size_type MemoryAddressMap::getNumInstances(const event_id & id, const tid_type & tid, const size_type & instanceNum) const
{
   const EventInstanceVec & eiv = _eventInstances[id][tid];
   return eiv.size();
}

} /* namespace Actul */
