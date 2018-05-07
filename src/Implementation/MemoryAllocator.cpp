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
#include <Logger/ActulLogger.h>
#include <Scheduler/ProcessInfo.h>

#include <utility>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <Implementation/MemoryAllocator.h>

namespace Actul
{
template<>
MemoryAllocator * SingletonClass<MemoryAllocator>::_instance = nullptr;

MemoryAllocator::MemoryAllocator()
{

}

bool MemoryAllocator::isValidAlignment(const void * ptr, const unsigned & align)
{
   uintptr_t addr = (uintptr_t)ptr;
   return addr % align == 0;
}

void * MemoryAllocator::alignedAlloc(const unsigned & num, const unsigned & align)
{
   void * res = NULL;
   if (ProcessInfo::checkProgramProperty(num % align == 0 && num / align >= 1, FailedProperty::INVALID_ALLOCATION))
   {
      unsigned int allocSize = num;
      void * mem = allocate_mmap(allocSize);
      res = mem;

      if (mem != NULL && !isValidAlignment(res, align))
      {
         // alignment not created, retry with space to rearrange pointer for alignment:
         allocSize = num + align;
         deallocate_mmap(num, mem);
         mem = allocate_mmap(allocSize);
         if (mem != NULL)
            res = getAlignment(mem, align);
      }
      if (mem != NULL)
      {
         const event_id & id = ActulLogger::notifySyncEventPre<EventType::HEAP_ALLOCATION>(res);
         if (id >= _heapAllocs.size())
            _heapAllocs.resize(id + 1);
         HeapLog & spec = _heapAllocs[id];
         spec.num = allocSize;
         spec.ptr = mem;

         ActulLogger::callScheduler<EventType::HEAP_ALLOCATION>(false, id);
         ActulLogger::notifySyncEventPost<EventType::HEAP_ALLOCATION>(res, id);
      }
      else
         actulAssert(false, "MemoryAllocator: could not allocate memory.");
   }
   //printStr("ALlocated mem: num:" + to_string(num) + " align:" + to_string(align) + "\n");
   return res;
}
void MemoryAllocator::free(void * ptr)
{
   if (ProcessInfo::checkProgramProperty(ptr != NULL, FailedProperty::INVALID_FREE))
   {
      event_id id = ActulLogger::notifySyncEventPre<EventType::HEAP_FREE>(ptr);
      HeapLog & spec = _heapAllocs[id];
      if (ProcessInfo::checkProgramProperty(spec.isAllocated(), FailedProperty::INVALID_FREE))
         deallocate_mmap(spec.num, spec.ptr);
      spec.num = 0;
      spec.ptr = nullptr;
      ActulLogger::notifySyncEventPost<EventType::HEAP_FREE>(ptr,id);
   }
}

void * MemoryAllocator::allocate_mmap(const unsigned & num)
{
   static unsigned id = 0;
   void * res = mmap(&id, num, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if (res == MAP_FAILED)
      if (ProcessInfo::checkProgramProperty(errno != ENOMEM, FailedProperty::NO_MEMORY))
         res = NULL;
   return res;
}

void MemoryAllocator::deallocate_mmap(const unsigned & num, void * ptr)
{
   munmap(ptr, num);
   ptr = NULL;
}

void * MemoryAllocator::getAlignment(const void * ptr, const unsigned & align)
{
   uintptr_t addr = (uintptr_t) ptr;
   uintptr_t r = addr % align;
   addr += align - r;
   return (void *) addr;
}

}

