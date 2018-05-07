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
#ifndef INCLUDE_UTIL_SHM_SHMOBJECT_HPP_
#define INCLUDE_UTIL_SHM_SHMOBJECT_HPP_

#include <Util/Defines.h>
#include <Util/Container/SHM/SHMemory.h>

namespace Actul
{
class SHMManager;
template<typename T>
class SHMArray;

class SHMObserver
{
 public:
   SHMObserver();

   virtual ~SHMObserver();

   //virtual void addObservable(SHMObject * in) = 0;

   virtual void notify(const size_type & objId, const int & shmId, const size_type & bytesAllocated) = 0;
};

class SHMObject
{
 public:
   SHMObject(const SHMObject & in) = delete;
   SHMObject(SHMObserver * in);

   virtual ~SHMObject();

   void setObserver(SHMObserver * observer);

   void destroy();

   void attach();

   void detach();

   int getShmId() const;

   void setShmId(const int & id);

   int getObjectId() const;

   void setMemSize(const size_type & num);

   size_type getMemSize() const;

   template<typename U>
   friend void Actul::deleteInstance(U*);
   template<typename U, typename ...Args>
   friend U * Actul::newInstance(Args ... args);

 protected:
   SHMObserver * _observer;
   SHMemory _mem;

   void reallocate(const size_type & numBytes = 0u);

   virtual void internalReallocate(const size_type & numBytes) = 0;

 private:

   friend class SHMManager;
   template<typename T>
   friend class SHMArray;

   SHMObject();
};

}

#endif /* INCLUDE_UTIL_SHM_SHMOBJECT_HPP_ */
