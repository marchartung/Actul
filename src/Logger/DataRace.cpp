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
#include <Logger/DataRace.h>
#include <cassert>

namespace Actul
{

bool areEqualStacks(const StackArray & s1, const StackArray & s2)
{
   bool res = true;
   for (size_type i = 0; i < MAX_FUNCTION_STACK_SIZE; ++i)
      if (s1[i] != s2[i])
      {
         res = false;
         break;
      }
   return res;
}

bool DataRace::isSameInstance(const DataRace & in) const
{
   return (in._a[0].isSameInstance(_a[0]) && in._a[1].isSameInstance(_a[1]))
       || (in._a[0].isSameInstance(_a[1]) && in._a[1].isSameInstance(_a[0]));
}

bool DataRace::isSameInstanceReversed(const DataRace & in) const
{
   return in._a[0].isSameInstance(_a[1]) && in._a[1].isSameInstance(_a[0]);
}


bool DataRace::isSameCodeAccess(const DataRace & in) const
{
   return (in._a[0].isSameCodeAccess(_a[0]) && in._a[1].isSameCodeAccess(_a[1]))
       || (in._a[0].isSameCodeAccess(_a[1]) && in._a[1].isSameCodeAccess(_a[0]));
}

template<>
const DataRace & invalid<DataRace>()
{
   static const DataRace res;
   return res;
}


template<>
String to_string<DataRace>(const DataRace & d)
{
   const MemoryAccess & a1 = d.getAccess(0),& a2 = d.getAccess(1);
   String accessT = to_string((a1.isWrite) ? "w" : "r") + ((a1.isAtomic) ? "a" : "") + ((a2.isWrite) ? "w" : "r") + ((a2.isAtomic) ? "a" : "");
   String instance = "_t"+to_string(a1.tid) + ":" + to_string(a1.instanceNum) + ",t"+to_string(a2.tid) + ":" + to_string(a2.instanceNum);
   return String("DR:[") + accessT + instance +"]";
}

MemoryAccess::MemoryAccess()
      : isWrite(false),
        isAtomic(false),
        width(invalid<uint8_t>()),
        tid(invalid<tid_type>()),
        instanceNum(invalid<uint32_t>()),
        numTotalReleases(invalid<uint32_t>()),
        numThreadAccesses(0),
        addr(nullptr),
        stack(invalid<StackArray>())
{
}
bool MemoryAccess::isSameInstance(const MemoryAccess & ma) const
{
   return (isWrite == ma.isWrite && isAtomic == ma.isAtomic && width == ma.width && tid == ma.tid && instanceNum == ma.instanceNum
         && addr == ma.addr && ma.stack == stack);
}

bool MemoryAccess::isSameCodeAccess(const MemoryAccess & ma) const
{
   bool writeEq = (isWrite && ma.isWrite) || (!isWrite && !ma.isWrite);
   bool atomicEq = (isAtomic && ma.isAtomic) || (!isAtomic && !ma.isAtomic);
   return (writeEq && atomicEq && width == ma.width && ma.stack == stack);
}

bool MemoryAccess::operator==(const MemoryAccess & ma) const
{
   return (isWrite == ma.isWrite && isAtomic == ma.isAtomic && width == ma.width && tid == ma.tid && instanceNum == ma.instanceNum
         && numTotalReleases == ma.numTotalReleases && addr == ma.addr && ma.stack == stack);
}


bool MemoryAccess::happendBefore(const MemoryAccess & ma) const
{
   return (ma.tid == tid && (ma.numTotalReleases > numTotalReleases || (ma.numTotalReleases == numTotalReleases && ma.numThreadAccesses > numThreadAccesses)));
}

bool MemoryAccess::operator!=(const MemoryAccess & ma) const
{
   return !(*this == ma);
}

DataRace::DataRace()
      : _order(false),
        _a(MemoryAccess())
{
}

DataRace::DataRace(const MemoryAccess & a1, const MemoryAccess & a2)
      : _order(false)
{
   _a[0] = a1;
   _a[1] = a2;
}

DataRace::DataRace(const DataRace & d)
      : _order(d._order),
        _a(d._a)
{
}

DataRace & DataRace::operator=(const DataRace & d)
{
   _order = d._order;
   _a = d._a;
   return *this;
}

bool DataRace::operator==(const DataRace & in) const
{
   return _a == in._a;
}
bool DataRace::operator!=(const DataRace & in) const
{
   return !(*this == in);
}

const bool & DataRace::order() const
{
   return _order;
}
void DataRace::setOrder(const bool & order)
{
   _order = order;
}

const bool & DataRace::isWrite(const size_type& num) const
{
   actulAssert(num < 2, "DataRace: Invald num access");
   return _a[num].isWrite;
}

const bool & DataRace::isAtomic(const size_type& num) const
{
   actulAssert(num < 2, "DataRace: Invald num access");
   return _a[num].isAtomic;
}

const uint8_t & DataRace::width(const size_type& num) const
{
   actulAssert(num < 2, "DataRace: Invald num access");
   return _a[num].width;
}

const tid_type & DataRace::tid(const size_type& num) const
{
   actulAssert(num < 2, "DataRace: Invald num access");
   return _a[num].tid;
}

const uint32_t & DataRace::instanceNum(const size_type& num) const
{
   actulAssert(num < 2, "DataRace: Invald num access");
   return _a[num].instanceNum;
}

const uint32_t & DataRace::numTotalReleases(const size_type& num) const
{
   actulAssert(num < 2, "DataRace: Invald num access");
   return _a[num].numTotalReleases;
}

AddressType DataRace::addr(const size_type& num) const
{
   actulAssert(num < 2, "DataRace: Invald num access");
   return _a[num].addr;
}

const StackArray & DataRace::stack(const size_type& num) const
{
   actulAssert(num < 2, "DataRace: Invald num access");
   return _a[num].stack;
}


const MemoryAccess & DataRace::getAccess(const size_type & num) const
{
   actulAssert(num < 2, "DataRace: Invald num access");
   return _a[num];
}

}
