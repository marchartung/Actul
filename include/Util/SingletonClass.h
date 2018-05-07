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
#ifndef INCLUDE_UTIL_SINGLETONCLASS_H_
#define INCLUDE_UTIL_SINGLETONCLASS_H_

#include <Util/Defines.h>

namespace Actul
{

template<class BaseClass>
class SingletonClass
{

 private:
   static BaseClass * _instance;

 public:

   static bool isAllocated()
   {
      return _instance != nullptr;
   }

   static BaseClass * peekInstance()
   {
      return _instance;
   }

   template <typename ... Args>
   static void allocateInstance(Args ... args)
   {
      actulAssert(!isAllocated(),"SingletonClass: Instance was already allocated");
      LibcAllocator<BaseClass> allocator;
      _instance = allocator.allocate(1);
      _instance = new (_instance) BaseClass(args...);

   }

   static void eraseInstance()
   {
      if (isAllocated())
      {
         LibcAllocator<BaseClass> allocator;
         _instance->~BaseClass();
         allocator.deallocate(_instance,sizeof(BaseClass));
         _instance = nullptr;
      }
   }

   template <typename ... Args>
   static BaseClass * getInstance(Args ... args)
   {
      if (!isAllocated())
      {
         allocateInstance<Args ...>(args...);
      }
      return _instance;
   }


};

} /* namespace Actul */

#endif /* INCLUDE_UTIL_SINGLETONCLASS_H_ */
