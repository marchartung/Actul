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
#ifndef INCLUDE_LOGGER_DATARACELOGGER_H_
#define INCLUDE_LOGGER_DATARACELOGGER_H_

#include <Logger/EpochLogger.h>
#include <Logger/EventLogger.h>
#include <Logger/HbLogger.h>
#include <Logger/LockSetLogger.h>
#include <Logger/YieldLogger.h>

namespace Actul
{

class DataRaceLogger
{

   DataRaceLogger(DataRaceLogger&&) = delete;
   DataRaceLogger(const DataRaceLogger&) = delete;

   struct DataRaceReferences
   {
      const AddressMemoryEventKey & k1;
      const AddressMemoryEventKey & k2;
      const EventInstance & ei1;
      const EventInstance & ei2;

      DataRaceReferences(const AddressMemoryEventKey & k1, const AddressMemoryEventKey & k2, const EventInstance & ei1, const EventInstance & ei2)
            : k1(k1),
              k2(k2),
              ei1(ei1),
              ei2(ei2)
      {
      }
   };

 public:

   DataRaceLogger();

   void findDataNewDataRaces(const event_id & id, const AddressType & addr);

 private:
   EpochLogger & _epLogger;
   EventLogger & _evLogger;
   FunctionStackImpl & _fsLogger;
   HbLogger & _hbLogger;
   LockSetLogger & _lsLogger;

   void findDataRaceOnKeys(const AddressType & addr, const EventInstance & newEi, const AddressMemoryEventKey & kNew, const AddressMemoryEventKey & kOld);

   void addDataRace(const DataRaceReferences & r, const AddressType & addr, const size_type & instanceNum);

};

} /* namespace Actul */

#endif /* INCLUDE_LOGGER_DATARACELOGGER_H_ */
