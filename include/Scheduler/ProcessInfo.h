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
#ifndef INCLUDE_PROCESSINFO_H_
#define INCLUDE_PROCESSINFO_H_

#include <Util/Defines.h>
#include <Scheduler/TestInfo.h>
#include <Scheduler/TestReport.h>

namespace Actul
{

class ProcessInfo
{
 public:

   static const char * getProgramName();

   unsigned long long getNumBytesAllocated() const;

   static TestInfo & getTestInfo();

   static bool checkProgramProperty(const bool & shouldBeTrue, const FailedProperty & assertion);

   static bool hasAssertion();

   static bool isTestReportSet();

   static TestReport & getTestReport();

   static void setTestReport(TestReport & report);

   static bool isFork();
   static void setFork(bool in);

   static const size_type & getNumReleases();
   static void increaseNumReleases();

   static double getRuntime();

   static void resetProgramStartTime();

   static TestSettings & getSettings();


 private:
   static bool _isFork;
   static String _programName;
   static int _actulAssertion;
   static size_type _numReleases;
   static TestReport * _report;
   static int64_t tStart;
   static TestSettings _settings;

};

} /* namespace Actul */

#endif /* INCLUDE_PROCESSINFO_H_ */
