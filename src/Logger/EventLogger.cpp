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
#include <Logger/EventLogger.h>
#include <Logger/EpochLogger.h>
#include <Implementation/FunctionStackImpl.h>

namespace Actul
{
template<>
EventLogger * SingletonClass<EventLogger>::_instance = nullptr;

EventLogger::EventLogger()
      : _memoryMap(0,*EpochLogger::getInstance(),*FunctionStackImpl::getInstance()),
        _barrierMap(),
        _condMap(),
        _lockMap(),
        _semMap()
{
   for(size_type i=0;i<_threadEntries.size();++i)
   {
      _threadEntries[i].type = EventType::UNDEFINED;
      _threadEntries[i].id = invalid<event_id>();
      _threadEntries[i].counter = invalid<uint32_t>();
   }
}

}
/* namespace Actul */
