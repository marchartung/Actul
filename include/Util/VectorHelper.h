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
#ifndef INCLUDE_UTIL_VECTORHELPER_H_
#define INCLUDE_UTIL_VECTORHELPER_H_

#include <vector>
#include <algorithm>

namespace Actul
{
/*
template<typename T>
bool unique_push_back(std::vector<T,LibcAllocator<T>> & vec, const T &elem)
{
   bool res = false;
   if(std::find(vec.begin(),vec.end(),elem) == vec.end())
   {
      vec.push_back(elem);
      res = true;
   }
   return res;
}


template<typename T>
void swap_index_to_last_and_pop_back(std::vector<T,LibcAllocator<T>> & vec, const size_type & index)
{
   assert(vec.size() > index);
   std::swap(vec[index],vec.back());
   vec.pop_back();

}

template<typename T>
void swap_first_last_and_pop_back(std::vector<T,LibcAllocator<T>> & vec)
{
   swap_index_to_last_and_pop_back(vec,0);
}

// Calculates the set difference R = a/b
template<typename T>
std::vector<T,LibcAllocator<T>> difference(const std::vector<T,LibcAllocator<T>> & a, const std::vector<T,LibcAllocator<T>> & b)
{
   std::vector<T,LibcAllocator<T>> tmp = b, res;
   for(size_type i=0;i<a.size();++i)
      if(unique_push_back(tmp,a[i]))
         res.push_back(a[i]);
   return res;
}
*/
}


#endif /* INCLUDE_UTIL_VECTORHELPER_H_ */
