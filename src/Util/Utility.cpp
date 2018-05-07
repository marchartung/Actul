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
#include <Util/Utility.h>
#include <Util/Invalid.h>
namespace Actul
{

bool Xor(const bool & a, const bool & b)
{
   return (a || b) && !(a && b);
}

bool ByteEqual(const void* ai, const void* bi, const size_type & sz)
{
   const char * a = (const char *)ai;
   const char * b = (const char *)bi;
   for(size_type i=0;i<sz;++i)
      if(a[i] != b[i])
         return false;
   return true;
}

size_type GaussSum(const size_type & n)
{
   return (Pow(n,2)+n)/2;
}

size_type fak(const size_type & n, const size_type & fakDivide)
{
   size_type res = 1, overflow;
   for(size_type i=fakDivide+1;i<n;++i)
   {
      overflow = res;
      res *= i;
      if(res <= overflow)
      {
         res = invalid<size_type>();
         break;
      }
   }
   return res;
}

size_type BinCoeff(const size_type & n , const size_type & k)
{
   size_type lower = (k < n-k) ? k : n-k, upper = (k > n-k) ? k : n-k;
   return fak(n,upper)/fak(lower);
}

}
