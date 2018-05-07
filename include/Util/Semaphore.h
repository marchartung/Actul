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
//    Marc Hartung
////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDE_UTIL_SEMAPHORE_H_
#define INCLUDE_UTIL_SEMAPHORE_H_

#include <Util/TypeDefs.h>

#include <atomic>
#include <semaphore.h>

namespace Actul
{

class Semaphore
{
 public:
   Semaphore();

   ~Semaphore();

   void init();

   void deinit();

   void post(const tid_type & posterTid);

   void wait();

   bool isThreadWaiting() const;

   bool isInitialized() const;

   tid_type getLastPostThreadId() const;

 private:
   std::atomic<bool> _isWaiting;
   std::atomic<bool> _isRunning;
   std::atomic<tid_type> _lastPoster;
   sem_t _semaphore;

};

} /* namespace Actul */

#endif /* INCLUDE_UTIL_SEMAPHORE_H_ */
