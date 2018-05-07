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

pthread_barrier_t b;
unsigned num[3];



void * thread(void * id)
{
   unsigned round = *((unsigned*)id);
   unsigned tid = round;
   pthread_barrier_wait(&b);
   num[round] = tid;
   round = (round+1) % 3;
   pthread_barrier_wait(&b);
   num[round] += tid;
   round = (round+1) % 3;
   pthread_barrier_wait(&b);
   num[round] += tid;
   round = (round+1) % 3;

   return 0;
}

int main()
{
   unsigned ids[3];
   ids[0] = 0; ids[1] = 1; ids[2] = 2;
   pthread_t pids[3];
   pthread_barrier_init(&b,0,3);

   pids[0] = 0;
   assert(pthread_create(&pids[1],NULL,thread,&ids[1]) == 0);
   assert(pthread_create(&pids[2],NULL,thread,&ids[2]) == 0);
   thread(&ids[0]);

   pthread_join(pids[1],NULL);
   pthread_join(pids[2],NULL);

   assert(num[0] == num[1] && num[1] == num[2]);

   std::cout << "Test ran" << std::endl;
   return 0;
}

