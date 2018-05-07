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
#include <Util/Defines.h>
#include <Util/DebugRoutines.h>
#include <ExternalInterface/MemoryFunctions.h>
#include <ExternalInterface/UtilFunctions.h>
#include <Implementation/MemoryAllocator.h>
#include <Scheduler/TestScheduler.h>

#include <unistd.h>
#include <cstdlib>

//char *getcwd(char *buf, size_t size);

void * actul_mem_allocate(const unsigned & num, const unsigned & align)
{
   unsigned sz = num;
   if (sz % align != 0)
   {
      sz = ((num / align) + 1) * align;
   }
   return Actul::MemoryAllocator::getInstance()->alignedAlloc(sz, align);
}

void actul_mem_deallocate(void * mem)
{
   Actul::MemoryAllocator::getInstance()->free(mem);
}

unsigned int Actul_sleep(unsigned int seconds)
{
   // sleep(seconds);
   return seconds;
}

int Actul_usleep(long seconds)
{
   // usleep(seconds);
   return seconds;
}

int Actul_nanosleep(void * req, void * rem)
{
   // nanosleep(req,rem);
   return 0;
}

void Actul_exit(int status)
{
   if (Actul::ThreadInfo::getThreadId() != 0)
      InternalPrintF("Warning: Exit call from created thread\n");
   else
      InternalPrintF("Warning: Exit call from Main\n");
   if (Actul::TestScheduler::isAllocated())
      Actul::TestScheduler::eraseInstance();
   exit(status);
}

int Actul_atexit(void (*f)())
{
   InternalPrintF("1Actul exit it on_exit\n");
   return 0;
}

int Actul___cxa_atexit(void (*f)(void *a), void *arg, void *dso)
{
   InternalPrintF("2Actul exit it on_exit\n");
   return 0;
}

int Actul_on_exit(void (*f)(int, void*), void *arg)
{
   InternalPrintF("3Actul exit it on_exit\n");
   return 0;
}

void * Actul_malloc(size_t size)
{
   void * res = actul_mem_allocate(size, ACTUL_SYSTEM_ALIGNMENT);
   return res;
}

/*void * Actul_xmalloc(size_t size)
 {
 void * res = actul_mem_allocate(size,ACTUL_SYSTEM_ALIGNMENT);
 if(res == NULL)
 {
 InternalPrintF("xmalloc failed to allocate");
 Actul___assert_fail();
 }
 return res;
 }*/

void * Actul_calloc(size_t size, size_t n)
{
   void *res = actul_mem_allocate(size * n, ACTUL_SYSTEM_ALIGNMENT);
   char *t = reinterpret_cast<char*>(res);
   for (Actul::size_type i = 0; i < size * n; ++i)
      t[i] = 0;

   return res;
}

void * Actul_realloc(void * p, size_t size)
{
   void * res = actul_mem_allocate(size, ACTUL_SYSTEM_ALIGNMENT);
   return res;
}

void Actul_free(void *p)
{
   if (p != NULL)
      actul_mem_deallocate(p);
}

void Actul_cfree(void *p)
{
   if (p != NULL)
      actul_mem_deallocate(p);
}

void * Actul_aligned_alloc(size_t align, size_t sz)
{
   return actul_mem_allocate(sz, align);
}

void * Actul_valloc(size_t sz)
{
   return actul_mem_allocate(sz, ACTUL_SYSTEM_ALIGNMENT);
}

void * Actul_palloc(size_t sz)
{
   return actul_mem_allocate(sz, ACTUL_SYSTEM_ALIGNMENT);
}

void* Actul__libc_memalign(size_t align, size_t sz)
{
   return actul_mem_allocate(sz, align);
}

void* Actul_memalign(size_t align, size_t sz)
{
   return actul_mem_allocate(sz, align);
}

int Actul_memcmp(const void * a1, const void * a2, unsigned long size)
{
   const char * p1 = reinterpret_cast<const char*>(a1);
   const char * p2 = reinterpret_cast<const char*>(a2);
   return Actul_strncmp(p1, p2, size);
}

int Actul_strncmp(const char * p1, const char * p2, unsigned long size)
{
   Actul::size_type i;
   int res = 0;
   for (i = 0; i < size; ++i)
   {
      Actul_MemoryAccess<false, false>(1, &p1[i]);
      Actul_MemoryAccess<false, false>(1, &p2[i]);
      if (p1[i] != p2[i])
      {
         if (p1[i] < p2[i])
         {
            res = -1;
            break;
         } else
         {
            res = 1;
            break;
         }
      }
   }

   return res;
}

void Actul_assert(int expression)
{
   if (!expression)
      Actul___assert_fail();
}

void Actul___assert_fail()
{
   if (Actul::ProcessInfo::getSettings().verbosity > 1)
      Actul::printStr("Thread " + Actul::to_string(Actul::ThreadInfo::getThreadId()) + " assert_fails\n");
   Actul::TestScheduler::eraseInstance();
   exit(SIGABRT);

}
