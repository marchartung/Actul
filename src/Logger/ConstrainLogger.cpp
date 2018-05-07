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
#include <Logger/ConstrainLogger.h>

namespace Actul
{

Constrainer::Constrainer(const DataRace & d)
      : yielder((d.order()) ? d.getAccess(0) : d.getAccess(1)),
        releaser((d.order()) ? d.getAccess(1) : d.getAccess(0))
{
}

Constrainer::Constrainer(const YieldEvent & ye)
      : yielder(ye)
{
}

Constrainer::Constrainer()
{
}

bool Constrainer::isReleased() const
{
   return !releaser.isValid();
}

void Constrainer::release()
{
   releaser.invalidate();
}

ConstrainLogger::ConstrainLogger(const TestSettings & settings)
      : _settings(settings),
        _evLogger(*EventLogger::getInstance()),
        _fsLogger(*FunctionStackImpl::getInstance()),
        _hbLogger(*HbLogger::getInstance()),
        _lsLogger(*LockSetLogger::getInstance()),
        _maxNumReads(settings.maxSequentialReads),
        _shouldYield(false)
{
}

bool ConstrainLogger::isCurrentThreadBlocked() const
{
   return isThreadBlocked(ThreadInfo::getThreadId());
}

bool ConstrainLogger::shouldCurrentThreadYield() const
{
   return _shouldYield[ThreadInfo::getThreadId()] || isCurrentThreadBlocked();
}

bool ConstrainLogger::isThreadBlocked(const tid_type& tid) const
{
   return isThreadReadBlocked(tid) || isThreadConstrained(tid);
}

bool ConstrainLogger::isThreadReadBlocked(const tid_type& tid) const
{
   return _readCounter[tid] >= _maxNumReads;
}

bool ConstrainLogger::isThreadConstrained(const tid_type& tid) const
{
   return _curThreadConstrainer[tid].size() > 0;
}


void ConstrainLogger::violateReadConstrain(const tid_type& tid)
{
   _readCounter[tid] = 0;
}

void ConstrainLogger::removeConstrainsFromThread(const tid_type& tid)
{
   _curThreadConstrainer[tid].clear();
}

size_type ConstrainLogger::findEnforcedYieldEvent(const YieldEvent & yield) const
{
   size_type res = invalid<size_type>();
   for (size_type i = 0; i < _enforceYields[yield.tid].size(); ++i)
      if (_constrains[_enforceYields[yield.tid][i]].yielder == yield)
      {
         res = _enforceYields[yield.tid][i];
         break;
      }
   return res;
}

void ConstrainLogger::addYieldEvent(const YieldEvent & yield)
{
   size_type pos = findEnforcedYieldEvent(yield);
   if (pos == invalid<size_type>())
   {
      Constrainer c(yield);
      size_type num = _constrains.size();
      _constrains.push_back(c);
      Constrainer & cs = _constrains.back();
      cs.yielder.id = _evLogger.addEvent(cs.yielder.type, cs.yielder.width, cs.yielder.addr);
      _enforceYields[cs.yielder.tid].push_back(num);
   }
}

void ConstrainLogger::addDataRaceEvent(const DataRace & dr)
{
   Constrainer c(dr);
   bool insertNewConstrainer = false;
   size_type pos = findEnforcedYieldEvent(c.yielder);
   if (pos == invalid<size_type>() || (!c.isReleased() && c.releaser != _constrains[pos].releaser))
      insertNewConstrainer = true; // insert new when Constrainer is completely new or the existing constrainer is another real constrainer

   if(!insertNewConstrainer)
   { // reuse the existing constrain cunstructed by addYieldEvent():
      actulAssert(_constrains[pos].isReleased(), "ConstrainLogger: assumed constrain can be overwritten, but was mistaken");
      _constrains[pos].releaser = c.releaser;
      _constrains[pos].releaser.id = _evLogger.addEvent(c.releaser.type, c.releaser.width, c.releaser.addr);
      _constrainReleaser[_constrains[pos].releaser.tid].push_back(pos);
   }
   else
   { // create new constrainer:
      pos = _constrains.size();
      _constrains.push_back(c);
      Constrainer & cs = _constrains.back();
      cs.yielder.id = _evLogger.addEvent(cs.yielder.type, cs.yielder.width, cs.yielder.addr);
      cs.releaser.id = _evLogger.addEvent(cs.releaser.type, cs.releaser.width, cs.releaser.addr);
      _enforceYields[cs.yielder.tid].push_back(pos);
      _constrainReleaser[cs.releaser.tid].push_back(pos);

   }
}

} /* namespace Actul */
