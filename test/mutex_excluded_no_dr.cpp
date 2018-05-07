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

pthread_mutex_t m;
unsigned counter = 0;

void * thread(void * id)
{
   pthread_mutex_lock(&m);
   ++counter;
   pthread_mutex_unlock(&m);
   return 0;
}

int main()
{
   pthread_t pid;
   pthread_mutex_init(&m,0);
   assert(pthread_create(&pid,NULL,thread,NULL) == 0);

   thread(NULL);

   pthread_join(pid, NULL);
   pthread_mutex_destroy(&m);

   assert(counter == 2);

   std::cout << "Test ran" << std::endl;
   return 0;
}

