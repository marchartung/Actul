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
#ifndef INCLUDE_UTIL_THREADLOCKGATE_H_
#define INCLUDE_UTIL_THREADLOCKGATE_H_

#include <vector>
#include <array>
#include <atomic>
#include <semaphore.h>
#include <Util/Semaphore.h>

#include <Util/Defines.h>


namespace Actul
{

class ThreadLockGate
{
   typedef std::atomic<size_type> AtomicSizeType;
 public:
   ThreadLockGate();

   ~ThreadLockGate();


   void init(const tid_type & releaseTid);
   void deinit(const tid_type & releaseTid);

   void yieldAndRelease(const tid_type & releaseTid);

   void releaseBlocking(const tid_type & releaseTid);
   void releaseNoWait(const tid_type & releaseTid);

   void exitRelease(const tid_type & toRelease, const tid_type & toExit);

   void yield();

   void waitForThreadToYield(const tid_type & tid) const;

   void waitForThreadToDeinit(const tid_type & tid);

   size_type size() const;

 private:
   bool _releaserExits;
   Array<Semaphore,ACTUL_MAX_NUM_THREADS> _sems; // using pointer is important, if not adding a thread can lead to reallocation of the semaphore, leading to undefinded behavior of waiting threads
};

}
#endif /* INCLUDE_UTIL_THREADLOCKGATE_H_ */
