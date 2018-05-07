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
#ifndef INCLUDE_UTIL_UTILITY_H_
#define INCLUDE_UTIL_UTILITY_H_

#include <type_traits>
#include <Util/TypeDefs.h>

namespace Actul
{

template<typename T>
const T & Max(const T & l, const T & r)
{
   return (l < r) ? r : l;
}

template<typename T>
const T & Min(const T & l, const T & r)
{
   return (l > r) ? r : l;
}

template<typename T>
typename std::make_unsigned<T>::type UAbs(const T & val)
{
   return (val < 0) ? -val : val;
}

template<typename T>
T Abs(const T & val)
{
   return (val < 0) ? -val : val;
}

template<typename T>
void Swap(T & l, T & r)
{
   T tmp = l;
   l = r;
   r = tmp;
}

bool ByteEqual(const void* ai, const void* bi, const size_type & sz);


//pow
template<class T, size_type n>
struct helper_pow
{
   static constexpr T pow(T const& a){
        return a*helper_pow<T,n-1>::pow(a);
    }
};

//final specialization pow
template<class T>
struct helper_pow<T,0>
{
    static constexpr T pow(T const& a){
        return 1;
    }
};

template<class T,size_type n>
constexpr T CEPow(const T & a)
{
   return helper_pow<T,n>::pow(a);
}

template<typename T>
T Pow(const T & val, const size_type & pow)
{
   T res = 1;
   for (size_type i = 0; i < pow; ++i)
      res *= val;
   return res;
}

size_type GaussSum(const size_type & n);

size_type BinCoeff(const size_type & n , const size_type & k);

size_type fak(const size_type & n, const size_type & fakDivide = 2);

template<typename T>
void RawCopy(const T * source, T * target, const size_type & num)
{
   for (size_type i = 0; i < num; ++i)
      target[i] = source[i];
}

template<typename T>
void Copy(const T * source, T * target, const size_type & num)
{
   if (num > 31)
   {
      if (sizeof(T) % 8 == 0)
         RawCopy<uint64_t>(reinterpret_cast<const uint64_t*>(source), reinterpret_cast<uint64_t*>(target), num * (sizeof(T) / 8));
      else if (sizeof(T) % 4 == 0)
         RawCopy<uint32_t>(reinterpret_cast<const uint32_t*>(source), reinterpret_cast<uint32_t*>(target), num * (sizeof(T) / 4));
      else if (sizeof(T) % 2 == 0)
         RawCopy<uint16_t>(reinterpret_cast<const uint16_t*>(source), reinterpret_cast<uint16_t*>(target), num * (sizeof(T) / 2));
      else
         RawCopy<char>(reinterpret_cast<const char*>(source), reinterpret_cast<char*>(target), num * sizeof(T));
   } else
      RawCopy<char>(reinterpret_cast<const char*>(source), reinterpret_cast<char*>(target), num * sizeof(T));

   /*for(size_type i=0;i<num;++i)
    target[i] = source[i];*/
}

bool Xor(const bool & a, const bool & b);

}

#endif /* INCLUDE_UTIL_UTILITY_H_ */
