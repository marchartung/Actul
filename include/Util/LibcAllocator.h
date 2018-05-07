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
#ifndef INCLUDE_LIBCALLOCATOR_H_
#define INCLUDE_LIBCALLOCATOR_H_

#include <cstddef>
#include <malloc.h>
#include <Util/EnvironmentDependencies.h>
#include <memory>

namespace Actul
{
// ALLOCATORS:
template<class T>
struct LibcAllocator
{

   typedef T value_type;
   typedef T* pointer;
   typedef const T* const_pointer;
   typedef T& reference;
   typedef const T& const_reference;
   typedef std::size_t size_type;
   typedef long int difference_type;

   template<class U> struct rebind
   {
      typedef LibcAllocator<U> other;
   };

   T* allocate(std::size_t n)
   {
      T * res = reinterpret_cast<T*>(InternalMalloc(n*sizeof(T)));
      return res;
   }

   template <class U, class... Args>
   void construct (U* p, Args&&... args)
   {
      p = ::new((void *)p) U(std::forward<Args>(args)...);
   }

   void construct (pointer p, const_reference val)
   {
      p = new (p) T(val);
   }

   LibcAllocator()
   {}

   template <class U>
   LibcAllocator(const LibcAllocator<U>& other)
   {
   }

   void deallocate(T* p, std::size_t n)
   {
      InternalFree(p);
   }

   template<class U>
   void destroy(U* p)
   {
      p->~U();
   }
};

template<class T, class U>
bool operator==(const LibcAllocator<T>&, const LibcAllocator<U>&)
{
   return false;
}

template<class T, class U>
bool operator!=(const LibcAllocator<T>&, const LibcAllocator<U>&)
{
   return false;
}

}
#endif /* INCLUDE_LIBCALLOCATOR_H_ */
