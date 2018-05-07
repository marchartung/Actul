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
#include <Util/Container/SHM/SHMObject.h>

namespace Actul
{

SHMObserver::SHMObserver()
{

}

SHMObserver::~SHMObserver()
{

}

SHMObject::SHMObject(SHMObserver * in)
      : _observer(in)
{
}

void SHMObject::setMemSize(const size_type& num)
{
   _mem._size = num;
}

size_type SHMObject::getMemSize() const
{
   return _mem._size;
}

SHMObject::SHMObject()
      : _observer(nullptr)
{
}

SHMObject::~SHMObject()
{

}

void SHMObject::setObserver(SHMObserver * observer)
{
   _observer = observer;
}

void SHMObject::destroy()
{
   _mem.deallocate();
}

void SHMObject::attach()
{
   _mem._data = _mem.attach();
}

void SHMObject::detach()
{
   _mem.detach();
}

int SHMObject::getShmId() const
{
   return _mem._shmId;
}

void SHMObject::setShmId(const int & id)
{
   _mem._shmId = id;
}

int SHMObject::getObjectId() const
{
   return _mem._id;
}

void SHMObject::reallocate(const size_type & numBytes)
{
   _mem.detach();
   _mem.deallocate();
   _mem._shmId = _mem.allocate(numBytes);
   _mem._data = _mem.attach();
   if (_observer != nullptr)
      _observer->notify(_mem._id, _mem._shmId,_mem.size());
}

}
