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
#include <Logger/Event.h>
#include <ExternalInterface/PthreadLib.h>
#include <Implementation/PthreadImpl.h>
#include <Scheduler/ThreadInfo.h>
#include <Util/Utility.h>



int Actul_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
   typedef std::tuple<pthread_t*,const pthread_attr_t*,Actul::ExecuteFunction, void*> Args;
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::THREAD_CREATE,int,Args>(thread,Args(thread,attr,start_routine,arg));
}

int Actul_pthread_join(pthread_t th, void ** ret)
{
   typedef std::tuple<pthread_t,void **> Args;
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::THREAD_JOIN,int,Args>(nullptr,Args(th,ret));
}

void Actul_pthread_exit(void * retval)
{
   Actul::PthreadImpl::getInstance()->act<Actul::EventType::THREAD_EXIT,void,void*>(retval, nullptr);
}

int Actul_pthread_barrier_init(pthread_barrier_t * b, pthread_barrierattr_t* attr, unsigned count)
{
   return Actul::PthreadImpl::getInstance()->initialize<Actul::EventType::BARRIER_INIT>(b,count);
}

int Actul_pthread_barrier_destroy(pthread_barrier_t* b)
{
   return Actul::PthreadImpl::getInstance()->deinitialize<Actul::EventType::BARRIER_DESTROY>(b);
}

int Actul_pthread_cond_init(pthread_cond_t* c, const pthread_condattr_t * a)
{
   return Actul::PthreadImpl::getInstance()->initialize<Actul::EventType::COND_INIT>(c,0);
}
int Actul_pthread_cond_destroy(pthread_cond_t* c)
{
   return Actul::PthreadImpl::getInstance()->deinitialize<Actul::EventType::COND_DESTROY>(c);
}

int Actul_pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *a)
{
   return Actul::PthreadImpl::getInstance()->initialize<Actul::EventType::MUTEXLOCK_INIT>(m,0);
}

int Actul_pthread_mutex_destroy(pthread_mutex_t *m)
{
   return Actul::PthreadImpl::getInstance()->deinitialize<Actul::EventType::MUTEXLOCK_DESTROY>(m);
}

int Actul_pthread_rwlock_init(pthread_rwlock_t *m, const pthread_rwlockattr_t *a)
{
   return Actul::PthreadImpl::getInstance()->initialize<Actul::EventType::RWLOCK_INIT>(m,0);
}

int Actul_pthread_rwlock_destroy(pthread_rwlock_t *m, const pthread_rwlockattr_t *a)
{
   return Actul::PthreadImpl::getInstance()->initialize<Actul::EventType::RWLOCK_DESTROY>(m,0);
}

int Actul_sem_init(sem_t * s, int pshared, unsigned val)
{
   return Actul::PthreadImpl::getInstance()->initialize<Actul::EventType::SEM_INIT>(s,val);
}

int Actul_sem_destroy(sem_t *s)
{
   return Actul::PthreadImpl::getInstance()->deinitialize<Actul::EventType::SEM_DESTROY>(s);
}


int Actul_pthread_barrier_wait(pthread_barrier_t * b)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::BARRIER_WAIT,int,void*>(b,nullptr);
}

int Actul_pthread_cond_wait(pthread_cond_t* c, pthread_mutex_t* m)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::COND_WAIT,int,pthread_mutex_t*>(c,m);
}

int pthread_cond_timedwait(pthread_cond_t * c, pthread_mutex_t * m, const struct timespec * abstime)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::COND_TIMEDWAIT,int,pthread_mutex_t*>(c,m);
}

int Actul_pthread_cond_signal(pthread_cond_t* c)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::COND_SIGNAL,int,void*>(c, nullptr);
}

int Actul_pthread_cond_broadcast(pthread_cond_t* c)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::COND_BCAST,int,void*>(c, nullptr);
}

int Actul_pthread_mutex_lock(pthread_mutex_t *m)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::MUTEXLOCK,int,void*>(m, nullptr);
}

int Actul_pthread_mutex_trylock(pthread_mutex_t *m)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::MUTEXTRYLOCK,int,void*>(m, nullptr);
}

int Actul_pthread_mutex_unlock(pthread_mutex_t *m)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::MUTEXUNLOCK,int,void*>(m, nullptr);
}

int Actul_pthread_rwlock_wrlock(pthread_rwlock_t *m)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::RWLOCK_WRITE,int,void*>(m, nullptr);
}

int Actul_pthread_rwlock_rdlock(pthread_rwlock_t *m)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::RWLOCK_READ,int,void*>(m, nullptr);
}

int Actul_pthread_rwlock_trywrlock(pthread_rwlock_t *m)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::RWTRYLOCK_WRITE,int,void*>(m, nullptr);
}

int Actul_pthread_rwlock_tryrdlock(pthread_rwlock_t *m)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::RWTRYLOCK_READ,int,void*>(m, nullptr);
}

int Actul_pthread_rwlock_unlock(pthread_rwlock_t *m)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::RWUNLOCK,int,void*>(m, nullptr);
}

int Actul_sem_post(sem_t *s)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::SEM_POST,int,void*>(s, nullptr);
}

int Actul_sem_wait(sem_t *s)
{
   return Actul::PthreadImpl::getInstance()->act<Actul::EventType::SEM_WAIT,int,void*>(s, nullptr);
}

