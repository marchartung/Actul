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
#ifndef INCLUDE_UTIL_ACTULMEMORYALLOCATOR_H_
#define INCLUDE_UTIL_ACTULMEMORYALLOCATOR_H_

#include <Util/SingletonClass.h>
#include <Util/Container/Vector.h>

namespace Actul
{

class MemoryAllocator : public SingletonClass<MemoryAllocator>
{
   friend class SingletonClass<MemoryAllocator>;

 public:

   void * alignedAlloc(const unsigned & num, const unsigned & align);

   void free(void * ptr);

 private:
   struct HeapLog
   {
      unsigned int num;
      void * ptr;

      HeapLog()
            : num(0),
              ptr(nullptr)
      {
      }

      bool isAllocated() const
      {
         return ptr != nullptr;
      }
   };
   typedef Vector<HeapLog> HeapLogVec;

   HeapLogVec _heapAllocs;


   MemoryAllocator();

   bool isValidAlignment(const void * ptr, const unsigned & align);

   void * allocate_mmap(const unsigned & num);

   void deallocate_mmap(const unsigned & num, void * ptr);

   void * getAlignment(const void * ptr, const unsigned & align);
};

}

#endif /* INCLUDE_UTIL_ACTULMEMORYALLOCATOR_H_ */
