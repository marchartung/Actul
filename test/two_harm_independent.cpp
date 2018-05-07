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


unsigned harmful1, harmful2;

void thread1()
{
   harmful1 = 0;
   harmful2 = 0;
}

void * thread2(void * id)
{
   harmful1 = 1;
   harmful2 = 1;
   return NULL;
}

int main()
{
   pthread_t pid;
   bool crash = true;
   assert(pthread_create(&pid,NULL,thread2,NULL) == 0);
   thread1();
   pthread_join(pid,NULL);

   assert(harmful1==1 && harmful2 == 1);

   std::cout << "Test ran" << std::endl;
   return 0;
}

