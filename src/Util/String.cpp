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
#include <Util/String.h>
#include <algorithm>

namespace Actul
{


template<>
String to_string(AddressType const & ptr)
{
   Stringstream res;
   res << const_cast<void*>(ptr);
   return res.str();
}

template<>
String to_string(const uint8_t & ptr)
{
   return to_string<unsigned>((int)ptr);
}

void printStr(const String & in)
{
   InternalPrintF(in.c_str());
}

void printDStr(const String & in)
{
   //InternalPrintF(in.c_str());
}

}
