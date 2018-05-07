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
#ifndef INCLUDE_LOGGER_CONSTRAINLOGGER_H_
#define INCLUDE_LOGGER_CONSTRAINLOGGER_H_

#include <Util/Defines.h>
#include <Scheduler/ThreadInfo.h>
#include <Logger/DataRace.h>
#include <Logger/Event.h>
#include <Logger/EventLogger.h>
#include <Logger/HbLogger.h>
#include <Logger/LockSetLogger.h>
#include <Logger/YieldEvent.h>

#include <Util/Container/Vector.h>
#include <Util/Container/Array.h>

namespace Actul
{

struct Constrainer
{
   YieldEvent yielder;
   YieldEvent releaser;

   Constrainer();

   Constrainer(const DataRace & d);

   Constrainer(const YieldEvent & ye);

   bool isReleased() const;

   void release();
};

class ConstrainLogger
{
   ConstrainLogger(const ConstrainLogger&) = delete;
   ConstrainLogger(ConstrainLogger&&) = delete;
   ConstrainLogger() = delete;

 public:

   ConstrainLogger(const TestSettings & settings);

   template<EventType et>
   void notifyEventPre(const event_id & id)
   {
      const tid_type & tid = ThreadInfo::getThreadId();

      // update read counter
      if (isRead<et>())
         ++_readCounter[tid];
      else
         _readCounter[tid] = 0;

      StackArray tmp;
      // look up if thread needs to be yielded/blocked:
      for (size_type i = 0; i < _enforceYields[tid].size(); ++i)
      {
         Constrainer & c = _constrains[_enforceYields[tid][i]];
         YieldEvent & ye = c.yielder;
         if (ye.id == id && ye.type == et && _fsLogger.getCurStackTop() == ye.stack[0] && (ye.isSimpleYield() || _evLogger.getCurInstanceNum<et>() == ye.instanceNum))
            if (_fsLogger.getCurStack(tmp) == ye.stack)  // expensive!!!
            {
               if(!ye.wasUsed)
               {
                  ProcessInfo::getTestReport().getYieldEvents().insert(ye);
                  ye.setUsed();
               }
               if (!ye.isSimpleYield())
               {
                  if (!c.isReleased())
                     _curThreadConstrainer[tid].push_back(_enforceYields[tid][i]);
                  _enforceYields[tid].unordered_remove(i);
               }
               _shouldYield[tid] = true;
            }

      }
   }

   template<EventType et>
   void notifyEventPost(const event_id & id)
   {
      const tid_type & tid = ThreadInfo::getThreadId();
      _shouldYield[tid] = false;
      if (isDefaultYieldEvent<et>())
      {
         _readCounter[tid] = 0;  // everytime when a thread is release, set back the successive read count
      }
      StackArray tmp;
      // look up ich thread needs to be blocked:
      for (size_type i = 0; i < _constrainReleaser[tid].size(); ++i)
      {
         Constrainer & c = _constrains[_constrainReleaser[tid][i]];
         YieldEvent & ye = c.releaser;
         if (ye.id == id && ye.type == et && _fsLogger.getCurStackTop() == ye.stack[0] && _evLogger.getCurInstanceNum<et>() - 1 == ye.instanceNum)
            if (_fsLogger.getCurStack(tmp) == ye.stack)
            {
               // lets release:
               for (size_type j = 0; j < _curThreadConstrainer[c.yielder.tid].size(); ++j)
                  if (_curThreadConstrainer[c.yielder.tid][j] == _constrainReleaser[tid][i])
                  {
                     _curThreadConstrainer[c.yielder.tid].unordered_remove(j);
                     break;
                  }
               c.release();
               _constrainReleaser[tid].unordered_remove(i);
            }
      }
   }

   bool isCurrentThreadBlocked() const;

   bool shouldCurrentThreadYield() const;

   bool isThreadBlocked(const tid_type & tid) const;

   bool isThreadReadBlocked(const tid_type & tid) const;

   bool isThreadConstrained(const tid_type & tid) const;

   void violateReadConstrain(const tid_type & tid);

   void removeConstrainsFromThread(const tid_type & tid);

   void addDataRaceEvent(const DataRace & dr);

   void addYieldEvent(const YieldEvent & yield);

   const size_type & numConstrainer() const
   {
      return _constrains.size();
   }

 private:
   const TestSettings & _settings;
   EventLogger & _evLogger;
   const FunctionStackImpl & _fsLogger;
   const HbLogger & _hbLogger;
   const LockSetLogger & _lsLogger;
   size_type _maxNumReads;
   BoolArray _shouldYield;
   SizeArray _readCounter;
   SizeVecArray _curThreadConstrainer;
   Vector<Constrainer> _constrains;
   SizeVecArray _enforceYields;
   SizeVecArray _constrainReleaser;

   size_type findEnforcedYieldEvent(const YieldEvent & yield) const;

};

} /* namespace Actul */

#endif /* INCLUDE_LOGGER_CONSTRAINLOGGER_H_ */
