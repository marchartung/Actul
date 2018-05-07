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
#ifndef INCLUDE_UTIL_ARRAY_H_
#define INCLUDE_UTIL_ARRAY_H_

#include <Util/TypeDefs.h>
#include <Util/Invalid.h>
#include <Util/String.h>

namespace Actul
{

template<typename T, size_type n>
class Array
{

   Array & operator=(Array<T, n> && in) = delete;
 public:

   typedef T value_type;

   Array()
   {
      for (size_type i = 0; i < size(); ++i)
         new (&_mem[i]) T();
   }

   Array(const T & val)
   {
      for (size_type i = 0; i < size(); ++i)
         new (&_mem[i]) T(val);
   }

   Array(const Array<T, n> & in)
   {
      for (size_type i = 0; i < size(); ++i)
      {
         _mem[i].~T();
         new (&_mem[i]) T(in[i]);
      }
   }

   Array(Array<T, n> && in)
   {
      // sensless to use :-/
      for (size_type i = 0; i < size(); ++i)
      {
         _mem[i].~T();
         new (&_mem[i]) T(in[i]);
      }
   }

   ~Array()
   {
      for (size_type i = 0; i < size(); ++i)
         _mem[i].~T();
   }

   bool operator==(const Array<T, n> & in) const
   {
      bool res = true;
      for (size_type i = 0; i < n; ++i)
         if (_mem[i] != in[i])
         {
            res = false;
            break;
         }
      return res;
   }

   bool operator!=(const Array<T, n> & in) const
   {
      return !(*this == in);
   }

   Array & operator=(const Array<T, n> & in)
   {
      for (size_type i = 0; i < size(); ++i)
         _mem[i] = in[i];
      return *this;
   }

   constexpr size_type size() const
   {
      return n;
   }

   String && to_string() const
   {
      String res("[");
      for(size_type i=0;i<size();++i)
         res += "\"" + Actul::to_string(_mem[i]) + "\" ";
      return std::move(res);
   }

   T & operator[](const size_type & idx)
   {
      return _mem[idx];
   }

   const T & operator[](const size_type & idx) const
   {
      return _mem[idx];
   }

   size_type find(const T & in) const
   {
      size_type res = invalid<size_type>();
      for(size_type i=0;i<size();++i)
         if(_mem[i] == in)
         {
            res = i;
            break;
         }
      return res;
   }

   bool contains(const T & in) const
   {
      return find(in) != invalid<size_type>();
   }

 private:
   T _mem[n];

};


}

#endif /* INCLUDE_UTIL_VECTOR_H_ */
