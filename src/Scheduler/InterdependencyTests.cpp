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
#include <Scheduler/InterdependencyTests.h>
#include <Util/Utility.h>

/*
 * this class has two states
 *    - validated
 *       - current setting is available and can be used for testing
 *    - not validated
 *       - the current setting is old and needs to be next()
 */

namespace Actul
{
InterdependencyTests::InterdependencyTests(const TestDatabase & tdb, const TestSettings & settings)
      : _settings(settings),
        _tdb(tdb),
        _currentlyNotValidated(true),
        _cycleFinder(tdb),
        _curPermutation(settings.numSystematic, false),
        _curTestRaces(settings.numSystematic, 0)
{
   //actulAssert(settings.numSystematic == 2, "TestState: only two data races allowed in interstage");

   for (size_type i = 0; i < _curTestRaces.size(); ++i)
   {
      _curTestRaces[i] = _curTestRaces.size() - i - 1;
      _curPermutation[i] = false;
   }

}

bool InterdependencyTests::isValidOrder() const
{
   SizeVec drIds = getCurTestRaceIds();
   size_type rid = _tdb.getReplayWith(drIds);
   return rid != invalid<size_type>() && !_cycleFinder.hasCylces(_curTestRaces, _curPermutation);
}

void InterdependencyTests::validate()
{
   if (hasTest_unsafe())
   {
      if (!isValidOrder())
         next();
   }
   _currentlyNotValidated = false;
}

bool InterdependencyTests::hasTest_unsafe() const
{
   return _curTestRaces[0] < _tdb.numDataRaces();
}

bool InterdependencyTests::hasTest()
{
   actulAssert(_curTestRaces.size() > 0, "InterdependencyTests: Cannot call hasTest. Class is not initialized");
   bool res = false;
   if (hasTest_unsafe())
   {
      if (_currentlyNotValidated)
         validate();
      res = !_currentlyNotValidated && hasTest_unsafe();
   }
   return res && !_settings.testOnlyPermutation;
}

SizeVec InterdependencyTests::getCurTestRaceIds() const
{
   actulAssert(hasTest_unsafe(), "InterdependencyTests: Cannot set get test, when the current is not valid");
   SizeVec res(_curTestRaces.size());
   for (size_type i = 0; i < _curTestRaces.size(); ++i)
   {
      if (i > 0)
         actulAssert(_curTestRaces[i - 1] > _curTestRaces[i], "InterdependencyTests: returning invalid test case");
      res[i] = _tdb.getPermutableDataRaceId(_curTestRaces[i]);
   }
   /*printStr("Testing follow ids:\n");
    for (size_type i = 0; i < res.size(); ++i)
    {
    printStr(to_string(res[i]) + " order: " + ((_curPermutation[i]) ? "1\n" : "0\n"));
    }*/
   return res;
}
const BoolVec & InterdependencyTests::getCurTestRaceOrders() const
{
   actulAssert(hasTest_unsafe(), "InterdependencyTests: Cannot set get test, when the current is not valid");
   return _curPermutation;
}

bool InterdependencyTests::next()
{
   actulAssert(hasTest_unsafe(), "InterdependencyTests: Cannot set next test, when the current is not valid");
   bool ticked = false;
   while (!ticked)
   {
      ticked = tryPermuteOrder();

      if (!ticked)
      {
         ticked = trySetNextRaces();
         if (ticked)
            ticked = isValidOrder();
         else
         {
            _currentlyNotValidated = true;
            break;
         }
      } else
         ticked = isValidOrder();
   }
   return ticked;
}

size_type InterdependencyTests::numTests() const
{
   //only valid vor nT = 2, GaussSum is not the correct formula for n>2
   const size_type nD = _tdb.numDataRaces(), &nT = _settings.numSystematic;

   return Pow(2, nT) * (BinCoeff(nD, nT));
}

bool InterdependencyTests::tryPermuteOrder()
{
   bool res = false;
   size_type i = _curPermutation.size();
   for (; i > 0; --i)
      if (!_curPermutation[i - 1])
      {
         _curPermutation[i - 1] = true;
         res = true;
         break;
      }
   for (size_type j = i; j < _curPermutation.size(); ++j)
      _curPermutation[j] = false;
   return res;
}

bool InterdependencyTests::isSenseLess() const
{
   bool containsHarmful = false;
   if (_tdb.hasViolatingExecution())
   {
      for (size_type i = 0; i < _curTestRaces.size(); ++i)
      {
         if (_tdb.getAttribute(_tdb.getPermutableDataRaceId(_curTestRaces[i])).getScore() > 2)
         {
            containsHarmful = true;
            break;
         }
      }
   }
   else
      containsHarmful = true;

   return !containsHarmful;
}

bool InterdependencyTests::trySetNextRaces()
{
   bool res = false;
   size_type foundRace;
   size_type lastRace, i;
   do
   {
      i = _curTestRaces.size();
      foundRace = invalid<size_type>();
      lastRace = 0;
      res = false;
      for (; i > 1; --i)
         if ((_curTestRaces[i - 1] > 0 && _curTestRaces[i - 1] - 1 > lastRace))
         {
            --_curTestRaces[i - 1];
            foundRace = _curTestRaces[i - 1];
            break;
         } else
            lastRace = _curTestRaces[i - 1];
      if (foundRace == invalid<size_type>())
      {
         {
            ++_curTestRaces[0];
            if (_tdb.numDataRaces() > _curTestRaces[0])

               foundRace = _curTestRaces[0];
            i = 1;

         }
      }
      if (foundRace != invalid<size_type>())
      {
         res = true;
         for (size_type j = 0; j < _curTestRaces.size() - i; ++j)
            _curTestRaces[i + j] = foundRace - j - 1;

         for (size_type j = 0; j < _curPermutation.size(); ++j)
            _curPermutation[j] = false;

      }
   } while (_settings.testOnlyHarmful && hasTest_unsafe() && isSenseLess());
   return res;
}

} /* namespace Actul */
