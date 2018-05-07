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
#include <Logger/DataRaceLogger.h>
#include <Scheduler/TestReport.h>
#include <Scheduler/ProcessInfo.h>
#include <Implementation/PthreadImpl.h>

namespace Actul
{

DataRaceLogger::DataRaceLogger()
      : _epLogger(*EpochLogger::getInstance()),
        _evLogger(*EventLogger::getInstance()),
        _fsLogger(*FunctionStackImpl::getInstance()),
        _hbLogger(*HbLogger::getInstance()),
        _lsLogger(*LockSetLogger::getInstance())
{
}

void Actul::DataRaceLogger::findDataNewDataRaces(const event_id& id, const AddressType & addr)
{
   const AddressMemoryEventKeyVec & eVec = _evLogger.getCachedMemoryEvents(addr);
   const AddressMemoryEventKey & newKey = _evLogger.getCachedAccess(addr);
   const EventInstance & newEi = _evLogger.getCachedInstance(addr);
   actulAssert(_epLogger.getCurEpochId() == newEi.epochId, "DataRaceLogger: Inconsistent epoch ids");
   actulAssert(newEi.epochId != invalid<size_type>(), "DataRaceLogger: Epoch of thread wasn't initialized");

   for (size_type i = 0; i < eVec.size(); ++i)
   {
      ProcessInfo::getTestReport().getTestInfo().setProgress();
      const AddressMemoryEventKey & key = eVec[i];
      if ((key.isWrite() || newKey.isWrite()) && (!key.atomic || !newKey.atomic) && newKey.overlaps(key))  // check memory conditions
         findDataRaceOnKeys(addr,newEi, newKey, key);
   }
}

void DataRaceLogger::findDataRaceOnKeys(const AddressType & addr, const EventInstance & newEi, const AddressMemoryEventKey & kNew, const AddressMemoryEventKey & kOld)
{
   const Epoch & newEp = _epLogger.getEpoch(newEi.epochId);
   const Clock & newCl = _hbLogger.getClock(newEp.clockId);
   const LockSet & newLs = _lsLogger.getLockSet(newEp.lockSetId);

   const TidVec & tids = PthreadImpl::peekInstance()->getThreads();
   for (size_type k = 0; k < tids.size(); ++k)
   {
      if (tids[k] != newEp.tid)
      {
         const EventInstanceVec & eiVec = _evLogger.getEventInstances<EventType::WRITE>(kOld.eventId, tids[k]);
         for (size_type j = 0; j < eiVec.size(); ++j)
         {
            const EventInstance & ei = eiVec[j];
            actulAssert(ei.epochId != invalid<size_type>(), "DataRaceLogger: Epoch of thread wasn't initialized");
            const Epoch & ep = _epLogger.getEpoch(ei.epochId);
            const LockSet & ls = _lsLogger.getLockSet(ep.lockSetId);
            if (ls.isNotExcluded(newLs))
            {
               const Clock & c = _hbLogger.getClock(ep.clockId);
               if (c.isNotOrdered(newCl, ep.tid, newEp.tid))
                  addDataRace(DataRaceReferences(kOld, kNew, ei, newEi), addr, j);
            }

         }
      }
   }
}

void DataRaceLogger::addDataRace(const DataRaceReferences & r, const AddressType & addr, const size_type & instanceNum)
{
   MemoryAccess a1, a2;

   a1.isWrite = r.k1.write;
   a1.isAtomic = r.k1.atomic;
   a1.width = r.k1.width;
   a1.tid = _epLogger.getEpoch(r.ei1.epochId).tid;
   a1.instanceNum = instanceNum;
   a1.numTotalReleases = r.ei1.numTotalReleases - 1;
   a1.numThreadAccesses = r.ei1.numThreadAccesses;
   a1.addr = AddressCluster::calcRealAddress(addr, r.k1.addrOffset);
   _fsLogger.getStack(a1.stack, a1.tid, r.ei1.funcStackPos);

   a2.isWrite = r.k2.write;
   a2.isAtomic = r.k2.atomic;
   a2.width = r.k2.width;
   a2.tid = _epLogger.getEpoch(r.ei2.epochId).tid;
   a2.instanceNum = _evLogger.getNumInstances<EventType::WRITE>(r.k2.eventId, a2.tid) - 1;  // minus 1: the current instance doesn't count
   a2.numTotalReleases = r.ei2.numTotalReleases - 1;
   a2.numThreadAccesses = r.ei2.numThreadAccesses;
   a2.addr = AddressCluster::calcRealAddress(addr, r.k2.addrOffset);
   _fsLogger.getStack(a2.stack, a2.tid, r.ei2.funcStackPos);

   ProcessInfo::getTestReport().getDataRaces().insert(DataRace(a1, a2));
}

} /* namespace Actul */
