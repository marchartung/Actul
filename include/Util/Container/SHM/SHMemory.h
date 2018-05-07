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
#ifndef SHM_SHMEMORY_HPP_
#define SHM_SHMEMORY_HPP_

#include <Util/Defines.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include <fstream>


namespace Actul {

class SHMObject;

struct SHMemory {

 public:
   friend class SHMObject;


   size_type size() const;

   char * data();

   const char * data() const;


 private:

   const int _id;
   const String _keyPhrase;
   const int _mode;
   const key_t _key;

   int _shmId;
   size_type _size;
   char * _data;

   static int _counter;


	SHMemory();

	~SHMemory();

   key_t createKey() const;

   int allocate(const size_type & numBytes = minSize());

   char * attach();

   void detach();

   void deallocate();

	void cleanUpLastRun(const key_t & in) const;

   static size_type minSize();
};
}
#endif /* SHM_SHMMEMORY_HPP_ */
