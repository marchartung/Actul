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
//    Marc Hartung
////////////////////////////////////////////////////////////////////////////////
#ifndef SYNCHRONIZATIONFUNCTIONS_HPP_
#define SYNCHRONIZATIONFUNCTIONS_HPP_

#include <ExternalInterface/CommonDefs.h>
#include <pthread.h>
#include <semaphore.h>

EXPORT_ATTRIBUTE int Actul_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
EXPORT_ATTRIBUTE int Actul_pthread_join(pthread_t th, void ** ret);
EXPORT_ATTRIBUTE void Actul_pthread_exit(void * retval);

EXPORT_ATTRIBUTE int Actul_pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *a);
EXPORT_ATTRIBUTE int Actul_pthread_mutex_destroy(pthread_mutex_t *m);
EXPORT_ATTRIBUTE int Actul_pthread_mutex_lock(pthread_mutex_t *m);
EXPORT_ATTRIBUTE int Actul_pthread_mutex_trylock(pthread_mutex_t *m);
EXPORT_ATTRIBUTE int Actul_pthread_mutex_unlock(pthread_mutex_t *m);

EXPORT_ATTRIBUTE int Actul_pthread_rwlock_wrlock(pthread_rwlock_t *m);
EXPORT_ATTRIBUTE int Actul_pthread_rwlock_rdlock(pthread_rwlock_t *m);
EXPORT_ATTRIBUTE int Actul_pthread_rwlock_trywrlock(pthread_rwlock_t *m);
EXPORT_ATTRIBUTE int Actul_pthread_rwlock_tryrdlock(pthread_rwlock_t *m);
EXPORT_ATTRIBUTE int Actul_pthread_rwlock_unlock(pthread_rwlock_t *m);

EXPORT_ATTRIBUTE int Actul_sem_init(sem_t * s, int pshared, unsigned val);
EXPORT_ATTRIBUTE int Actul_sem_destroy(sem_t *s);
EXPORT_ATTRIBUTE int Actul_sem_post(sem_t *s);
EXPORT_ATTRIBUTE int Actul_sem_wait(sem_t *s);

EXPORT_ATTRIBUTE int Actul_pthread_barrier_init(pthread_barrier_t * b, pthread_barrierattr_t* attr, unsigned count);
EXPORT_ATTRIBUTE int Actul_pthread_barrier_destroy(pthread_barrier_t* b);
EXPORT_ATTRIBUTE int Actul_pthread_barrier_wait(pthread_barrier_t * b);

EXPORT_ATTRIBUTE int Actul_pthread_cond_init(pthread_cond_t *c, const pthread_condattr_t *a);
EXPORT_ATTRIBUTE int Actul_pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m);
EXPORT_ATTRIBUTE int Actul_pthread_cond_signal(pthread_cond_t *c);
EXPORT_ATTRIBUTE int Actul_pthread_cond_broadcast(pthread_cond_t *c);
EXPORT_ATTRIBUTE int Actul_pthread_cond_destroy(pthread_cond_t *c);

#endif /* SYNCHRONIZATIONFUNCTIONS_HPP_ */
