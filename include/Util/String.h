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
#ifndef INCLUDE_UTIL_STRING_H_
#define INCLUDE_UTIL_STRING_H_

#include <Util/TypeDefs.h>
#include <Util/LibcAllocator.h>
#include <string>
#include <sstream>


namespace Actul
{

typedef std::basic_stringstream<char,std::char_traits<char>,LibcAllocator<char>> Stringstream;
typedef std::basic_string<char,std::char_traits<char>,LibcAllocator<char>> String;



template<typename T>
String to_string(const T & in)
{
   Stringstream res;
   res << in;
   return res.str();
}
template<>
String to_string(const uint8_t & ptr);

template<>
String to_string(AddressType const & ptr);

void printStr(const String & in);

void printDStr(const String & in);

}


#endif /* INCLUDE_UTIL_STRING_H_ */
