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
#include <Implementation/FunctionStackImpl.h>
#include <Scheduler/ReportPrinter.h>

namespace Actul
{
template<>
FunctionStackImpl * SingletonClass<FunctionStackImpl>::_instance = nullptr;

FunctionStackImpl::FunctionStackImpl()
{
}

size_type FunctionStackImpl::getCurStackPos() const
{
   return _stack[ThreadInfo::getThreadId()].size() - 1;
}

AddressType FunctionStackImpl::getCurStackTop() const
{
   AddressType res = _stack[ThreadInfo::getThreadId()].back().addr;
   return res;
}

StackArray & FunctionStackImpl::getStack(StackArray & res, const tid_type& tid, const size_type& stackPos) const
{
   actulAssert(tid < _stack.size(), "FunctionStackLogger: No stack recorded for tid");
   actulAssert(stackPos < _stack[tid].size(), "FunctionStackLogger: Passed stackPos is invald for tid");
   const FunctionEventVec & stack = _stack[tid];
   size_type lvl = 0;
   size_type i = stackPos, j = 0;
   if (stack[i].type == 0)
      res[j++] = stack[i--].addr;
   for (; i != 0 && j < res.size(); --i)
   {
      if (lvl == 0)
      {
         if (stack[i].type == -1)
            res[j++] = stack[i].addr;
         else
            lvl += stack[i].type;
      } else
      {
         actulAssert ( stack[i].type >= 0 || lvl > 0 ,"FunctionStackImpl: Error in stack implementation");
         lvl += stack[i].type;
      }
   }
   //ReportPrinter::printStack(res);
   actulAssert(j < res.size(), "FunctionStackLogger: Size of function stack is too small");
   return res;
}

StackArray & FunctionStackImpl::getCurStack(StackArray & res) const
{
   return getStack(res, ThreadInfo::getThreadId(), getCurStackPos());
}

void FunctionStackImpl::add(const AddressType & addr, const int& type)
{
   const tid_type & tid = ThreadInfo::getThreadId();
   actulAssert(tid < _stack.size(), "FunctionStackImpl: Invalid thread id");
   _stack[tid].push_back(FunctionStackEntry(type, addr));
}

} /* namespace Actul */
