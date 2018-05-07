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
#include <Util/Container/SHM/SHMManager.h>
#include <Util/Container/Vector.h>
#include <Util/Defines.h>

namespace Actul
{

template<>
SHMManager * SingletonClass<SHMManager>::_instance = nullptr;

SHMManager::SHMManager()
      : _allocIds(Actul::newInstance<SHMArray<SHMObjectRef>>()),
        _existingSHMs(Vector<SHMObject *>())
{
   _existingSHMs.push_back(_allocIds);
}

SHMManager::~SHMManager()
{
}

/*void SHMManager::addObservable(SHMObject * in)
 {
 in->setObserver(this);
 in->setObjId(_allocIds.size());
 _allocIds.push_back(in);
 }*/

void SHMManager::detachObservables()
{
   const size_type endI = _allocIds->size();
   for (size_type i = 0; i < endI; ++i)
   {
      SHMObjectRef & p = _allocIds->at(i);
      if (!p.staticSize) // elements static in size doesn't need to be reattached
         p.object->detach();
   }
}

void SHMManager::attachObservables()
{
   for (size_type i = 0; i < _allocIds->size(); ++i)
   {
      SHMObjectRef & tmp = _allocIds->at(i);
      if (!tmp.staticSize)
      {
         tmp.object->setShmId(tmp.shmId);
         tmp.object->setMemSize(tmp.bytesAllocated);
         tmp.object->attach();
      }
   }
}

void SHMManager::destroyAll()
{
   for (size_type i = 0; i < _existingSHMs.size(); ++i)
      _existingSHMs[i]->destroy();
   _existingSHMs.clear();
}


void SHMManager::detachAll()
{
   for (size_type i = 0; i < _existingSHMs.size(); ++i)
      _existingSHMs[i]->detach();
}

void SHMManager::notify(const size_type& objId, const int& shmId, const size_type & bytesAllocated)
{
   SHMObjectRef & tmp = _allocIds->at(objId - _allocIds->getObjectId() - 1);
   tmp.shmId = shmId;  // the id of allocIds is the first managed by this class so its the first/offset
   tmp.bytesAllocated = bytesAllocated;
}


size_type SHMManager::bytesAllocated() const
{
   size_type res = 0;
   for(size_type i=0;i<_allocIds->size();++i)
      res += _allocIds->at(i).bytesAllocated;
   //printStr("SHMMem: " + to_string(res) + "\n");
   return res;
}

}
