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
#ifndef LOCKSET_HPP_
#define LOCKSET_HPP_

#include <vector>

#include <Util/TypeDefs.h>
#include <Util/String.h>
#include <Util/Container/Vector.h>

namespace Actul
{

class LockSet
{
   friend String to_string<LockSet>(const LockSet & c);
   typedef Vector<int32_t> RawLockSet;
 public:

   LockSet();
   LockSet(const LockSet & in);

   LockSet & operator=(const LockSet & in);

   bool operator==(const LockSet & in) const;

   void addWriteLock(const event_id & in);
   void addReadLock(const event_id & in);
   void removeLock(const event_id & in);

   bool exclude(const LockSet & in) const;

   bool isNotExcluded(const LockSet & in) const;

 private:
   RawLockSet _locks;

   int32_t getLockVal(const event_id & event) const;
   event_id getEventId(const int32_t & lock) const;
};

typedef Vector<LockSet> LockSetVec;

template<>
String to_string<LockSet>(const LockSet & c);

} /* namespace Actul */

#endif /* LOCKSET_HPP_ */
