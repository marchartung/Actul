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
#ifndef INCLUDE_UTIL_SHM_SHMMANAGER_HPP_
#define INCLUDE_UTIL_SHM_SHMMANAGER_HPP_

#include <Util/Defines.h>
#include <Util/Container/Vector.h>
#include <Util/Container/SHM/SHMArray.h>
#include <Util/Container/SHM/SHMWrapper.h>
#include <Util/SingletonClass.h>


namespace Actul
{
class SHMManager : public SHMObserver, public SingletonClass<SHMManager>
{
   friend class SingletonClass<SHMManager> ;
 public:

   ~SHMManager();

   //void addObservable(SHMObject * in) override;

   void notify(const size_type & objId, const int & shmId, const size_type & bytesAllocated) override;

   void initialize();

   void detachObservables();

   void attachObservables();

   template<typename T>
   SHMArray<T> * getSHMArray();
   template<typename T>
   SHMWrapper<T> * getSHMWrapper();


   void destroyAll();

   void detachAll();

   size_type bytesAllocated() const;


 private:

   struct SHMObjectRef
   {
      bool staticSize;
      SHMObject * object;
      int shmId;
      size_type bytesAllocated;

      SHMObjectRef(SHMObject * p, const int & shm, const size_type & num, const bool & staticSize)
            : staticSize(staticSize),
              object(p),
              shmId(shm),
              bytesAllocated(num)
      {
      }

   };

   SHMArray<SHMObjectRef> * _allocIds;
   Vector<SHMObject *> _existingSHMs;

   SHMManager();

};

template<typename T>
inline SHMArray<T>* SHMManager::getSHMArray()
{
   SHMArray<T> * res = Actul::newInstance<SHMArray<T>>(this);
   _existingSHMs.push_back(res);
   SHMObjectRef tmp(res, res->getShmId(), res->getMemSize(), false);
   _allocIds->insert(tmp);
   return res;
}

template<typename T>
inline SHMWrapper<T>* SHMManager::getSHMWrapper()
{
   SHMWrapper<T> * res = Actul::newInstance<SHMWrapper<T>>(this);
   _existingSHMs.push_back(res);
   SHMObjectRef tmp(res, res->getShmId(), res->getMemSize(), false);
   _allocIds->insert(tmp);
   return res;
}
}
#endif /* INCLUDE_UTIL_SHM_SHMMANAGER_HPP_ */
