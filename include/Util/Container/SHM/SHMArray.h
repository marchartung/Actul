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
#ifndef INCLUDE_SHM_SHMARRAY_HPP_
#define INCLUDE_SHM_SHMARRAY_HPP_

#include <Util/Container/PBase/PArray.h>
#include <Util/Container/SHM/SHMObject.h>
#include <Util/Container/Vector.h>
#include <Util/EnvironmentDependencies.h>
#include <Util/DebugRoutines.h>

#include <utility>
#include <vector>
#include <cmath>

namespace Actul
{

template<typename T>
class SHMArray : public SHMObject, public PArray<T>
{

 public:

   SHMArray(const SHMArray<T> & in) = delete;

   virtual ~SHMArray()
   {

   }

   SHMArray<T> & operator=(const SHMArray<T> & in)
   {
      _mem = in._mem;
      getPtr() = in._ptr;
      return *this;
   }

   Vector<T> && getVector() const
   {
      Vector<T> res;
      res.unsafe_resize(PArray<T>::size());
      for (size_type i = 0; i < PArray<T>::size(); ++i)
         res.push_back(PArray<T>::at(i));
      return std::move(res);
   }

   void resize(const size_type & n) override
   {
      if (n > PArray<T>::capacity())
         internalReallocate(n);
      PArray<T>::resize(n);
   }

   bool insert(const T & in) override
   {
      if (!PArray<T>::insert(in))
      {
         actulAssert(PArray<T>::capacity() != 0, "SHMArray: Insert failed because was not inited");
         size_type numElems = PArray<T>::capacity() * 2u;
         internalReallocate(numElems);
         actulAssert(PArray<T>::insert(in), "Couldn't extend SHMArray.");
      }
      return true;
   }

   SHMArray<T> & operator=(const Vector<T> & in)
   {
      this->resize(in.size());
      for (size_type i = 0; i < in.size(); ++i)
         this->operator [](i) = in[i];
      return *this;
   }

   template<typename U>
   friend void Actul::deleteInstance(U*);

   template<typename U>
   friend U * Actul::newInstance();

   template<typename U, typename ...Args>
   friend U * Actul::newInstance(Args ...args);

   friend class SHMManager;

 private:

   SHMArray()
         : SHMObject(),
           PArray<T>()
   {
      initialize();
   }

   SHMArray(SHMObserver * in)
         : SHMObject(in),
           PArray<T>()
   {
      initialize();
   }

 protected:

   char* getPtr() override
   {
      return _mem.data();
   }

   const char* getPtr() const override
   {
      return _mem.data();
   }

   void initialize()
   {
      internalReallocate(5);
      PArray<T>::setCurrentSize(0);
      PArray<T>::setMaxElementCapacity(5);
   }

   void checkCompare(char * a, char * b, size_type num)
   {
      for (size_type i = 0; i < num; ++i)
         actulAssert(a[i] == b[i], "SHMArray: Compare failed");
   }

   void internalReallocate(const size_type & numElements) override
   {
      size_type numBytes = PArray<T>::getBytesForNumElems(numElements);
      if (_mem.size() != 0)
      {
         size_type tmpSize = _mem.size();
         char * tmp = reinterpret_cast<char*>(InternalMalloc(tmpSize));
         std::copy(_mem.data(), _mem.data() + tmpSize, tmp);
         checkCompare(tmp, _mem.data(), tmpSize);
         reallocate(numBytes);

         std::copy(tmp, tmp + tmpSize, _mem.data());
         checkCompare(tmp, _mem.data(), tmpSize);
         InternalFree(tmp);
      } else
         reallocate(numBytes);

      PArray<T>::setMaxElementCapacity(numElements);
   }
};
} /* namespace Actul */

#endif /* INCLUDE_SHM_SHMARRAY_HPP_ */
