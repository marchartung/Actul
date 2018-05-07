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
#include <Scheduler/TestSettings.h>
#include <Util/String.h>

namespace Actul
{

TestSettings::TestSettings()
      : trackIntroducing(false),
        testOnlyPermutation(false),
        testOnlyHarmful(false),
        testOnlyRandom(false),
        useNewClassification(true),
        debugOutput(1),
        verbosity(1),
        maxNumTests(std::numeric_limits<size_type>::max()),
        minNumTests(1),
        maxNumWorkers(1),
        maxMemoryMB(1024 * 32),
        seedOffset(1),
        maxNumMillisecDelay(1000),
        maxSecondsRuntime(24*3600),
        maxSuccessiveReleases(20),
        maxSequentialReads(500),
        maxIdleLoops(100000),
        numIdlMicroSeconds(2000),
        numSystematic(2),
        maxCheckedDataRaces(10000),
        printInterval(5.0)
{
   readEnv();
   if (verbosity > 1)
      print();
}

void TestSettings::print()
{
   printStr("Actul start settings:\n");
   printStr("ACTUL_TRACK_INTRODUCING=" + to_string(trackIntroducing) + "\n");
   printStr("ACTUL_ONLY_PERMUTATION=" + to_string(testOnlyPermutation) + "\n");
   printStr("ACTUL_ONLY_HARMFUL=" + to_string(testOnlyHarmful) + "\n");
   printStr("ACTUL_ONLY_RANDOM=" + to_string(testOnlyRandom) + "\n");
   printStr("ACTUL_NEW_CLASSIFICATION=" + to_string(useNewClassification) + "\n");
   printStr("ACTUL_DEBUG_OUTPUT=" + to_string(debugOutput) + "\n");
   printStr("ACTUL_VERBOSITY=" + to_string(verbosity) + "\n");
   printStr("ACTUL_MAX_NUM_TESTS=" + to_string(maxNumTests) + "\n");
   printStr("ACTUL_MIN_NUM_TESTS=" + to_string(minNumTests) + "\n");
   printStr("ACTUL_NUM_WORKERS=" + to_string(maxNumWorkers) + "\n");
   printStr("ACTUL_MAX_MEM_MB=" + to_string(maxMemoryMB) + "\n");
   printStr("ACTUL_SEED=" + to_string(seedOffset) + "\n");
   printStr("ACTUL_MAX_TIME=" + to_string(maxSecondsRuntime) + "\n");
   printStr("ACTUL_SUCCESSIVE_RELEASES=" + to_string(maxSuccessiveReleases) + "\n");
   printStr("ACTUL_MAX_SEQUENTIAL_READS=" + to_string(maxSequentialReads) + "\n");
   printStr("ACTUL_MAX_IDLE_LOOPS=" + to_string(maxIdleLoops) + "\n");
   printStr("ACTUL_IDLE_MICROSECONDS=" + to_string(numIdlMicroSeconds) + "\n");
   printStr("ACTUL_NUM_SYSTEMATIC=" + to_string(numSystematic) + "\n");
   printStr("ACTUL_MAX_CHECKED_DATARACES=" + to_string(maxCheckedDataRaces) + "\n");
   printStr("ACTUL_PRINT_INTERVAL=" + to_string(printInterval) + "\n");
}

template<typename T>
void getEnv(T & res, const String & var)
{
   if (std::getenv(var.c_str()) != nullptr)
   {
      Stringstream ss(std::getenv(var.c_str()));
      ss >> res;
   }
}

void TestSettings::readEnv()
{

   getEnv(trackIntroducing, "ACTUL_TRACK_INTRODUCING");
   getEnv(testOnlyPermutation, "ACTUL_ONLY_PERMUTATION");
   getEnv(testOnlyHarmful, "ACTUL_ONLY_HARMFUL");
   getEnv(testOnlyRandom, "ACTUL_ONLY_RANDOM");
   getEnv(useNewClassification, "ACTUL_NEW_CLASSIFICATION");
   getEnv(debugOutput, "ACTUL_DEBUG_OUTPUT");
   getEnv(verbosity, "ACTUL_VERBOSITY");
   getEnv(maxNumTests, "ACTUL_MAX_NUM_TESTS");
   getEnv(minNumTests, "ACTUL_MIN_NUM_TESTS");
   getEnv(maxNumWorkers, "ACTUL_NUM_WORKERS");
   getEnv(maxMemoryMB, "ACTUL_MAX_MEM_MB");
   getEnv(seedOffset, "ACTUL_SEED");
   getEnv(maxSecondsRuntime, "ACTUL_MAX_TIME");
   getEnv(maxSuccessiveReleases, "ACTUL_SUCCESSIVE_RELEASES");
   getEnv(maxSequentialReads, "ACTUL_MAX_SEQUENTIAL_READS");
   getEnv(maxIdleLoops, "ACTUL_MAX_IDLE_LOOPS");
   getEnv(numIdlMicroSeconds, "ACTUL_IDLE_MICROSECONDS");
   getEnv(numSystematic, "ACTUL_NUM_SYSTEMATIC");
   getEnv(maxCheckedDataRaces, "ACTUL_MAX_CHECKED_DATARACES");
   getEnv(printInterval, "ACTUL_PRINT_INTERVAL");

   // check testing modes:
   if(testOnlyRandom && testOnlyHarmful)
   {
      testOnlyHarmful = false;
      testOnlyPermutation = false;
      printStr("Warning: Overwriting variable ACTUL_ONLY_HARMFUL and ACTUL_ONLY_PERMUTATION to 0, because ACTUL_ONLY_RANDOM is set");
   }
   if(testOnlyPermutation && testOnlyHarmful)
   {
      testOnlyHarmful = false;
      printStr("Warning: Overwriting variable ACTUL_ONLY_HARMFUL to 0, because ACTUL_ONLY_PERMUTATION is set");
   }
   // check classification
   if(useNewClassification && trackIntroducing)
   {
      printStr("Warning: Overwriting variable ACTUL_TRACK_INTRODUCING to 0, because ACTUL_NEW_CLASSIFICATION is set");
      trackIntroducing = false;
   }
}

}

