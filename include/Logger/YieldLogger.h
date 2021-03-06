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
#ifndef INCLUDE_UTIL_YIELDIMPL_HPP_
#define INCLUDE_UTIL_YIELDIMPL_HPP_

#include <Util/Defines.h>
#include <Util/ThreadLockGate.h>
#include <Util/SingletonClass.h>

#include <array>

namespace Actul
{

class YieldLogger : public SingletonClass<YieldLogger>
{
   friend class SingletonClass<YieldLogger> ;

   YieldLogger(YieldLogger&&) = delete;
   YieldLogger(const YieldLogger&) = delete;

 public:

   void init();

   void deinit();

   void yieldAndReleaseThread(const tid_type & tid);

   void yield();

   void waitForThreadToYield(const tid_type& tid) const;

   void releaseThread(const tid_type & tid);

   void exitReleaseThread(const tid_type & toRelease, const tid_type & toExit);

 private:
   tid_type _lastReleasedTid;
   ThreadLockGate _gate;

   YieldLogger();
   ~YieldLogger();

};

} /* namespace Actul */

#endif /* INCLUDE_UTIL_YIELDIMPL_HPP_ */
