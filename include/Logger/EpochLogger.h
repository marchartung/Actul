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
#ifndef INCLUDE_LOGGER_EPOCHLOGGER_H_
#define INCLUDE_LOGGER_EPOCHLOGGER_H_

#include <Util/Defines.h>
#include <Util/SingletonClass.h>
#include <Util/Container/Vector.h>

namespace Actul
{

struct Epoch
{
   tid_type tid;
   size_type clockId;
   size_type lockSetId;

   Epoch()
   : tid(invalid<tid_type>()),
     clockId(invalid<size_type>()),
     lockSetId(invalid<size_type>())
   {

   }

   Epoch(const tid_type & tid, const size_type & clockId, const size_type & lockSetId,const size_type & numReleases)
         : tid(tid),
           clockId(clockId),
           lockSetId(lockSetId)
   {
   }

   Epoch(const Epoch & in)
   :  tid(in.tid),
      clockId(in.clockId),
      lockSetId(in.lockSetId)
   {

   }
};
typedef Vector<Epoch> EpochVec;

class EpochLogger : public SingletonClass<EpochLogger>
{
   friend class SingletonClass<EpochLogger> ;

   EpochLogger(EpochLogger&&) = delete;
   EpochLogger(const EpochLogger&) = delete;

 public:

   size_type getCurEpochId() const;

   const Epoch & getEpoch(const size_type & epochId) const;

   void updateClock(const tid_type & tid, const size_type & clockId);
   void updateLockSet(const tid_type & tid, const size_type & lockSetId);

   void curUpdateRelease();

 private:
   SizeArray _curEpochs;
   EpochVec _epochs;

   EpochLogger();
};

} /* namespace Actul */

#endif /* INCLUDE_LOGGER_EPOCHLOGGER_H_ */
