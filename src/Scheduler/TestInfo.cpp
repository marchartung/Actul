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

#include <Scheduler/TestInfo.h>

namespace Actul
{

template<>
String to_string<FailedProperty>(const FailedProperty & p)
{
   String res;
   switch(p)
   {
      case FailedProperty::NO_FAILED_PROPERTY:
         res = "NO_FAILED_PROPERTY";
         break;
      case FailedProperty::DOUBLE_INITIALIZE_LOCK:
      res = "DOUBLE_INITIALIZE_LOCK";
      break;
      case FailedProperty::DOUBLE_ACQUIRE_LOCK:
      res = "DOUBLE_ACQUIRE_LOCK";
      break;
      case FailedProperty::UNLOCK_NOT_OWNED_LOCK:
      res = "UNLOCK_NOT_OWNED_LOCK";
      break;
      case FailedProperty::ACCESS_UNINITIALIZED_LOCK:
      res = "ACCESS_UNINITIALIZED_LOCK";
      break;
      case FailedProperty::ACCESS_UNINITIALIZED_BARRIER:
      res = "ACCESS_UNINITIALIZED_BARRIER";
      break;
      case FailedProperty::ACCESS_UNINITIALIZED_CONDITIONAL:
      res = "ACCESS_UNINITIALIZED_CONDITIONAL";
      break;
      case FailedProperty::NUM_THREADS_SUCCEEDED:
      res = "NUM_THREADS_SUCCEEDED";
      break;
      case FailedProperty::NO_SUCH_PID:
      res = "NO_SUCH_PID";
      break;
      case FailedProperty::THREAD_NOT_JOINABLE:
      res = "THREAD_NOT_JOINABLE";
      break;
      case FailedProperty::WRONG_DESTROY_LOCK:
      res = "WRONG_DESTROY_LOCK";
      break;
      case FailedProperty::NOT_OWNED_COND_LOCK:
      res = "NOT_OWNED_COND_LOCK";
      break;
      case FailedProperty::DOUBLE_SYNC_INITIALIZE:
      res = "DOUBLE_SYNC_INITIALIZE";
      break;
      case FailedProperty::INVALID_SYNC_DESTROY:
      res = "INVALID_SYNC_DESTROY";
      break;
      case FailedProperty::WRONG_SYNC_CALL_TYPE:
      res = "WRONG_SYNC_CALL_TYPE";
      break;
      case FailedProperty::INVALID_FREE:
      res = "INVALID_FREE";
      break;
      case FailedProperty::NO_MEMORY:
      res = "NO_MEMORY";
      break;
      case FailedProperty::DEADLOCK:
      res = "DEADLOCK";
      break;
      case FailedProperty::INVALID_ALLOCATION:
      res = "INVALID_ALLOCATION";
      break;
      case FailedProperty::SEGMENTATION_FAULT:
      res = "SEGMENTATION_FAULT";
      break;

      default:
         actulAssert(false,"No string for program property defined");
   }
   return res;
}

TestInfo::TestInfo()
      : failedProperty(FailedProperty::NO_FAILED_PROPERTY),
        isRunning(true),
        replayFailed(false),
        returnValue(invalid<int>()),
        replayId(invalid<size_type>()),
        workerId(invalid<size_type>()),
        progressCounter(0),
        runtime(0.0),
        isInScheduler(false)
{
   isInScheduler[0] = true; // main thread is during creation in the scheduler
}

bool TestInfo::hasViolation() const
{
   return returnValue != 0;
}

void TestInfo::setProgress()
{
   ++progressCounter;
}

size_type TestInfo::getProgressCounter() const
{
   return progressCounter;
}

void TestInfo::clear()
{
   *this = TestInfo();
}

}
