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
#include <Util/EnvironmentDependencies.h>
#include <malloc.h>

#include <stdio.h>
#include <stdlib.h>

unsigned long long numAllocatedMB(const pid_t & pid)
{
   char name[256];
   int value;

   sprintf(name, "/proc/%d/statm", pid);
   FILE* in = fopen(name, "rb");
   if (in == NULL)
      return 0;

   if (fscanf(in, "%d", &value) != 1)
      value = 0;
   fclose(in);
   return (value*4)/1024;
}
