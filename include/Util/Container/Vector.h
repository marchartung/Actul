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
#ifndef INCLUDE_UTIL_VECTOR_H_
#define INCLUDE_UTIL_VECTOR_H_

#include <Util/EnvironmentDependencies.h>
#include <Util/TypeDefs.h>
#include <Util/Utility.h>
#include <Util/Invalid.h>
#include <Util/DebugRoutines.h>

namespace Actul
{

template<typename T>
class Vector
{
 public:

   typedef T value_type;

   Vector(const size_type & num, const T & val = T())
         : _sz(0),
           _cp(0),
           _mem(nullptr)
   {
      resize(num, val);
   }

   Vector()
         : Vector(1, T())
   {
      _sz = 0;
   }

   Vector(const Vector<T> & in)
         : Vector<T>(in.size(), T())
   {
      _sz = 0;
      *this = in;
   }

   Vector(Vector<T> && in)
         : _sz(0),
           _cp(0),
           _mem(nullptr)
   {
      *this = std::move(in);
   }

   ~Vector()
   {
      if (_cp != 0)
      {
         actulAssert(_mem != nullptr, "Vector: invalid free");
         InternalFree(_mem);
         _mem = nullptr;
         _sz = 0;
         _cp = 0;
      }
   }

   bool empty() const
   {
      return _sz == 0;
   }

   Vector & operator=(const Vector<T> & in)
   {
      resize(in._sz);
      for (size_type i = 0; i < size(); ++i)
      {
         new (&_mem[i]) T(in._mem[i]);
      }
      return *this;
   }
   Vector & operator=(Vector<T> && in)
   {
      Swap(_sz, in._sz);
      Swap(_cp, in._cp);
      Swap(_mem, in._mem);
      return *this;
   }

   void reserve(const size_type & num)
   {
      if (num > _cp)
      {
         T * tmp = (T*) InternalMalloc(sizeof(T) * num);
         actulAssert(tmp != nullptr, "Vector: allocation failed");
         Copy(_mem, tmp, _sz);
         if (_cp != 0)
         {
            actulAssert(_mem != nullptr, "Vector: invalid free");
            InternalFree(_mem);
         }
         _mem = tmp;
         _cp = num;
      }

   }

   void push_back(const T & elem)
   {
      if (_sz + 1 > _cp)
      {
         T tmp(elem);
         reserve(2 * Max(1u, _sz));
         new (&_mem[_sz++]) T(tmp);
      } else
         new (&_mem[_sz++]) T(elem);
   }

   void push_back(T && elem)
   {
      if (_sz + 1 > _cp)
         reserve(2 * Max(1u, _sz));
      new (&_mem[_sz++]) T(std::move(elem));
   }

   void swap(Vector<T> & in)
   {
      Swap(_sz,in._sz);
      Swap(_cp,in._cp);
      Swap(_mem,in._mem);
   }

   void sort()
   {
      size_type minIdx;
      for(size_type i=0;i<size();++i)
      {
         minIdx = i;
         for(size_type j=i+1;j<size();++j)
            if(_mem[j] < _mem[minIdx])
               minIdx = j;
         Swap(_mem[i],_mem[minIdx]);
      }
   }

   void pop_back()
   {
      back().~T();
      --_sz;
   }

   void unsafe_push_back(const T & elem)
   {
      new (&_mem[_sz++]) T(elem);
   }

   void unsafe_push_back(T && elem)
   {
      new (&_mem[_sz++]) T(std::move(elem));
   }

   void resize(const size_type & num, const T & elem = T())
   {
      size_type i = _sz;
      unsafe_resize(num);
      for (; i < _sz; ++i)
         new (&_mem[i]) T(elem);
   }

   void unsafe_resize(const size_type & num)
   {
      reserve(num);
      _sz = num;
   }

   const size_type & size() const
   {
      return _sz;
   }

   const size_type & capacity() const
   {
      return _cp;
   }

   void clear()
   {
      for (size_type i = 0; i < _sz; ++i)
         _mem[i].~T();
      _sz = 0;
   }

   T & constructAt(const size_type & idx)
   {
      if(idx>=size())
         resize(idx+1);
      return _mem[idx];
   }

   T & operator[](const size_type & idx)
   {
      actulAssert(idx < _sz, "Vector: Invalid vector access");
      return _mem[idx];
   }

   const T & operator[](const size_type & idx) const
   {
      actulAssert(idx < _sz, "Vector: Invalid vector access");
      return _mem[idx];
   }

   T & back()
   {
      actulAssert(_sz > 0, "Vector: Invalid back access");
      return _mem[_sz - 1];
   }

   const T & back() const
   {
      actulAssert(_sz > 0, "Vector: Invalid back access");
      return _mem[_sz - 1];
   }

   void remove(const size_type & idx)
   {
      for(size_type i=idx;i<size()-1;++i)
         _mem[i] = _mem[i+1];
      --_sz;
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

   void unordered_remove(const size_type & idx)
   {
      Swap(_mem[idx], back());
      pop_back();
   }

 private:
   size_type _sz;
   size_type _cp;
   T * _mem;

};
}

#endif /* INCLUDE_UTIL_VECTOR_H_ */
