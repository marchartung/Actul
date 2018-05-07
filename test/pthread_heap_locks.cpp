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

pthread_mutex_t * ms;
unsigned harmless1 = 0;
unsigned harmless2 = 0;


void * thread(void * id)
{
   pthread_mutex_lock(&ms[0]);
   ++harmless1;
   pthread_mutex_unlock(&ms[0]);
   pthread_mutex_lock(&ms[1]);
   ++harmless2;
   pthread_mutex_unlock(&ms[1]);
   return NULL;
}

int main()
{
   pthread_t pid;
   ms = new pthread_mutex_t[2];
   pthread_mutex_init(&ms[0],0);
   pthread_mutex_init(&ms[1],0);
   assert(pthread_create(&pid,NULL,thread,NULL) == 0);

   thread(NULL);

   pthread_join(pid,NULL);

   assert(harmless1 == 2 && harmless1 == harmless2);

   pthread_mutex_destroy(&ms[0]);
   pthread_mutex_destroy(&ms[1]);
   delete[] ms;
   std::cout << "Test ran" << std::endl;
   return 0;
}

