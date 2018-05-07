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
#include <Util/Semaphore.h>
#include <Util/DebugRoutines.h>
#include <Scheduler/ThreadInfo.h>

namespace Actul
{


Semaphore::Semaphore()
   : _isWaiting(false),
     _isRunning(false),
     _lastPoster(-1)
{
}

Semaphore::~Semaphore()
{
}


void Semaphore::init()
{
   actulAssert(sem_init(&_semaphore, 0, 0) == 0, "Semaphore: Cannot init sem");
   _isWaiting.store(false,std::memory_order::memory_order_release);
   _isRunning.store(true,std::memory_order::memory_order_release);
}

void Semaphore::deinit()
{
   actulAssert(sem_destroy(&_semaphore) == 0, "Semaphore: Cannot destroy sem");
   _isWaiting.store(false,std::memory_order::memory_order_release);
   _isRunning.store(false,std::memory_order::memory_order_release);
}

void Semaphore::post(const tid_type & posterTid)
{
   _lastPoster.store(posterTid,std::memory_order::memory_order_release);
   actulAssert(sem_post(&_semaphore)==0,"Semaphore: Cannot post sem");
}

void Semaphore::wait()
{
   _isWaiting.store(true,std::memory_order::memory_order_release);
   actulAssert(sem_wait(&_semaphore)==0,"Semaphore: Cannot wait sem");
   _isWaiting.store(false,std::memory_order::memory_order_release);
}

bool Semaphore::isThreadWaiting() const
{
   return _isWaiting.load(std::memory_order::memory_order_acquire);
}

bool Semaphore::isInitialized() const
{
   return _isRunning.load(std::memory_order::memory_order_acquire);
}

tid_type Semaphore::getLastPostThreadId() const
{
   return _lastPoster.load(std::memory_order::memory_order_acquire);
}
} /* namespace Actul */
