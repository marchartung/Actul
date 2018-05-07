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
#ifndef INCLUDE_ENVIRONMENTDEPENDENCIES_HPP_
#define INCLUDE_ENVIRONMENTDEPENDENCIES_HPP_


#ifdef Actul_LLVM_THROW
#define THROWABLE throw()
#else
#define THROWABLE
#endif

#include <pthread.h>
#include <malloc.h>
#include <cstdlib>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <wait.h>

#define InternalMalloc malloc
#define InternalFree free
#define InternalMemalign memalign

#define InternalPthread_mutex_init pthread_mutex_init
#define InternalPthread_mutex_lock pthread_mutex_lock
#define InternalPthread_mutex_trylock pthread_mutex_trylock
#define InternalPthread_mutex_unlock pthread_mutex_unlock
#define InternalPthread_mutex_destroy pthread_mutex_destroy


#define InternalPthread_create pthread_create
#define InternalPthread_join pthread_join
#define IntenalPthread_exit pthread_exit

#define InternalPthread_attr_getdetachstate pthread_attr_getdetachstate

#define InternalPthread_create pthread_create

#define InternalOpen open
#define InternalClose close
#define InternalWrite write
#define InternalRead read

#define InternalFork fork
#define InternalWaitpid waitpid
#define InternalWait wait

#define InternalExit exit
#define InternalPrintF(X) printf("%s",X)
#define RawPrint(...) printf(__VA_ARGS__)
#define InternalUsleep usleep

unsigned long long numAllocatedMB(const pid_t & pid);

#endif /* INCLUDE_ENVIRONMENTDEPENDENCIES_HPP_ */
