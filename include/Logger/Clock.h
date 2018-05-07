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
#ifndef CLOCK_HPP_
#define CLOCK_HPP_

#include <Util/LibcAllocator.h>
#include <Util/String.h>
#include <Util/TypeDefs.h>
#include <vector>
#include <cstdint>
#include <Util/Container/Vector.h>

namespace Actul
{

class Clock;

template<>
String to_string<Clock>(const Clock & c);

typedef Vector<int32_t> RawClock;
class Clock
{

   friend String to_string<Clock>(const Clock & c);

 public:
   Clock();

   Clock(const size_type & num);

   Clock(const Clock & in);

   Clock & operator=(const Clock & in);

   bool operator==(const Clock & in) const;
   bool operator!=(const Clock & in) const;

   void resize(const size_type & num);

   bool happendBefore(const Clock & in, const tid_type & t1, const tid_type & t2) const;

   bool isNotOrdered(const Clock & in, const tid_type & t1, const tid_type & t2) const;

   Clock & max(const Clock & in);


   /**
    * maxBarrierClock() has to be called with each thread id [tid].
    * Internally it checks if a thread already was set in the clock.
    * If a thread was already set, it returns false, otherwise true
    * After every thread called this function, the function normalizeBarrierClock() has to be called
    */
   bool maxBarrierClock(const tid_type & tid, const Clock & in);

   bool normalizeBarrierClock();

   void tick(const tid_type & tid);

 private:
   RawClock _clock;

   size_type maxSize(const Clock & in) const;
   size_type minSize(const Clock & in) const;

};

typedef Vector<Clock> ClockVec;

} /* namespace Actul */

#endif /* CLOCK_HPP_ */
