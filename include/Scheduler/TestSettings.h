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
#ifndef INCLUDE_SCHEDULER_TESTSETTINGS_H_
#define INCLUDE_SCHEDULER_TESTSETTINGS_H_

#include <Util/Defines.h>

namespace Actul
{

struct TestSettings
{
   bool trackIntroducing;
   bool testOnlyPermutation;
   bool testOnlyHarmful;
   bool testOnlyRandom;
   bool useNewClassification;
   size_type debugOutput;
   size_type verbosity;
   size_type maxNumTests;
   size_type minNumTests;
   size_type maxNumWorkers;
   size_type maxMemoryMB;
   size_type seedOffset;
   size_type maxNumMillisecDelay;
   size_type maxSecondsRuntime;
   size_type maxSuccessiveReleases;
   size_type maxSequentialReads;
   size_type maxIdleLoops;
   size_type numIdlMicroSeconds;
   size_type numSystematic;
   size_type maxCheckedDataRaces;
   double printInterval;

   TestSettings();

   void readEnv();

   void print();

};

}


#endif /* INCLUDE_SCHEDULER_TESTSETTINGS_H_ */
