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
#include <Util/Container/SHM/SHMemory.h>
#include <Util/EnvironmentDependencies.h>
#include <Util/DebugRoutines.h>
#include <Scheduler/ProcessInfo.h>

#include <cstdio>
#include <errno.h>
#include <fcntl.h>

namespace Actul
{

int SHMemory::_counter = 0;

SHMemory::SHMemory()
      : _id(_counter++),
        _mode(0666),
        _key(createKey()),
        _shmId(allocate()),
        _size(minSize()),
        _data(attach())
{
}

SHMemory::~SHMemory()
{
}

int SHMemory::allocate(const size_type & size)
{
   int res;
   actulAssert((res = shmget(_key, size, _mode | IPC_CREAT)) != -1,"Couldn't create shm memory.");
   _size = size;
   return res;
}

key_t SHMemory::createKey() const
{
   key_t res = 0;
   //int f;
   //f = InternalOpen(_keyPhrase.c_str(), O_CREAT, 0400);
   //actulAssert(f >= 0,"Couldn't create key file for shm");
   //InternalClose(f);
   actulAssert((res = ftok(ProcessInfo::getProgramName(), _id)) != -1, "Couldn't create key for shm on file");
   cleanUpLastRun(res);
   return res;
}

void SHMemory::cleanUpLastRun(const key_t & in) const
{
   int shmId = shmget(in, 1, _mode | IPC_CREAT);
   if (shmId != -1)
      actulAssert (shmctl(shmId, IPC_RMID, NULL) != -1,"Cleanup in cleanup failed");
}

char * SHMemory::attach()
{
   void * tmp = shmat(_shmId, (void*) 0, 0);
   if (tmp == (void *) -1)
   {
      switch (errno)
      {
         case ENOMEM:
            InternalPrintF("ENOMEM\n");
            break;
         case EACCES:
            InternalPrintF("EACCES\n");
            break;
         case EIDRM:
            InternalPrintF("EIDRM\n");
            break;
         case EINVAL:
            InternalPrintF("EINVAL\n");
            break;
         default:
            InternalPrintF("UNKNOWN\n");
      }
      actulAssert(false,"Couldn't attach to shm memory.");
   }
   return reinterpret_cast<char *>(tmp);
}

void SHMemory::detach()
{
   if (_data != nullptr)
      actulAssert(shmdt((void*) _data) != -1,"Couldn't detach from shm memory.");
   _data = nullptr;
}

void SHMemory::deallocate()
{
   actulAssert(shmctl(_shmId, IPC_RMID, NULL) != -1,"Couldn't remove shm memory.");
}

size_type SHMemory::size() const
{
   return _size;
}

char* SHMemory::data()
{
   return _data;
}

const char* SHMemory::data() const
{
   return _data;
}

size_type SHMemory::minSize()
{
   return sizeof(size_t) * 2;
}

}
