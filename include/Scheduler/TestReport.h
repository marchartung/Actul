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
#ifndef INCLUDE_ANALYSE_TESTREPORT_HPP_
#define INCLUDE_ANALYSE_TESTREPORT_HPP_

#include <Logger/DataRace.h>
#include <Util/Defines.h>
#include <Logger/Event.h>
#include <Logger/YieldEvent.h>
#include <Scheduler/TestInfo.h>
#include <Scheduler/ThreadInfo.h>
#include <Util/Container/SHM/SHMArray.h>
#include <Util/Container/SHM/SHMWrapper.h>

namespace Actul
{

struct ReplayToken
{
   bool readViolated;
   bool releaseViolated;
   bool constrainViolated;
   EventType et;
   tid_type tid;
   size_type numAccesses;

   ReplayToken & operator=(ReplayToken &&) = delete;

   ReplayToken()
         : ReplayToken(false, false, false, EventType::UNDEFINED, invalid<tid_type>())
   {
   }

   ReplayToken(const bool & violatedRead, const bool & violatedRelease, const bool & violatedConstrain, const EventType & et, const tid_type & tid)
         : readViolated(violatedRead),
           releaseViolated(violatedRelease),
           constrainViolated(violatedConstrain),
           et(et),
           tid(tid),
           numAccesses(ThreadInfo::getNumAccesses())
   {
   }

   ReplayToken(const MemoryAccess & a)
         : readViolated(false),
           releaseViolated(false),
           constrainViolated(false),
           et(makeAccessType(a.isWrite, a.isAtomic)),
           tid(a.tid),
           numAccesses(a.numThreadAccesses)
   {

   }

   ReplayToken(const ReplayToken & in)
         : readViolated(in.readViolated),
           releaseViolated(in.releaseViolated),
           constrainViolated(in.constrainViolated),
           et(in.et),
           tid(in.tid),
           numAccesses(in.numAccesses)
   {

   }
   ReplayToken(ReplayToken && in)
         : readViolated(in.readViolated),
           releaseViolated(in.releaseViolated),
           constrainViolated(in.constrainViolated),
           et(in.et),
           tid(in.tid),
           numAccesses(in.numAccesses)
   {

   }

   ReplayToken & operator=(const ReplayToken & in)
   {
      readViolated = in.readViolated;
      releaseViolated = in.releaseViolated;
      constrainViolated = in.constrainViolated;
      et = in.et;
      tid = in.tid;
      numAccesses = in.numAccesses;
      return *this;
   }

   ReplayToken & operator=(const MemoryAccess & in)
   {
      readViolated = false;
      releaseViolated = false;
      constrainViolated = false;
      et = makeAccessType(in.isWrite, in.isAtomic);
      tid = in.tid;
      numAccesses = in.numThreadAccesses;
      return *this;
   }

   bool operator<(const ReplayToken & t) const
   {
      return numAccesses < t.numAccesses;
   }

   bool operator>(const ReplayToken & t) const
   {
      return numAccesses > t.numAccesses;
   }

   bool operator==(const ReplayToken & in) const
   {
      return et == in.et && tid == in.tid && readViolated == in.readViolated && releaseViolated == in.releaseViolated && constrainViolated == in.constrainViolated;  // && numAccesses == in.numAccesses;
   }

   bool operator!=(const ReplayToken & in) const
   {
      return !(*this == in);
   }
};

typedef Vector<ReplayToken> ReplayTokenVec;
typedef Vector<ReplayTokenVec> ReplayTokenVecVec;

typedef SHMArray<ReplayToken> ReplayTokenShmVec;
typedef SHMArray<DataRace> DataRaceShmVec;
typedef SHMArray<YieldEvent> YieldEventShmVec;
typedef SHMArray<AddRaceStruct> AddRaceStructShmVec;

class TestReport
{
 public:
   TestReport(const TestReport& in) = delete;

   TestReport();

   ~TestReport();

   bool isFinished() const;

   bool hasDeadlock() const;

   bool hasDataRace() const;

   TestInfo & getTestInfo();
   const TestInfo & getTestInfo() const;

   DataRaceShmVec & getDataRaces();
   const DataRaceShmVec & getDataRaces() const;

   DataRaceShmVec & getTestDataRaces();
   const DataRaceShmVec & getTestDataRaces() const;

   ReplayTokenShmVec & getReplayTokenArray();
   const ReplayTokenShmVec & getReplayTokenArray() const;

   YieldEventShmVec & getYieldEvents();
   const YieldEventShmVec & getYieldEvents() const;

   AddRaceStructShmVec & getAddRaceStruct();
   const AddRaceStructShmVec & getAddRaceStruct() const;

   void clear();

   void reattach();

   void print();

   void check();

 private:
   SHMManager * _manager;

   SHMWrapper<TestInfo> * _testInfo;
   DataRaceShmVec * _dataRaces;
   DataRaceShmVec * _testDataRaces;
   YieldEventShmVec * _doneYields;
   ReplayTokenShmVec * _replay;
   AddRaceStructShmVec * _addRaces;

};

} /* namespace Actul */

#endif /* INCLUDE_ANALYSE_TESTREPORT_HPP_ */
