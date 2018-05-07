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
#include <pthread.h>
#include <iostream>
#include <cassert>

unsigned * datarace = 0;



void * thread(void * id)
{
   *datarace = 1;
   return NULL;
}

int main()
{
   unsigned id = 1;
   pthread_t pid;
   bool crash = true;
   datarace = new unsigned();
   assert(pthread_create(&pid,NULL,thread,&id) == 0);

   if(*datarace == 0)
      crash = false;
   pthread_join(pid,NULL);

   assert(!crash);

   delete datarace;
   std::cout << "Test ran" << std::endl;
   return 0;
}

