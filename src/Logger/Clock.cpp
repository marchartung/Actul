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
#include <Logger/Clock.h>
#include <cmath>

namespace Actul
{

template<>
String to_string<Clock>(const Clock & c)
{
   String res("[");
   for (size_type i = 0; i < c._clock.size() - 1; ++i)
   {
      res += to_string(c._clock[i]) + ",";
   }
   res += to_string(c._clock.back()) + "]";
   return res;
}

Clock::Clock()
      : _clock(2, 0)
{
}

Clock::Clock(const size_type & num)
      : _clock(num, 0)
{

}

Clock::Clock(const Clock& in)
      : _clock(in._clock)
{
}

Clock& Clock::operator =(const Clock& in)
{
   _clock = in._clock;
   return *this;
}


void Clock::resize(const size_type & num)
{
   _clock.resize(num,0);
}

bool Clock::isNotOrdered(const Clock& in, const tid_type & t1, const tid_type & t2) const
{
   RawClock::value_type v1t1 = (_clock.size() > t1) ? _clock[t1] : 0,
                        v1t2 = (_clock.size() > t2) ? _clock[t2] : 0,
                        v2t1 = (in._clock.size() > t1) ? in._clock[t1] : 0,
                        v2t2 = (in._clock.size() > t2) ? in._clock[t2] : 0;
   return (v1t1 < v2t1 && v1t2 >= v2t2) ||
          (v1t1 >= v2t1 && v1t2 < v2t2);
}

bool Clock::operator ==(const Clock& in) const
{
   size_type msize = maxSize(in);

   RawClock::value_type a, b;
   for (size_type i = 0; i < msize; ++i)
   {
      a = (_clock.size() > i) ? _clock[i] : -1;
      b = (in._clock.size() > i) ? in._clock[i] : -1;
      if (a != b)
         return false;
   }
   return true;  // TODO maybe check for not checked overlap if it is 0???
}

bool Clock::operator !=(const Clock& in) const
{
   return !(*this == in);
}

Clock & Clock::max(const Clock& in)
{
   RawClock::value_type a, b;
   for (size_type i = 0; i < _clock.size(); ++i)
   {
      a = (_clock.size() > i) ? _clock[i] : 0;
      b = (in._clock.size() > i) ? in._clock[i] : 0;
      _clock[i] = (a < b) ? b : a;
   }
   return *this;
}

bool Actul::Clock::maxBarrierClock(const tid_type& tid, const Clock & in)
{
   bool res = false;
   if (_clock.size() < in._clock.size())
      _clock.resize(in._clock.size(), 0);
   if (_clock[tid] >= 0)
   {
      _clock[tid] = in._clock[tid] + 1;
      uint32_t a, b;
      for (size_type i = 0; i < in._clock.size(); ++i)
      {
         a = (_clock.size() > i) ? std::abs(_clock[i]) : 0;
         b = (in._clock.size() > i) ? in._clock[i] : 0;
         _clock[i] = -((a < b) ? b : a);
      }
      res = true;
   }
   return res;
}

void Clock::tick(const tid_type& tid)
{
   if(_clock.size() <= tid)
      _clock.resize(tid+1,0);
   ++_clock[tid];
}

size_type Clock::maxSize(const Clock& in) const
{
   return (in._clock.size() < _clock.size()) ? _clock.size() : in._clock.size();
}

bool Clock::normalizeBarrierClock()
{
   for (size_type i = 0; i < _clock.size(); ++i)
      _clock[i] = -_clock[i];
   return true;
}

size_type Clock::minSize(const Clock& in) const
{
   return (_clock.size() < in._clock.size()) ? _clock.size() : in._clock.size();
}

} /* namespace Actul */
