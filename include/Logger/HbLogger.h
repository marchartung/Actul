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
#ifndef HBBUFFER_HPP_
#define HBBUFFER_HPP_

#include <Util/Defines.h>
#include <Logger/Clock.h>
#include <Logger/Event.h>
#include <Logger/EpochLogger.h>
#include <Util/DebugRoutines.h>
#include <Util/SingletonClass.h>

#include <Util/Container/Array.h>

namespace Actul
{

struct HbLoggerMemorySpec
{
   size_type historySize;
};

class HbLogger : public SingletonClass<HbLogger>
{
   friend class SingletonClass<HbLogger>;

   HbLogger(HbLogger&&) = delete;
   HbLogger(const HbLogger&) = delete;

   struct ThreadHbState
   {
      size_type curClockId;
   };
   typedef Array<ThreadHbState, ACTUL_MAX_NUM_THREADS> ThreadHbStateArray;

 public:

   size_type curThreadTickClock();

   void threadHappendAfterClock(const tid_type & tid, const size_type & clockId);

   void curThreadHappendAfterClock(const size_type & clockId);

   void barrierOnThreads(const TidArray & tids);

   const Clock & getClock(const size_type & cId) const;
   const size_type & getClockId(const tid_type & tid) const;

   const size_type & getCurClockId() const;
   const Clock & getCurClock() const;

 private:
   EpochLogger & _epLogger;
   ThreadHbStateArray _threadStates;
   ClockVec _clockHistory;

   HbLogger();
   ~HbLogger();

   size_type createNewClock(const size_type & clockId);

};
} /* namespace Actul */
#endif /* HBBUFFER_HPP_ */
