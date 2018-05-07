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
#ifndef INCLUDE_UTIL_SHM_SHMWRAPPER_HPP_
#define INCLUDE_UTIL_SHM_SHMWRAPPER_HPP_

#include <Util/Container/SHM/SHMObject.h>
#include <Util/DebugRoutines.h>
namespace Actul
{

template<typename T>
class SHMWrapper : public SHMObject
{
 public:
   SHMWrapper() = delete;
   SHMWrapper(const SHMWrapper & in) = delete;

   SHMWrapper(SHMObserver * in)
         : SHMObject(in)
   {
      initialize();
   }
   ~SHMWrapper()
   {

   }

   T & getElem()
   {
      return *reinterpret_cast<T*>(_mem.data());
   }

   const T & getElem() const
   {
      return *reinterpret_cast<T*>(_mem.data());
   }

 private:

   void initialize()
   {
      SHMObject::reallocate(sizeof(T));
      T * elem = &getElem();
      elem = new (elem) T();
   }
   void internalReallocate(const size_type & numBytes) override
   {
      actulAssert(_mem.size() == 0, "SHMWrapper can't be reallocated");
      reallocate(numBytes);
   }

};

}
#endif /* INCLUDE_UTIL_SHM_SHMWRAPPER_HPP_ */
