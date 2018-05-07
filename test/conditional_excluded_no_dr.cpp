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

pthread_cond_t c;
pthread_mutex_t m;
unsigned done = 0;

void * thread(void * id)
{
   pthread_mutex_lock(&m);
   while(pthread_cond_wait(&c,&m) != 0);
   done = 1;
   pthread_mutex_unlock(&m);
   return 0;
}

int main()
{
   unsigned id = 1;
   unsigned tmp = 0;
   pthread_t pid;

   pthread_cond_init(&c,0);
   pthread_mutex_init(&m,0);

   assert(pthread_create(&pid,NULL,thread,&id) == 0);
   while(tmp==0)
   {
      pthread_mutex_lock(&m);
      pthread_cond_signal(&c);
      tmp = done;
      pthread_mutex_unlock(&m);
   }

   pthread_join(pid, NULL);

   pthread_cond_destroy(&c);
   pthread_mutex_destroy(&m);

   assert(done == 1);

   std::cout << "Test ran" << std::endl;
   return 0;
}

