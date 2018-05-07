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
#ifndef INCLUDE_THREADINFO_H_
#define INCLUDE_THREADINFO_H_

#include <Util/Defines.h>

namespace Actul
{


class ThreadInfo
{
   ThreadInfo() = delete;
   ThreadInfo(const ThreadInfo&) = delete;

 public:

   static bool isInScheduler();

   static void setInScheduler(const bool &  isScheduler);

   static const tid_type & getThreadId();

   static void setThreadId(const tid_type & tid);
/*
   static size_type getEpochId();
   static void setEpochId(const size_type & eId);
*/
   static const size_type & getNumAccesses();
   static void increaceNumAccesses();

 private:
   static thread_local volatile bool _inScheduler;
   static thread_local tid_type _tid;
   static thread_local volatile size_type _epochId;
   static thread_local size_type _numAccessesOnThread;
};

} /* namespace Actul */

#endif /* INCLUDE_THREADINFO_H_ */
