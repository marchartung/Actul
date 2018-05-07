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
#include <Scheduler/ProcessInfo.h>
#include <Util/LibcAllocator.h>
#include <errno.h>
#include <Scheduler/TestScheduler.h>
#include <chrono>

namespace Actul
{

int64_t getMilliSec()
{
   return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

bool ProcessInfo::_isFork = false;
String ProcessInfo::_programName = "";
int ProcessInfo::_actulAssertion = 0;
size_type ProcessInfo::_numReleases = 0;
TestReport * ProcessInfo::_report = nullptr;
int64_t ProcessInfo::tStart = getMilliSec();
TestSettings ProcessInfo::_settings;


TestSettings & ProcessInfo::getSettings()
{
   return _settings;
}

double ProcessInfo::getRuntime()
{
   return ((double) (getMilliSec() - tStart)) / 1000.0;
}

void ProcessInfo::resetProgramStartTime()
{
   tStart = getMilliSec();
}

const size_type & ProcessInfo::getNumReleases()
{
   return _numReleases;
}
void ProcessInfo::increaseNumReleases()
{
   ++_numReleases;
}

bool ProcessInfo::isFork()
{
   return _isFork;
}

const char * ProcessInfo::getProgramName()
{
   return program_invocation_name;
}

bool ProcessInfo::checkProgramProperty(const bool & shouldBeTrue, const FailedProperty & assertion)
{
   bool res = shouldBeTrue;
   if (_report != nullptr)
   {
      if (!res)
      {
         if (static_cast<int>(assertion) >= static_cast<int>(FailedProperty::INVALID_FREE))
         {
            getTestInfo().failedProperty = assertion;
            if (_settings.verbosity > 1)
               printStr("ProcessInfo: Failed program property (" + to_string(assertion) + ")\n");
            exit(SIGABRT);
         } else
            res = true;
      }
   } else
      res = true;
   return res;
}

bool ProcessInfo::isTestReportSet()
{
   return _report != nullptr;
}

bool ProcessInfo::hasAssertion()
{
   return getTestInfo().failedProperty != FailedProperty::NO_FAILED_PROPERTY;
}

void ProcessInfo::setFork(bool in)
{
   _isFork = in;
}

TestInfo & ProcessInfo::getTestInfo()
{
   return _report->getTestInfo();
}

TestReport & ProcessInfo::getTestReport()
{
   return *_report;
}

void ProcessInfo::setTestReport(TestReport & report)
{
   _report = &report;
}

} /* namespace Actul */
