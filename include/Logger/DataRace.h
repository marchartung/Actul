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
#ifndef INCLUDE_ANALYSE_DATARACE_H_
#define INCLUDE_ANALYSE_DATARACE_H_

#include <cstdint>
#include <Util/TypeDefs.h>
#include <Util/Defines.h>
#include <Util/Container/Array.h>
#include <Util/Container/Vector.h>

namespace Actul
{

struct MemoryAccess
{
   bool isWrite;
   bool isAtomic;
   uint8_t width;
   tid_type tid;
   uint32_t instanceNum;
   uint32_t numTotalReleases;
   uint32_t numThreadAccesses;
   AddressType addr;
   StackArray stack;

   MemoryAccess();

   bool isSameInstance(const MemoryAccess & ma) const;

   bool isSameCodeAccess(const MemoryAccess & ma) const;

   bool operator==(const MemoryAccess & ma) const;

   bool operator!=(const MemoryAccess & ma) const;

   bool happendBefore(const MemoryAccess & ma) const;

};
typedef Array<MemoryAccess,2> MemoryAccess2Array;

class DataRace
{
   DataRace(DataRace && d) = delete;
 public:

   DataRace();

   DataRace(const DataRace & d);

   DataRace(const MemoryAccess & a1, const MemoryAccess & a2);

   DataRace & operator=(const DataRace & d);

   const MemoryAccess & getAccess(const size_type & num) const;

   const bool & order() const;
   void setOrder(const bool & num);

   const bool & isWrite(const size_type & num) const;
   const bool & isAtomic(const size_type & num) const;
   const uint8_t & width(const size_type & num) const;
   const tid_type & tid(const size_type & num) const;
   const uint32_t & instanceNum(const size_type & num) const;
   const uint32_t & numTotalReleases(const size_type & num) const;
   AddressType addr(const size_type & num) const;
   const StackArray & stack(const size_type & num) const;

   bool isSameCodeAccess(const DataRace & in) const;

   bool isSameInstance(const DataRace & in) const;

   bool isSameInstanceReversed(const DataRace & in) const;

   bool operator==(const DataRace & in) const;
   bool operator!=(const DataRace & in) const;

 private:

   bool _order;
   MemoryAccess2Array _a;
};

template<>
String to_string<DataRace>(const DataRace & d);


template<>
const DataRace & invalid<DataRace>();

typedef Vector<DataRace> DataRaceVec;
typedef Vector<DataRace*> DataRacePtrVec;


struct AddRaceStruct
{
   bool toAdd;
   bool wasFirstPermuted;
   size_type id;
   size_type order;

   AddRaceStruct()
         : toAdd(false),
           wasFirstPermuted(false),
           id(invalid<size_type>()),
           order(invalid<size_type>())
   {
   }
};
typedef Vector<AddRaceStruct> AddRaceStructVec;

}
#endif /* INCLUDE_ANALYSE_DATARACE_H_ */
