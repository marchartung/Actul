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
#include <Scheduler/PermutationTests.h>
#include <Scheduler/ProcessInfo.h>

namespace Actul
{

PermutationTests::PermutationTests(const TestDatabase& tdb, const TestSettings & settings)
      : _tdb(tdb),
        _settings(settings),
        _curDr(1, 0),
        _orders(1, true)
{
}

bool PermutationTests::hasTest() const
{
   actulAssert(_curDr.size() == 1, "PermutationTests: invalid hasTest() call.");
   bool res = _tdb.numDataRaceCandidates() > _curDr[0];
   return res;
}

const SizeVec& PermutationTests::getCurTestRaceIds() const
{
   actulAssert(hasTest(), "PermutationTests: Cannot return test races, when there is no test.");
   if (ProcessInfo::getSettings().verbosity > 1)
      printStr("Testing " + to_string(_curDr[0]) + "\n");
   return _curDr;
}

const BoolVec& PermutationTests::getCurTestRaceOrders() const
{
   actulAssert(hasTest(), "PermutationTests: Cannot return test orders, when there is no test.");
   return _orders;
}

void PermutationTests::next()
{
   actulAssert(_curDr.size() == 1, "PermutationTests: invalid next() call.");
   if(!_settings.testOnlyPermutation)
      while(++_curDr[0] < _tdb.numDataRaceCandidates() && _tdb.dataRaceIsPermuted(_curDr[0]));
   else
      ++_curDr[0];
}

size_type PermutationTests::numTests() const
{
   return _tdb.numDataRaceCandidates();
}

} /* namespace Actul */
