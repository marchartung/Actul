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
#ifndef INCLUDE_UTIL_INVALID_H_
#define INCLUDE_UTIL_INVALID_H_

#include <algorithm>

namespace Actul
{
template<typename T>
T invalid(const T & in)
{
   return std::numeric_limits<T>::max();
}

template<typename T>
const T & invalid()
{
   static T res = std::numeric_limits<T>::max();
   return res;
}

template<typename T>
bool isInvalid(const T & in)
{
   return in == std::numeric_limits<T>::max();
}

template<typename T>
bool isValid(const T & in)
{
   return in != std::numeric_limits<T>::max();
}

}



#endif /* INCLUDE_UTIL_INVALID_H_ */
