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
#ifndef INCLUDE_LOGGER_FUNCTIONSTACKLOGGER_H_
#define INCLUDE_LOGGER_FUNCTIONSTACKLOGGER_H_

#include <Util/Defines.h>
#include <Util/SingletonClass.h>
#include <Util/TypeDefs.h>
#include <Scheduler/ThreadInfo.h>
#include <Logger/Event.h>

#include <Util/Container/Vector.h>

namespace Actul
{

class FunctionStackImpl : public SingletonClass<FunctionStackImpl>
{
   friend class SingletonClass<FunctionStackImpl> ;

   FunctionStackImpl(FunctionStackImpl&&) = delete;
   FunctionStackImpl(const FunctionStackImpl &) = delete;

   enum FunctionCallType
   {
      FUNCTIONCALLTYPE_ENTRY,
      FUNCTIONCALLTYPE_EXIT,
      FUNCTIONCALLTYPE_SINGLEDEPTH
   };
   struct FunctionStackEntry
   {
      int type;
      AddressType addr;
      FunctionStackEntry()
            : type(invalid<int>()),
              addr(nullptr)
      {
      }
      FunctionStackEntry(const int & type, const AddressType & addr)
            : type(type),
              addr(addr)
      {
      }
   };
   typedef Vector<FunctionStackEntry> FunctionEventVec;
   typedef Array<FunctionEventVec, ACTUL_MAX_NUM_THREADS> FunctionEventVecArray;
 public:

   template<EventType ET>
   void notifyEvent(const unsigned long & width, const AddressType & addr)
   {
      static_assert(isFunctionEvent<ET>(), "FunctionStackLogger: Cannot call notifyEvent with non-function event");

      if (ET == EventType::FUNCTION_ENTRY)
         add(addr, -1);
      else if (ET == EventType::FUNCTION_EXIT)
         add(addr, 1);
      else
         add(addr, 0);
   }

   size_type getCurStackPos() const;

   AddressType getCurStackTop() const;

   StackArray & getStack(StackArray & res, const tid_type & tid, const size_type & stackPos) const;

   StackArray & getCurStack(StackArray & res) const;

 private:
   FunctionEventVecArray _stack;

   FunctionStackImpl();

   void add(const AddressType & addr, const int & type);

};

} /* namespace Actul */

#endif /* INCLUDE_LOGGER_FUNCTIONSTACKLOGGER_H_ */
