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
#include <Logger/LockSet.h>
#include <Util/DebugRoutines.h>
#include <Util/Utility.h>
#include <Util/Invalid.h>

#include <cmath>

namespace Actul
{

template<>
String to_string<LockSet>(const LockSet & c)
{
   String res("(");
   if (c._locks.size() > 0)
   {
      for (size_type i = 0; i < c._locks.size() - 1; ++i)
         res += to_string(c._locks[i]) + " ";
      res += to_string(c._locks[c._locks.size() - 1]);
   }
   res += ")";
   return res;
}

LockSet::LockSet()
      : _locks(0)
{
}

LockSet::LockSet(const LockSet& in)
      : _locks(in._locks)
{
}

int32_t LockSet::getLockVal(const event_id & event) const
{
   return static_cast<int32_t>(event)+1;
}

event_id LockSet::getEventId(const int32_t & lock) const
{
   return static_cast<event_id>(Abs(lock)-1);
}

LockSet& LockSet::operator =(const LockSet& in)
{
   _locks = in._locks;
   return *this;
}

// locks are shifted by one for distinguish between read and write locks
void LockSet::addWriteLock(const event_id& in)
{
   _locks.push_back(getLockVal(in));
}

void LockSet::addReadLock(const event_id& in)
{
   _locks.push_back(-getLockVal(in));
}

void LockSet::removeLock(const event_id& in)
{
   size_type toSwap = invalid<size_type>();
   int32_t tmp = getLockVal(in);
   for (size_type i = 0; i < _locks.size(); ++i)
      if (Abs(_locks[i]) == tmp)
      {
         toSwap = i;
         break;
      }
   actulAssert(toSwap != invalid<size_type>(),"LockSet: Lock was not in the lock set");
   _locks.unordered_remove(toSwap);

}

bool LockSet::exclude(const LockSet& in) const
{
   const RawLockSet & l1 = _locks;
   const RawLockSet & l2 = in._locks;
   for (size_type i=0;i<l1.size();++i)
      for (size_type j=0;j<l2.size();++j)
         if (Abs(l1[i]) == Abs(l2[j]) && (l1[i] > 0 || l2[j] > 0))  // one has to be a write lock for exclusion
         {
            return true;
         }
   return false;
}

bool LockSet::isNotExcluded(const LockSet& in) const
{
   return !exclude(in);
}

} /* namespace Actul */

bool Actul::LockSet::operator ==(const LockSet& in) const
{
   if (in._locks.size() == _locks.size())
   {
      for (size_type i = 0; i < _locks.size(); ++i)
         if (_locks[i] != in._locks[i])
            return false;
   }
   else
      return false;
   return true;

}
