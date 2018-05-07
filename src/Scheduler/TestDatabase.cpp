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
#include <Scheduler/TestDatabase.h>
#include <Scheduler/ProcessInfo.h>

namespace Actul
{

TestDatabase::DataRaceIterativAttribute::DataRaceIterativAttribute()
{
   noViolation[0] = false;
   noViolation[1] = false;
   violation[0] = false;
   violation[1] = false;
   wasTested = false;
}

void TestDatabase::DataRaceIterativAttribute::updateOnRun(const size_type & maxId, bool violationHappend, const size_type & order, const AddRaceStructShmVec & drs,
                                                          const bool & setIntoducing)
{
   if (violationHappend)
      violation[order] = true;
   else
      noViolation[order] = true;
   if (setIntoducing)
   {
      if (introducing.size() <= maxId)
      {
         Array<bool, 2> tmp(false);
         introducing.resize(maxId + 1, tmp);
      }
      for (size_type i = 0; i < drs.size(); ++i)
      {
         introducing[drs[i].id][order] = true;
      }
   }
}

size_type TestDatabase::getPermutableDataRaceId(const size_type & permutId) const
{
   actulAssert(_permutedDataRaces.size() > permutId, "TestDatabase: Permutable data race id not found");
   return _permutedDataRaces[permutId];
}

bool TestDatabase::DataRaceIterativAttribute::isPermuted() const
{
   return (noViolation[0] || violation[0]) && (noViolation[1] || violation[1]);
}

bool TestDatabase::DataRaceIterativAttribute::isSHarmful() const
{
   bool res = false;
   bool o0 = Xor(noViolation[0], violation[0]), o1 = Xor(noViolation[1], violation[1]);
   if (o0 && o1)
      res = (noViolation[0] && violation[1]) || (noViolation[1] && violation[0]);
   return res;
}

bool TestDatabase::DataRaceIterativAttribute::isIntroducing() const
{
   return findIntroduced().size() > 0;
}

bool TestDatabase::DataRaceIterativAttribute::isViolating() const
{
   return (!noViolation[0] && violation[0]) || (!noViolation[1] && violation[1]);
}

bool TestDatabase::DataRaceIterativAttribute::isOrderDependent() const
{
   return noViolation[0] != noViolation[1] || violation[0] != violation[1];
}

Vector<size_type> TestDatabase::DataRaceIterativAttribute::findIntroduced(const size_type & start) const
{
   Vector<size_type> res;
   for (size_type i = start; i < introducing.size(); ++i)
      if (introducing[i][0] + introducing[i][1] == 1)
      {
         res.push_back(i);
      }
   return res;
}

TestDatabase::TestDatabase(const TestSettings & setting)
      : _hasViolating(false),
        _settings(setting)
{

}

void TestDatabase::getAddedDataRaces(AddRaceStructShmVec & added, const DataRaceShmVec & races) const
{
   added.clear();
   for (size_type i = 0; i < races.size(); ++i)
   {
      ProcessInfo::getTestReport().getTestInfo().setProgress();
      added.insert(getId(races[i]));
   }
}

void Actul::TestDatabase::addTestData(const bool & violationHappend, const DataRaceShmVec& races, const ReplayTokenShmVec& replay, const YieldEventShmVec & yields,
                                      AddRaceStructShmVec & addedDr)
{
   bool addTest = false;
   if (violationHappend)
      _hasViolating = true;
   size_type maxId = 0;
   if (addedDr.empty())
      getAddedDataRaces(addedDr, races);
   else
      actulAssert(races.size() == addedDr.size(), "TestDatabase: Test data race information are incomplete");
   for (size_type i = 0; i < races.size(); ++i)
   {
      bool isNew = addedDr[i].id == invalid<size_type>() && (addedDr[i] = getId(races[i])).id == invalid<size_type>();
      if (isNew && _settings.maxCheckedDataRaces > _drInfos.size())
      {
         addTest = true;
         add(races[i], addedDr[i]);
      } else if (!isNew)
         update(races[i], addedDr[i]);

      if (isValid(addedDr[i].id) && addedDr[i].id > maxId)
         maxId = addedDr[i].id;
   }
   if (addTest)
      _drTests.push_back(DataRaceTest(maxId, races, replay, yields, addedDr));

   for (size_type i = 0; i < races.size(); ++i)
   {
      if (isValid(addedDr[i].id))
      {
         _drInfos[addedDr[i].id].itAttribute.updateOnRun(maxId, violationHappend, addedDr[i].order, addedDr, _settings.trackIntroducing);

      }
   }
}

size_type TestDatabase::getReplayWith(const SizeVec & ids) const
{
   actulAssert(ids.size() > 0, "DataRaceDataBase no ids passed");
   size_type res = invalid<size_type>();
   bool containsAll;
   for (size_type i = _drInfos[ids[0]].referenceTest; i < _drTests.size(); ++i)
   {
      containsAll = true;
      for (size_type j = 0; j < ids.size(); ++j)
         if (!_drTests[i].contains(ids[j]))
         {
            containsAll = false;
            break;
         }
      if (containsAll)
      {
         res = i;
         break;
      }
   }
   return res;
}

AddRaceStruct TestDatabase::getId(const DataRace & d) const
{
   AddRaceStruct res;
   res.wasFirstPermuted = false;
   res.order = 0;
   res.id = findDataRace(d);  // lookup if already in
   res.wasFirstPermuted = false;
   if (res.id != invalid<size_type>())
   {
      if (getDataRace(res.id).isSameInstanceReversed(d))  // check for reversed order
      {
         res.order = 1;
         if (!_drInfos[res.id].wasPermuted())  // check if it was the first permutation
         {
            res.wasFirstPermuted = true;
         }
      }
   }
   return res;
}

bool TestDatabase::add(const DataRace& d, AddRaceStruct & addDr)
{
   actulAssert(addDr.id == invalid<size_type>(), "TestDatabase: Cannot add data race which is already added");
   // add data race:
   addDr.id = _drInfos.size();
   _drInfos.push_back(DataRaceInfo(_drTests.size()));

   return true;
}

bool TestDatabase::update(const DataRace& d, AddRaceStruct & addDr)
{
   actulAssert(addDr.id < _drInfos.size(), "TestDatabase: Invalid data race id passed");
   bool res = false;
   if (addDr.wasFirstPermuted && !_drInfos[addDr.id].wasPermuted())
   {
      _drInfos[addDr.id].itAttribute.wasTested = true;
      _drInfos[addDr.id].setPermuted();
      _permutedDataRaces.push_back(addDr.id);  // add to the permuteable drs
      res = true;
   }
   return res;
}

size_type TestDatabase::findDataRace(const DataRace & d) const
{
   size_type res = invalid<size_type>();
   for (size_type i = 0; i < _drInfos.size(); ++i)
   {
      if (_drInfos[i].referenceTest < _drTests.size())  // hack, newly inserted data races have no reference tests yet
      {
         const DataRace & dbDr = _drTests[_drInfos[i].referenceTest].get(i);
         if (dbDr.isSameInstance(d))
         {
            res = i;
            break;
         }
      }
   }
   return res;
}

const DataRace& Actul::TestDatabase::getDataRace(const size_type& did, const size_type & replayId) const
{
   actulAssert(did <= _drInfos.size(), "DataRaceDataBase: Invalid data id passed to getDataRace (1)");
   size_type tId;
   if (replayId == invalid<size_type>())
   {
      const DataRaceInfo & info = _drInfos[did];
      tId = info.referenceTest;
      actulAssert(tId < _drTests.size(), "DataRaceDataBase: Data race id and test id are inconsistent");
   } else
   {
      tId = replayId;
      actulAssert(tId < _drTests.size(), "DataRaceDataBase: Test to data race id not found");
   }
   return _drTests[tId].get(did);
}

const ReplayTokenVec& Actul::TestDatabase::getReplay(const size_type& rid) const
{
   actulAssert(rid < _drTests.size(), "DataRaceDataBase: Invalid replay id passed");
   return _drTests[rid].replay;
}

size_type TestDatabase::numDataRaceCandidates() const
{
   return _drInfos.size();
}

size_type TestDatabase::numDataRaces() const
{
   return _permutedDataRaces.size();
}

bool TestDatabase::dataRaceIsPermuted(const size_type & drId) const
{
   return _drInfos[drId].wasPermuted();
}

bool TestDatabase::hasViolatingExecution() const
{
   return _hasViolating;
}

void TestDatabase::setTested(const DataRace & d)
{
   size_type id = findDataRace(d);
   if (isValid(id))
   {
      _drInfos[id].itAttribute.wasTested = true;
   }
}

DataRaceAttribute Actul::TestDatabase::getAttribute(const size_type& did) const
{
   actulAssert(did <= _drInfos.size(), "DataRaceDataBase: Invalid data id passed");

   const DataRaceIterativAttribute & attr = _drInfos[did].itAttribute;
   DataRaceAttribute res;
   if (attr.isPermuted())
   {
      res.falsePositive = false;
      res.harmful = attr.isSHarmful();
      res.introducing = attr.isIntroducing();
      if (res.introducing && !res.harmful)
      {
         auto drIds = attr.findIntroduced(0);
         for (size_type i = 0; i < drIds.size(); ++i)
            if (_drInfos[drIds[i]].itAttribute.isSHarmful())
            {
               res.harmful = true;
               break;
            }
      }

      res.violating = attr.isViolating();
      res.orderDependent = attr.isOrderDependent();
   } else
   {
      if (attr.wasTested)
         res.isTested = true;
      res.falsePositive = true;
   }

   return res;
}

TestDatabase::DataRaceTest::DataRaceTest()
{
}

TestDatabase::DataRaceTest::DataRaceTest(const size_type & maxId, const DataRaceShmVec& races, const ReplayTokenShmVec& rep, const YieldEventShmVec & inyields,
                                         const AddRaceStructShmVec & addedRaces)
{
   replay.resize(rep.size());
   for (size_type i = 0; i < rep.size(); ++i)
      replay[i] = rep[i];
   yields.resize(inyields.size());
   for (size_type i = 0; i < inyields.size(); ++i)
      yields[i] = inyields[i];
   dataRaces.resize(maxId + 1);
   for (size_type i = 0; i < races.size(); ++i)
   {
      if (isValid(addedRaces[i].id))
      {
         dataRaces[addedRaces[i].id] = races[i];
         dataRaces[addedRaces[i].id].setOrder(addedRaces[i].order);
      }
   }
}

bool Actul::TestDatabase::DataRaceTest::contains(const size_type& dataRaceId) const
{
   return dataRaces.size() > dataRaceId && dataRaces[dataRaceId] != invalid<DataRace>();
}

const DataRace& Actul::TestDatabase::DataRaceTest::get(const size_type& dataRaceId) const
{
   actulAssert(dataRaceId < dataRaces.size() && dataRaces[dataRaceId] != invalid<DataRace>(), "DataRaceTest: Invalid data id passed");
   return dataRaces[dataRaceId];
}

} /* namespace Actul */
