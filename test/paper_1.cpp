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

struct Sem
{
    bool isWaiting;
    bool isPosted;
    pthread_mutex_t m;
    pthread_cond_t c;
    
    Sem() : isWaiting(false),isPosted(false) {}
    
    void post()
    {
        pthread_mutex_lock(&m);
        if(isWaiting)
            pthread_cond_signal(&c);
        isPosted = true;
        pthread_mutex_unlock(&m);
    }
    
    void wait()
    {
        pthread_mutex_lock(&m);
        if(!isPosted)
        {
            isWaiting = true;
            pthread_cond_wait(&c,&m);
        }
        
        pthread_mutex_unlock(&m);
    }
};

unsigned m,n,o,p,q;
pthread_mutex_t mtx, mtx2;
Sem s;


void init()
{
    m = 0;
    n = 0;
    o = 0;
    p = 0;
    q = 0;
    pthread_mutex_init(&mtx,0);
    pthread_mutex_init(&mtx2,0);
    pthread_mutex_init(&s.m,0);
    pthread_cond_init(&s.c,0);
}

void deinit()
{
    pthread_mutex_destroy(&mtx);
    pthread_mutex_destroy(&mtx2);
    pthread_mutex_destroy(&s.m);
    pthread_cond_destroy(&s.c);
}

void thread0()
{
    m++;
    n++;
    s.post();
    
    pthread_mutex_lock(&mtx);
    q++;
    p++;
    pthread_mutex_unlock(&mtx);
    o++;
}

void * thread1(void * id)
{
    o++;
    pthread_mutex_lock(&mtx);
    m++;
    p++;
    s.wait();
    n++;
    pthread_mutex_lock(&mtx2);
    q++;
    pthread_mutex_unlock(&mtx2);
    pthread_mutex_unlock(&mtx);
}

bool exampleAssert()
{
    return true;
}

int main()
{
   unsigned id = 1;
   pthread_t pid;
   bool crash = true;
   
   init();
   
   assert(pthread_create(&pid,NULL,thread1,&id) == 0);
   
   thread0();
   
   pthread_join(pid,NULL);

   assert(exampleAssert());
   deinit();

   std::cout << "Test ran" << std::endl;
   return 0;
}

