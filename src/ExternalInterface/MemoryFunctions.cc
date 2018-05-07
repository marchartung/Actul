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
#include <ExternalInterface/MemoryFunctions.h>
#include <Implementation/FunctionStackImpl.h>
#include <utility>

#define IMPL_MEMACCESS_READ(__N) DECL_MEMACCESS_READ(__N) { \
		                           Actul_MemoryAccess<false,false>(__N,addr);\
                               }

#define IMPL_MEMACCESS_WRITE(__N) DECL_MEMACCESS_WRITE(__N) { \
                                 Actul_MemoryAccess<true,false>(__N,addr);\
                               }

#define IMPL_MEMACCESS_UNALIGNED_READ(__N) DECL_MEMACCESS_UNALIGNED_READ(__N) { \
                                          Actul_MemoryAccess<false,false>(__N,addr);\
                                        }

#define IMPL_MEMACCESS_UNALIGNED_WRITE(__N) DECL_MEMACCESS_UNALIGNED_WRITE(__N) { \
                                             Actul_MemoryAccess<true,false>(__N,addr);\
                                           }

#define IMPL_MEMACCESS_READ_PC(__N) DECL_MEMACCESS_READ_PC(__N) { \
                                    Actul_MemoryAccess<false,false>(__N,addr);\
                                  }

#define IMPL_MEMACCESS_WRITE_PC(__N) DECL_MEMACCESS_WRITE_PC(__N) { \
                                       Actul_MemoryAccess<true,false>(__N,addr);\
                                   }

#define IMPL_MEMACCESS_ALL(__N) IMPL_MEMACCESS_READ(__N) \
                              IMPL_MEMACCESS_WRITE(__N) \
                              IMPL_MEMACCESS_UNALIGNED_READ(__N) \
                              IMPL_MEMACCESS_UNALIGNED_WRITE(__N) \
                              IMPL_MEMACCESS_READ_PC(__N) \
                              IMPL_MEMACCESS_WRITE_PC(__N)

#define IMPL_MEMACCESS_ATOMIC_LOAD(__N) \
      DECL_MEMACCESS_ATOMIC_LOAD(__N) { \
         Actul_MemoryAccess<false,true>(__N/8,reinterpret_cast<const Actul::AddressType &>(a)); }

#define IMPL_MEMACCESS_ATOMIC_STORE(__N) \
      DECL_MEMACCESS_ATOMIC_STORE(__N) { \
         Actul_MemoryAccess<true,true>(__N/8,reinterpret_cast<const Actul::AddressType &>(a)); }

#define IMPL_MEMACCESS_ATOMIC_EXCHANGE(__N) \
      DECL_MEMACCESS_ATOMIC_EXCHANGE(__N) { \
         Actul_MemoryAccess<false,true>(__N,reinterpret_cast<const Actul::AddressType &>(a)); }

#define IMPL_MEMACCESS_ATOMIC_FETCH_ADD(__N) \
      DECL_MEMACCESS_ATOMIC_FETCH_ADD(__N) { \
         Actul_MemoryAccess<false,true>(__N/8,reinterpret_cast<const Actul::AddressType &>(a)); }

#define IMPL_MEMACCESS_ATOMIC_FETCH_SUB(__N) \
      DECL_MEMACCESS_ATOMIC_FETCH_SUB(__N) { \
         Actul_MemoryAccess<false,true>(__N/8,reinterpret_cast<const Actul::AddressType &>(a)); }

#define IMPL_MEMACCESS_ATOMIC_AND(__N) \
      DECL_MEMACCESS_ATOMIC_AND(__N) { \
         Actul_MemoryAccess<false,true>(__N/8,reinterpret_cast<const Actul::AddressType &>(a)); }

#define IMPL_MEMACCESS_ATOMIC_OR(__N) \
      DECL_MEMACCESS_ATOMIC_OR(__N) { \
         Actul_MemoryAccess<false,true>(__N/8,reinterpret_cast<const Actul::AddressType &>(a)); }

#define IMPL_MEMACCESS_ATOMIC_XOR(__N) \
      DECL_MEMACCESS_ATOMIC_XOR(__N) { \
         Actul_MemoryAccess<false,true>(__N/8,reinterpret_cast<const Actul::AddressType &>(a)); }

#define IMPL_MEMACCESS_ATOMIC_NANA(__N) \
      DECL_MEMACCESS_ATOMIC_NANA(__N) { \
         Actul_MemoryAccess<false,true>(__N/8,reinterpret_cast<const Actul::AddressType &>(a)); }

#define IMPL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_STRONG(__N) \
		DECL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_STRONG(__N) { \
         Actul_MemoryAccess<false,true>(__N/8,reinterpret_cast<const Actul::AddressType &>(a)); }

#define IMPL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_WEAK(__N) \
      DECL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_WEAK(__N) { \
         Actul_MemoryAccess<false,true>(__N/8,reinterpret_cast<const Actul::AddressType &>(a)); }

#define IMPL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_VAL(__N) \
      DECL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_VAL(__N) { \
         Actul_MemoryAccess<false,true>(__N/8,reinterpret_cast<const Actul::AddressType &>(a)); }

#define IMPL_MEMACCESS_ATOMIC_ALL(__N) \
                            IMPL_MEMACCESS_ATOMIC_LOAD(__N)\
									 IMPL_MEMACCESS_ATOMIC_STORE(__N)\
									 IMPL_MEMACCESS_ATOMIC_EXCHANGE(__N)\
									 IMPL_MEMACCESS_ATOMIC_FETCH_ADD(__N)\
									 IMPL_MEMACCESS_ATOMIC_FETCH_SUB(__N)\
									 IMPL_MEMACCESS_ATOMIC_AND(__N)\
									 IMPL_MEMACCESS_ATOMIC_OR(__N)\
									 IMPL_MEMACCESS_ATOMIC_XOR(__N)\
									 IMPL_MEMACCESS_ATOMIC_NANA(__N)\
									 IMPL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_STRONG(__N)\
									 IMPL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_WEAK(__N)\
									 IMPL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_VAL(__N)

void Actul_vptr_read(void **vptr_p)
{
   Actul_MemoryAccess<false, false>(8, vptr_p);
}

void Actul_vptr_update(void **vptr_p, void *new_val)
{
   if (*vptr_p != new_val)
      Actul_MemoryAccess<true, false>(8, vptr_p);
}

void Actul_func_entry(void *call_pc)
{
   Actul::FunctionStackImpl::getInstance()->notifyEvent<Actul::EventType::FUNCTION_ENTRY>(0,call_pc);
}
void Actul_func_exit()
{
   Actul::FunctionStackImpl::getInstance()->notifyEvent<Actul::EventType::FUNCTION_EXIT>(0,0);
}

void Actul_read_range(void *addr, unsigned long size)
{
   Actul_MemoryAccess<false, false>(size, addr);
}

void Actul_write_range(void *addr, unsigned long size)
{
   Actul_MemoryAccess<true, false>(size, addr);
}

IMPL_MEMACCESS_ALL(1)
IMPL_MEMACCESS_ALL(2)
IMPL_MEMACCESS_ALL(4)
IMPL_MEMACCESS_ALL(8)
IMPL_MEMACCESS_ALL(16)

IMPL_MEMACCESS_ATOMIC_ALL(8)
IMPL_MEMACCESS_ATOMIC_ALL(16)
IMPL_MEMACCESS_ATOMIC_ALL(32)
IMPL_MEMACCESS_ATOMIC_ALL(64)
