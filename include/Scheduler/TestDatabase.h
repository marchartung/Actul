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
#ifndef DATARACEDATABASE_H_
#define DATARACEDATABASE_H_

#include <Scheduler/DataRaceAttribute.h>
#include <Scheduler/TestReport.h>

namespace Actul
{

class TestDatabase
{
   TestDatabase(TestDatabase&&) = delete;
   TestDatabase(const TestDatabase&) = delete;

 public:

   TestDatabase(const TestSettings & setting);

   void addTestData(const bool & violationHappend, const DataRaceShmVec & races, const ReplayTokenShmVec & replay, const YieldEventShmVec & yields, AddRaceStructShmVec & addedDr);

   void getAddedDataRaces(AddRaceStructShmVec & added, const DataRaceShmVec & races) const;

   size_type numDataRaceCandidates() const;

   size_type numDataRaces() const;

   size_type getPermutableDataRaceId(const size_type & permutId) const;

   const DataRace& getDataRace(const size_type & did, const size_type & replayId = invalid<size_type>()) const;

   const ReplayTokenVec & getReplay(const size_type & rip) const;

   size_type getReplayWith(const SizeVec & ids) const;

   DataRaceAttribute getAttribute(const size_type & did) const;

   bool dataRaceIsPermuted(const size_type & drId) const;

   bool hasViolatingExecution() const;

   void setTested(const DataRace & d);

 private:

   struct DataRaceIterativAttribute
   {
      bool noViolation[2];
      bool violation[2];
      bool wasTested;
      Vector<Array<bool, 2>> introducing;

      DataRaceIterativAttribute();

      void updateOnRun(const size_type & maxId, bool violationHappend, const size_type & order, const AddRaceStructShmVec & drs, const bool & setIntoducing);

      bool isPermuted() const;

      bool isSHarmful() const;

      bool isIntroducing() const;

      bool isViolating() const;

      bool isOrderDependent() const;

      Vector<size_type> findIntroduced(const size_type & start = 0) const;

   };

   struct DataRaceInfo
   {
      size_type referenceTest;
      DataRaceIterativAttribute itAttribute;

      DataRaceInfo()
            : referenceTest(invalid<size_type>()),
              itAttribute(),
              isPermuted(false)
      {
      }

      DataRaceInfo(const size_type & refTestId)
            : referenceTest(refTestId),
              itAttribute(),
              isPermuted(false)
      {
      }

      void setPermuted()
      {
         isPermuted = true;
      }

      const bool & wasPermuted() const
      {
         return isPermuted;
      }

    private:

      bool isPermuted;

   };

   struct DataRaceTest
   {
      Vector<DataRace> dataRaces;
      ReplayTokenVec replay;
      YieldEventVec yields;
      DataRaceTest();

      DataRaceTest(const size_type & maxId, const DataRaceShmVec & races, const ReplayTokenShmVec & replay, const YieldEventShmVec & yields,
                   const AddRaceStructShmVec & addedRaces);

      bool contains(const size_type & dataRaceId) const;

      const DataRace & get(const size_type & dataRaceId) const;
   };
   bool _hasViolating;
   const TestSettings & _settings;
   SizeVec _permutedDataRaces;
   Vector<DataRaceTest> _drTests;
   Vector<DataRaceInfo> _drInfos;

   bool add(const DataRace & d, AddRaceStruct & addDr);
   bool update(const DataRace & d, AddRaceStruct & addDr);

   AddRaceStruct getId(const DataRace & d) const;

   size_type findDataRace(const DataRace & d) const;
};

} /* namespace Actul */

#endif /* DATARACEDATABASE_H_ */
