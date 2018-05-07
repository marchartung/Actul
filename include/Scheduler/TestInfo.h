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
#ifndef INCLUDE_TESTINFO_HPP_
#define INCLUDE_TESTINFO_HPP_

#include <Util/String.h>
#include <Scheduler/TestSettings.h>

namespace Actul
{
enum FailedProperty
{
   NO_FAILED_PROPERTY,

   // NON CRITICAL
   DOUBLE_INITIALIZE_LOCK, // double initialize a lock
   DOUBLE_ACQUIRE_LOCK,
   UNLOCK_NOT_OWNED_LOCK,
   ACCESS_UNINITIALIZED_LOCK,
   ACCESS_UNINITIALIZED_BARRIER,
   ACCESS_UNINITIALIZED_CONDITIONAL,
   NUM_THREADS_SUCCEEDED,
   NO_SUCH_PID,
   THREAD_NOT_JOINABLE,
   WRONG_DESTROY_LOCK, // destroy lock which is not free or not initialized
   NOT_OWNED_COND_LOCK,
   DOUBLE_SYNC_INITIALIZE, // a condition, barrier or semaphore was initialized twice
   INVALID_SYNC_DESTROY, //a condition, barrier or semaphore was destroyed without initialize
   WRONG_SYNC_CALL_TYPE, // a condition, barrier or semaphore was called with wrong function

   //CRITICAL:
   INVALID_FREE,// DONT MOVE!!! invalid free or double free
   NO_MEMORY,
   DEADLOCK,
   INVALID_ALLOCATION,
   SEGMENTATION_FAULT
};

template<>
String to_string<FailedProperty>(const FailedProperty & p);

struct TestInfo
{
   FailedProperty failedProperty;
   bool isRunning;
   bool replayFailed;
   int returnValue;
   size_type replayId;
   size_type workerId;
   size_type progressCounter;
   double runtime;
   BoolArray isInScheduler;

   TestInfo();

   bool hasViolation() const;

   void clear();

   void setProgress();

   size_type getProgressCounter() const;

};

}

#endif /* INCLUDE_TESTINFO_HPP_ */
