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

volatile unsigned benign;
volatile unsigned harm;

void thread0()
{
    harm = 0;
    benign = 0;
}

void * thread1(void * id)
{
    benign = 0;
    harm = 1;
    return NULL;
}

bool exampleAssert()
{
    return harm == 1;
}

int main()
{
   unsigned id = 1;
   pthread_t pid;
   bool crash = true;
   assert(pthread_create(&pid,NULL,thread1,&id) == 0);
   
   thread0();
   
   pthread_join(pid,NULL);

   assert(exampleAssert());

   std::cout << "Test ran" << std::endl;
   return 0;
}

