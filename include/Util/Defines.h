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
#ifndef INCLUDE_DEFINES_HPP_
#define INCLUDE_DEFINES_HPP_

#include <ExternalInterface/CommonDefs.h>
#include <Util/LibcAllocator.h>
#include <Util/DebugRoutines.h>
#include <Util/TypeDefs.h>
#include <Util/Container/Array.h>
#include <Util/Container/Vector.h>
#include <Util/Invalid.h>

#include <new>
#include <list>

namespace Actul
{

#define ACTUL_MAX_NUM_THREADS 32
#define ACTUL_SYSTEM_ALIGNMENT 4
#define MAX_FUNCTION_STACK_SIZE 30
#define PRINT_WRONGLY_USED

typedef Vector<int> IntVec;
typedef Vector<pid_t> PidVec;
typedef Vector<tid_type> TidVec;
typedef Vector<bool> BoolVec;
typedef Vector<BoolVec> BoolVecVec;
typedef Vector<size_type> SizeVec;
typedef Vector<uint32_t> Uint32Vec;
typedef Vector<SizeVec> SizeVecVec;

// TODO get rid of lists
typedef std::list<tid_type,LibcAllocator<tid_type>> TidList;
typedef std::list<size_type,LibcAllocator<size_type>> SizeList;
typedef std::list<int,LibcAllocator<int>> IntList;

typedef Array<size_type, ACTUL_MAX_NUM_THREADS> SizeArray;
typedef Array<SizeVec, ACTUL_MAX_NUM_THREADS> SizeVecArray;
typedef Array<tid_type, ACTUL_MAX_NUM_THREADS> TidArray;
typedef Array<IntList::iterator,ACTUL_MAX_NUM_THREADS>    IntListIteratorArray;
typedef Array<AddressType, MAX_FUNCTION_STACK_SIZE> StackArray;
typedef Array<bool, ACTUL_MAX_NUM_THREADS> BoolArray;

typedef Array<bool,2> Bool2Array;
typedef Array<uint8_t,2> Uint8_t2Array;
typedef Array<tid_type,2> Tid2Array;
typedef Array<uint32_t,2> Uint32_t2Array;
typedef Array<AddressType,2> VoidPtr2Array;
typedef Array<StackArray,2> StackArray2Array;




template<typename T, typename ...Args>
T * newInstance(Args ... args)
{
   LibcAllocator<T> allocator;
   T * res = allocator.allocate(1);
   res = new (res) T(args...);
   return res;
}

template<typename T>
T * newInstance()
{
   LibcAllocator<T> allocator;
   T * res = allocator.allocate(1);
   res = new (res) T();
   return res;
}

template<typename T>
void deleteInstance(T* in)
{
   LibcAllocator<T> allocator;
   in->~T();
   allocator.deallocate(in, sizeof(T));
}



} /* namespace Actul */
#endif /* INCLUDE_DEFINES_HPP_ */
