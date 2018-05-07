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
#ifndef INCLUDE_ActulHEADER_MEMORYFUNCTIONS_HPP_
#define INCLUDE_ActulHEADER_MEMORYFUNCTIONS_HPP_

#include <ExternalInterface/CommonDefs.h>
#include <Logger/ActulLogger.h>
#include <Logger/ConstrainLogger.h>
#include <Scheduler/ProcessInfo.h>

EXPORT_ATTRIBUTE void Actul_func_entry(void *call_pc);
EXPORT_ATTRIBUTE void Actul_func_exit();

template<bool isWrite, bool isAtomic>
inline void Actul_MemoryAccess(const unsigned long & width, const Actul::AddressType & addr)
{
   Actul::FunctionStackImpl::getInstance()->notifyEvent<Actul::EventType::FUNCTION_SINGLE>(0, __builtin_return_address(1));
   Actul::ActulLogger::notifyAccessEvent<Actul::makeAccessType<isWrite, isAtomic>()>(width, addr);
}

#define DECL_MEMACCESS_READ(__N) EXPORT_ATTRIBUTE void Actul_read##__N(void *addr)
#define DECL_MEMACCESS_WRITE(__N) EXPORT_ATTRIBUTE void Actul_write##__N(void *addr)
#define DECL_MEMACCESS_UNALIGNED_READ(__N) EXPORT_ATTRIBUTE void Actul_unaligned_read##__N(void *addr)
#define DECL_MEMACCESS_UNALIGNED_WRITE(__N) EXPORT_ATTRIBUTE void Actul_unaligned_write##__N(void *addr)
#define DECL_MEMACCESS_READ_PC(__N) EXPORT_ATTRIBUTE void Actul_read##__N##_pc(void *addr, void *pc)
#define DECL_MEMACCESS_WRITE_PC(__N) EXPORT_ATTRIBUTE void Actul_write##__N##_pc(void *addr, void *pc)

#define DECL_MEMACCESS_ALL(__N) DECL_MEMACCESS_READ(__N); \
		                        DECL_MEMACCESS_WRITE(__N); \
								      DECL_MEMACCESS_UNALIGNED_READ(__N); \
									   DECL_MEMACCESS_UNALIGNED_WRITE(__N); \
									   DECL_MEMACCESS_READ_PC(__N); \
									   DECL_MEMACCESS_WRITE_PC(__N);

#define DECL_MEMACCESS_ATOMIC_LOAD(__N) EXPORT_ATTRIBUTE void Actul_atomic##__N##_load(void *a)
#define DECL_MEMACCESS_ATOMIC_STORE(__N) EXPORT_ATTRIBUTE void Actul_atomic##__N##_store(void *a)
#define DECL_MEMACCESS_ATOMIC_EXCHANGE(__N) EXPORT_ATTRIBUTE void Actul_atomic##__N##_exchange(const a##__N *a, a##__N v, morder mo)
#define DECL_MEMACCESS_ATOMIC_FETCH_ADD(__N) EXPORT_ATTRIBUTE void Actul_atomic##__N##_fetch_add(const a##__N *a, a##__N v, morder mo)
#define DECL_MEMACCESS_ATOMIC_FETCH_SUB(__N) EXPORT_ATTRIBUTE void Actul_atomic##__N##_fetch_sub(const a##__N *a, a##__N v, morder mo)
#define DECL_MEMACCESS_ATOMIC_AND(__N) EXPORT_ATTRIBUTE void Actul_atomic##__N##_fetch_and(const a##__N *a, a##__N v, morder mo)
#define DECL_MEMACCESS_ATOMIC_OR(__N) EXPORT_ATTRIBUTE void Actul_atomic##__N##_fetch_or(const a##__N *a, a##__N v, morder mo)
#define DECL_MEMACCESS_ATOMIC_XOR(__N) EXPORT_ATTRIBUTE void Actul_atomic##__N##_fetch_xor(const a##__N *a, a##__N v, morder mo)
#define DECL_MEMACCESS_ATOMIC_NANA(__N) EXPORT_ATTRIBUTE void Actul_atomic##__N##_fetch_nand(const a##__N *a, a##__N v, morder mo)
#define DECL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_STRONG(__N) EXPORT_ATTRIBUTE void Actul_atomic##__N##_compare_exchange_strong(a##__N *a, a##__N *c, a##__N v, morder mo, morder fmo)
#define DECL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_WEAK(__N) EXPORT_ATTRIBUTE void Actul_atomic##__N##_compare_exchange_weak(a##__N *a, a##__N *c, a##__N v, morder mo, morder fmo)
#define DECL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_VAL(__N) EXPORT_ATTRIBUTE void Actul_atomic##__N##_compare_exchange_val(a##__N *a, a##__N *c, a##__N v, morder mo, morder fmo)

#define DECL_MEMACCESS_ATOMIC_ALL(__N) DECL_MEMACCESS_ATOMIC_LOAD(__N); \
		DECL_MEMACCESS_ATOMIC_STORE(__N); \
		DECL_MEMACCESS_ATOMIC_EXCHANGE(__N); \
		DECL_MEMACCESS_ATOMIC_FETCH_ADD(__N); \
		DECL_MEMACCESS_ATOMIC_FETCH_SUB(__N); \
		DECL_MEMACCESS_ATOMIC_AND(__N); \
		DECL_MEMACCESS_ATOMIC_OR(__N); \
		DECL_MEMACCESS_ATOMIC_XOR(__N); \
		DECL_MEMACCESS_ATOMIC_NANA(__N); \
		DECL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_STRONG(__N); \
		DECL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_WEAK(__N); \
		DECL_MEMACCESS_ATOMIC_COMPARE_EXCHANGE_VAL(__N);

DECL_MEMACCESS_ALL(1)
DECL_MEMACCESS_ALL(2)
DECL_MEMACCESS_ALL(4)
DECL_MEMACCESS_ALL(8)
DECL_MEMACCESS_ALL(16)

EXPORT_ATTRIBUTE void Actul_vptr_read(void **vptr_p);

EXPORT_ATTRIBUTE void Actul_vptr_update(void **vptr_p, void *new_val);

EXPORT_ATTRIBUTE void Actul_read_range(void *addr, unsigned long size);  // NOLINT

EXPORT_ATTRIBUTE void Actul_write_range(void *addr, unsigned long size);  // NOLINT

// These should match declarations from public tsan_interface_atomic.h header.
typedef unsigned char a8;
typedef unsigned short a16;  // NOLINT
typedef unsigned int a32;
typedef unsigned long long a64;  // NOLINT

// Part of ABI, do not change.
// http://llvm.org/viewvc/llvm-project/libcxx/trunk/include/atomic?view=markup
typedef enum
{
   mo_relaxed,
   mo_consume,
   mo_acquire,
   mo_release,
   mo_acq_rel,
   mo_seq_cst
} morder;

DECL_MEMACCESS_ATOMIC_ALL(8)
DECL_MEMACCESS_ATOMIC_ALL(16)
DECL_MEMACCESS_ATOMIC_ALL(32)
DECL_MEMACCESS_ATOMIC_ALL(64)

EXPORT_ATTRIBUTE void Actul_atomic_thread_fence(morder mo);

EXPORT_ATTRIBUTE void Actul_atomic_signal_fence(morder mo);

#endif /* INCLUDE_ActulHEADER_MEMORYFUNCTIONS_HPP_ */
