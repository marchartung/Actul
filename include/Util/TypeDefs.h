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
#ifndef INCLUDE_TYPEDEFS_HPP_
#define INCLUDE_TYPEDEFS_HPP_

#include <cstdint>

#include <sys/types.h>

namespace Actul
{

typedef uint16_t tid_type;
typedef int16_t sid_type;
typedef uint32_t size_type;
typedef uint32_t event_id;
typedef int64_t sync_status_type;

typedef const void * AddressType;


typedef void*(*ExecuteFunction)(void*);



}



#endif /* INCLUDE_TYPEDEFS_HPP_ */
