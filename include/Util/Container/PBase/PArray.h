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
#ifndef PARRAY_HPP_
#define PARRAY_HPP_

#include <algorithm>
#include <utility>

#include <Util/DebugRoutines.h>
#include <Util/TypeDefs.h>

namespace Actul
{

template<typename U, typename E>
class PGraph;
template<typename T>
class PNode;

template<typename T>
class PArray
{
 public:
   // initial call to the space in ptr
   PArray(char * ptr, const size_type & size)
   {
      this->size() = 0;
      internalMemCapacity() = size;
   }

   // reconstruct array from pointer
   PArray(char * ptr)
   {
      //assuming the the content is already set
   }

   PArray()
   {

   }

   // move constructor
   PArray(PArray<T> && in)
   {
      //assuming the content is already set
   }

   virtual ~PArray()
   {

   }

   T & back()
   {
      return this->operator [](size() - 1);
   }

   const T & back() const
   {
      return this->operator [](size() - 1);
   }

   T & operator[](const size_type & index)
   {
      return (reinterpret_cast<T*>(getPtr() + offsetToElements()))[index];
   }
   const T & operator[](const size_type & index) const
   {
      return (reinterpret_cast<const T*>(getPtr() + offsetToElements()))[index];
   }

   T & at(const size_type & index)
   {
      return (reinterpret_cast<T*>(getPtr() + offsetToElements()))[index];
   }
   const T & at(const size_type & index) const
   {
      return (reinterpret_cast<const T*>(getPtr() + offsetToElements()))[index];
   }

   virtual bool insert(const T & in)
   {
      size_type cap = capacity(), sizeC = size();
      if (cap > sizeC)
      {
         T * array = reinterpret_cast<T*>(getPtr() + offsetToElements());
         array[size()] = in;
         setSize(size()+1);
         return true;
      } else
         return false;
   }

   virtual void resize(const size_type & n)
   {
      actulAssert(n <= capacity(), "PArray: Resize not implemented for growing");
      setSize(n);
   }

   void shrinkToFit()
   {
      if (internalMemCapacity() > sizeof(T) * size())
      {
         internalMemCapacity() = sizeof(T) * size();
      }
   }

   void clear()
   {
      setSize(0);
   }

   bool empty() const
   {
      return size() == 0;
   }

   size_type capacity() const
   {
      return internalMemCapacity();
   }

   const size_type & size() const
   {
      return *reinterpret_cast<const size_type*>(getPtr());
   }

 protected:

   size_type getBytesForNumElems(const size_type & numElems) const
   {
      return offsetToElements() + sizeof(T) * numElems;
   }

   size_type getNumElemsFromBytes(const size_type & numBytes) const
   {
      return (numBytes-offsetToElements())/sizeof(T);
   }

   void setMaxElementCapacity(const size_type & numElems)
   {
      internalMemCapacity() = numElems;
   }

   void setCurrentSize(const size_type & numElems)
   {
      setSize(numElems);
   }


 private:

   virtual char* getPtr() = 0;

   virtual const char* getPtr() const = 0;

   constexpr size_type offsetToElements() const
   {
      return 2*sizeof(size_t);
   }

   // version
   size_type & internalMemCapacity()
   {
      return *reinterpret_cast<size_type*>(getPtr() + sizeof(size_type));
   }

   const size_type & internalMemCapacity() const
   {
      return *reinterpret_cast<const size_type*>(getPtr() + sizeof(size_type));
   }

   void setSize(const size_type & inSize)
   {
      *reinterpret_cast<size_type*>(getPtr()) = inSize;
   }

};

} /* namespace Actul */

#endif /* PARRAY_HPP_ */
