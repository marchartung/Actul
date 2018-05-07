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
#ifndef INCLUDE_EXTERNALINTERFACE_UTILFUNCTIONS_H_
#define INCLUDE_EXTERNALINTERFACE_UTILFUNCTIONS_H_

#include <ExternalInterface/CommonDefs.h>


EXPORT_ATTRIBUTE unsigned int Actul_sleep(unsigned int seconds);

EXPORT_ATTRIBUTE int Actul_usleep(long seconds);

EXPORT_ATTRIBUTE int Actul_nanosleep(void * req, void * rem);

EXPORT_ATTRIBUTE void Actul_exit(int status);

EXPORT_ATTRIBUTE int Actul_atexit(void (*f)());

EXPORT_ATTRIBUTE int Actul___cxa_atexit(void (*f)(void *a), void *arg, void *dso);

EXPORT_ATTRIBUTE int Actul_on_exit(void(*f)(int, void*), void *arg);

EXPORT_ATTRIBUTE void * Actul_malloc(size_t size);

//EXPORT_ATTRIBUTE void * Actul_xmalloc(size_t size);

EXPORT_ATTRIBUTE void * Actul___libc_memalign(size_t align, size_t sz);

EXPORT_ATTRIBUTE void * Actul_calloc(size_t size, size_t n);

EXPORT_ATTRIBUTE void * Actul_realloc(void * p, size_t size);

EXPORT_ATTRIBUTE void Actul_free(void *p);

EXPORT_ATTRIBUTE void Actul_cfree(void *p);

EXPORT_ATTRIBUTE void * Actul_memalign(size_t align, size_t sz);

EXPORT_ATTRIBUTE void * Actul_aligned_alloc(size_t align, size_t sz);

EXPORT_ATTRIBUTE void * Actul_valloc(size_t sz);

EXPORT_ATTRIBUTE void * Actul_palloc(size_t sz);

EXPORT_ATTRIBUTE int Actul_posix_memalign(void **memptr, size_t align, size_t sz);

EXPORT_ATTRIBUTE int Actul_memcmp(const void *a1, const void *a2, unsigned long size);

EXPORT_ATTRIBUTE int Actul_strncmp(const char *a1, const char *a2, unsigned long size);

EXPORT_ATTRIBUTE void Actul_assert(int expression);
EXPORT_ATTRIBUTE void Actul___assert_fail();

#endif /* INCLUDE_EXTERNALINTERFACE_UTILFUNCTIONS_H_ */
