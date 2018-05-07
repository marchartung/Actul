/*
==============================================================================
LLVM Release License
==============================================================================
University of Illinois/NCSA
Open Source License

Copyright (c) 2003-2017 University of Illinois at Urbana-Champaign.
All rights reserved.

Developed by:

    LLVM Team

    University of Illinois at Urbana-Champaign

    http://llvm.org

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal with
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimers.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimers in the
      documentation and/or other materials provided with the distribution.

    * Neither the names of the LLVM Team, University of Illinois at
      Urbana-Champaign, nor the names of its contributors may be used to
      endorse or promote products derived from this Software without specific
      prior written permission.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
SOFTWARE.
*/

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
#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Analysis/CaptureTracking.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Module.h>
#include <llvm/ProfileData/InstrProf.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>

#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/IR/LegacyPassManager.h>


using namespace llvm;

#define DEBUG_TYPE "actul"

#ifdef USE_LLVM_OLD
#define PASS_NAME_RETURN const char*
#else
#define PASS_NAME_RETURN StringRef
#endif

static const char * const kActulModuleCtorName = "actul.module_ctor";
static const char * const kActulInitName = "Actul_Init";
static const char * const kActulModuleDtorName = "actul.module_ctor";
static const char * const kActulDestrName = "Actul_Destruct";

static const char * const toReplaceFunctions[] = { "pthread_create", "pthread_join", "pthread_detach", "pthread_mutex_lock", "pthread_mutex_unlock", "pthread_mutex_init",
      "pthread_mutex_destroy", "pthread_mutex_trylock", "pthread_barrier_init", "pthread_barrier_destroy","pthread_rwlock_wrlock","pthread_rwlock_rdlock","pthread_rwlock_trywrlock",
      "pthread_rwlock_tryrdlock","pthread_rwlock_unlock","pthread_exit",
      "pthread_barrier_wait", "pthread_cond_init", "pthread_cond_wait", "pthread_cond_signal", "pthread_cond_broadcast", "pthread_cond_destroy", "sleep", "usleep", "nanosleep",
        "memcmp", "strncmp", "assert",
      "__assert_fail","atomic_flag_test_and_set","atomic_flag_clear", "exit" };

        // "malloc","__libc_memalign", "calloc", "realloc", "xmalloc", "free", "cfree", "memalign", "aligned_alloc", "valloc", "palloc", "posix_memalign",

static const char * shouldBeSupported[] = {"sem_init", "sem_destroy", "sem_post", "sem_wait"};
static const long int numReplaceFunctions = sizeof(toReplaceFunctions) / sizeof(const char *);
static const long int numShouldFunctions = sizeof(shouldBeSupported) / sizeof(const char *);

namespace
{

/// ActulTester: instrument the code in module to find races.
struct ActulTester : public FunctionPass
{
   ActulTester()
         : FunctionPass(ID)
   {
   }
   PASS_NAME_RETURN getPassName() const override;
   void getAnalysisUsage(AnalysisUsage &AU) const override;
   bool runOnFunction(Function &F) override;
   bool doInitialization(Module &M) override;
   static char ID;  // Pass identification, replacement for typeid.

 private:
   void initializeCallbacks(Module &M);
   bool instrumentLoadOrStore(Instruction *I, const DataLayout &DL);
   bool instrumentAtomic(Instruction *I, const DataLayout &DL);
   bool instrumentMemIntrinsic(Instruction *I);
   void chooseInstructionsToInstrument(SmallVectorImpl<Instruction *> &Local, SmallVectorImpl<Instruction *> &All, const DataLayout &DL);
   bool addrPointsToConstantData(Value *Addr);
   int getMemoryAccessFuncIndex(Value *Addr, const DataLayout &DL);


   Type *IntptrTy;
   IntegerType *OrdTy;
   // Callbacks to run-time library are computed in doInitialization.
   Function *ActulFuncEntry;
   Function *ActulFuncExit;
   // Accesses sizes are powers of two: 1, 2, 4, 8, 16.
   static const size_t kNumberOfAccessSizes = 5;
   Function *ActulRead[kNumberOfAccessSizes];
   Function *ActulWrite[kNumberOfAccessSizes];
   Function *ActulUnalignedRead[kNumberOfAccessSizes];
   Function *ActulUnalignedWrite[kNumberOfAccessSizes];
   Function *ActulAtomicLoad[kNumberOfAccessSizes];
   Function *ActulAtomicStore[kNumberOfAccessSizes];
   Function *ActulAtomicRMW[AtomicRMWInst::LAST_BINOP + 1][kNumberOfAccessSizes];
   Function *ActulAtomicCAS[kNumberOfAccessSizes];
   Function *ActulAtomicThreadFence;
   Function *ActulAtomicSignalFence;
   Function *ActulVptrUpdate;
   Function *ActulVptrLoad;
   Function *MemmoveFn, *MemcpyFn, *MemsetFn;
   Function *ActulCtorFunction;
   Function *ActulDtorFunction;
   Function *ConschReplaceFunctions[numReplaceFunctions];

   bool compareCStr(const char * s1, const char * s2);

   long int isReplaceFunction(const char * funcName);

};
}  // namespace

char ActulTester::ID = 0;
static RegisterPass<ActulTester> X("actul", "Testing concurrency issues", false /* Only looks at CFG */, false /* Analysis Pass */);

PASS_NAME_RETURN ActulTester::getPassName() const
{
   return "ActulTester";
}

void ActulTester::getAnalysisUsage(AnalysisUsage &AU) const
{
   AU.addRequired<TargetLibraryInfoWrapperPass>();
}

bool ActulTester::compareCStr(const char * s1, const char * s2)
{
   unsigned i = 0;
   while (s1[i] == s2[i])
   {
      if (s1[i] == '\0')
         return true;
      ++i;
   }
   return false;
}

long int ActulTester::isReplaceFunction(const char * funcName)
{
   for(long int i=0;i<numShouldFunctions;++i)
      if(compareCStr(funcName, shouldBeSupported[i]))
      {
         errs() << "Warning: Function found which should be supported: " << funcName << "\n";
         return -1;
      }

   for (long int i = 0; i < numReplaceFunctions; ++i)
   {
      if (compareCStr(funcName, toReplaceFunctions[i]))
         return i;
   }
   return -1;
}

void ActulTester::initializeCallbacks(Module &M)
{
   IRBuilder<> IRB(M.getContext());
   // Initialize the callbacks.
   ActulFuncEntry = checkSanitizerInterfaceFunction(M.getOrInsertFunction("Actul_func_entry", IRB.getVoidTy(), IRB.getInt8PtrTy(), nullptr));
   ActulFuncExit = checkSanitizerInterfaceFunction(M.getOrInsertFunction("Actul_func_exit", IRB.getVoidTy(), nullptr));
   OrdTy = IRB.getInt32Ty();

   for (size_t i = 0; i < kNumberOfAccessSizes; ++i)
   {
      const unsigned ByteSize = 1U << i;
      const unsigned BitSize = ByteSize * 8;
      std::string ByteSizeStr = utostr(ByteSize);
      std::string BitSizeStr = utostr(BitSize);
      SmallString < 32 > ReadName("Actul_read" + ByteSizeStr);
      ActulRead[i] = checkSanitizerInterfaceFunction(M.getOrInsertFunction(ReadName, IRB.getVoidTy(), IRB.getInt8PtrTy(), nullptr));

      SmallString < 32 > WriteName("Actul_write" + ByteSizeStr);
      ActulWrite[i] = checkSanitizerInterfaceFunction(M.getOrInsertFunction(WriteName, IRB.getVoidTy(), IRB.getInt8PtrTy(), nullptr));

      SmallString < 64 > UnalignedReadName("Actul_unaligned_read" + ByteSizeStr);
      ActulUnalignedRead[i] = checkSanitizerInterfaceFunction(M.getOrInsertFunction(UnalignedReadName, IRB.getVoidTy(), IRB.getInt8PtrTy(), nullptr));

      SmallString < 64 > UnalignedWriteName("Actul_unaligned_write" + ByteSizeStr);
      ActulUnalignedWrite[i] = checkSanitizerInterfaceFunction(M.getOrInsertFunction(UnalignedWriteName, IRB.getVoidTy(), IRB.getInt8PtrTy(), nullptr));

      SmallString < 32 > AtomicLoadName("Actul_atomic" + BitSizeStr + "_load");
      ActulAtomicLoad[i] = checkSanitizerInterfaceFunction(M.getOrInsertFunction(AtomicLoadName, IRB.getVoidTy(), IRB.getInt8PtrTy(), nullptr));

      SmallString < 32 > AtomicStoreName("Actul_atomic" + BitSizeStr + "_store");
      ActulAtomicStore[i] = checkSanitizerInterfaceFunction(M.getOrInsertFunction(AtomicStoreName, IRB.getVoidTy(), IRB.getInt8PtrTy(), nullptr));

      for (int op = AtomicRMWInst::FIRST_BINOP; op <= AtomicRMWInst::LAST_BINOP; ++op)
      {
         ActulAtomicRMW[op][i] = nullptr;
         const char *NamePart = nullptr;
         if (op == AtomicRMWInst::Xchg)
            NamePart = "_exchange";
         else if (op == AtomicRMWInst::Add)
            NamePart = "_fetch_add";
         else if (op == AtomicRMWInst::Sub)
            NamePart = "_fetch_sub";
         else if (op == AtomicRMWInst::And)
            NamePart = "_fetch_and";
         else if (op == AtomicRMWInst::Or)
            NamePart = "_fetch_or";
         else if (op == AtomicRMWInst::Xor)
            NamePart = "_fetch_xor";
         else if (op == AtomicRMWInst::Nand)
            NamePart = "_fetch_nand";
         else
            continue;
         SmallString < 32 > RMWName("Actul_atomic" + itostr(BitSize) + NamePart);
         ActulAtomicRMW[op][i] = checkSanitizerInterfaceFunction(M.getOrInsertFunction(RMWName, IRB.getVoidTy(), IRB.getInt8PtrTy(), nullptr));
      }

      SmallString < 32 > AtomicCASName("Actul_atomic" + BitSizeStr + "_compare_exchange_val");
      ActulAtomicCAS[i] = checkSanitizerInterfaceFunction(M.getOrInsertFunction(AtomicCASName, IRB.getVoidTy(), IRB.getInt8PtrTy(), nullptr));
   }
   ActulVptrUpdate = checkSanitizerInterfaceFunction(M.getOrInsertFunction("Actul_vptr_update", IRB.getVoidTy(), IRB.getInt8PtrTy(), IRB.getInt8PtrTy(), nullptr));
   ActulVptrLoad = checkSanitizerInterfaceFunction(M.getOrInsertFunction("Actul_vptr_read", IRB.getVoidTy(), IRB.getInt8PtrTy(), nullptr));
   //ActulAtomicThreadFence = checkSanitizerInterfaceFunction(M.getOrInsertFunction("Actul_atomic_thread_fence", IRB.getVoidTy(), OrdTy, nullptr));
   ActulAtomicThreadFence = checkSanitizerInterfaceFunction(M.getOrInsertFunction("Actul_atomic_thread_fence", IRB.getVoidTy(), nullptr));
   //ActulAtomicSignalFence = checkSanitizerInterfaceFunction(M.getOrInsertFunction("Actul_atomic_signal_fence", IRB.getVoidTy(), OrdTy, nullptr));
   ActulAtomicSignalFence = checkSanitizerInterfaceFunction(M.getOrInsertFunction("Actul_atomic_signal_fence", IRB.getVoidTy(), nullptr));

   MemmoveFn = checkSanitizerInterfaceFunction(M.getOrInsertFunction("memmove", IRB.getInt8PtrTy(), IRB.getInt8PtrTy(), IRB.getInt8PtrTy(), IntptrTy, nullptr));
   MemcpyFn = checkSanitizerInterfaceFunction(M.getOrInsertFunction("memcpy", IRB.getInt8PtrTy(), IRB.getInt8PtrTy(), IRB.getInt8PtrTy(), IntptrTy, nullptr));
   MemsetFn = checkSanitizerInterfaceFunction(M.getOrInsertFunction("memset", IRB.getInt8PtrTy(), IRB.getInt8PtrTy(), IRB.getInt32Ty(), IntptrTy, nullptr));
}

bool ActulTester::doInitialization(Module &M)
{
   const DataLayout &DL = M.getDataLayout();
   IntptrTy = DL.getIntPtrType(M.getContext());
   std::tie(ActulCtorFunction, std::ignore) = createSanitizerCtorAndInitFunctions(M, kActulModuleCtorName, kActulInitName, /*InitArgTypes=*/{ },
   /*InitArgs=*/{ });
   std::tie(ActulDtorFunction, std::ignore) = createSanitizerCtorAndInitFunctions(M, kActulModuleDtorName, kActulDestrName, /*InitArgTypes=*/{ },
   /*InitArgs=*/{ });

   appendToGlobalCtors(M, ActulCtorFunction, 0);
   appendToGlobalDtors(M, ActulDtorFunction, 0);

   return true;
}

static bool isVtableAccess(Instruction *I)
{
   if (MDNode *Tag = I->getMetadata(LLVMContext::MD_tbaa))
      return Tag->isTBAAVtableAccess();
   return false;
}

// Do not instrument known races/"benign races" that come from compiler
// instrumentatin. The user has no way of suppressing them.
static bool shouldInstrumentReadWriteFromAddress(Value *Addr)
{
   // Peel off GEPs and BitCasts.
   Addr = Addr->stripInBoundsOffsets();

   if (GlobalVariable *GV = dyn_cast < GlobalVariable > (Addr))
   {
      /*if (GV->hasSection())
      {
         StringRef SectionName = GV->getSection();
         // Check if the global is in the PGO counters section.
         if (SectionName.endswith(getInstrProfCountersSectionName(false)))
            return false;
      }*/

      // Check if the global is private gcov data.
      if (GV->getName().startswith("__llvm_gcov") || GV->getName().startswith("__llvm_gcda"))
         return false;
   }

   // Do not instrument acesses from different address spaces; we cannot deal
   // with them.
   if (Addr)
   {
      Type *PtrTy = cast < PointerType > (Addr->getType()->getScalarType());
      if (PtrTy->getPointerAddressSpace() != 0)
         return false;
   }

   return true;
}

bool ActulTester::addrPointsToConstantData(Value *Addr)
{
   // If this is a GEP, just analyze its pointer operand.
   if (GetElementPtrInst *GEP = dyn_cast < GetElementPtrInst > (Addr))
      Addr = GEP->getPointerOperand();

   if (GlobalVariable *GV = dyn_cast < GlobalVariable > (Addr))
   {
      if (GV->isConstant())
      {
         return true;
      }
   } else if (LoadInst *L = dyn_cast < LoadInst > (Addr))
   {
      if (isVtableAccess(L))
      {
         return true;
      }
   }
   return false;
}

void ActulTester::chooseInstructionsToInstrument(SmallVectorImpl<Instruction *> &Local, SmallVectorImpl<Instruction *> &All, const DataLayout &DL)
{
   SmallSet<Value*, 8> WriteTargets;
   // Iterate from the end.
   for (Instruction *I : reverse(Local))
   {
      //if(dyn_cast < StoreInst > (I))
      //   errs() << "instrument store\n";
      //else if(dyn_cast < LoadInst > (I))
      //   errs() << "instrument load\n";
      if (StoreInst *Store = dyn_cast < StoreInst > (I))
      {
         Value *Addr = Store->getPointerOperand();
         if (!shouldInstrumentReadWriteFromAddress(Addr))
         {
            //errs() << "reject0\n";
            continue;
         }
         WriteTargets.insert(Addr);
      } else
      {
         LoadInst *Load = cast < LoadInst > (I);
         Value *Addr = Load->getPointerOperand();
         if (!shouldInstrumentReadWriteFromAddress(Addr))
         {
            //errs() << "reject1\n";
            continue;
         }
         /*if (WriteTargets.count(Addr))
         {
            errs() << "reject2\n";
            continue;
         }*/
         if (addrPointsToConstantData(Addr))
         {
            //errs() << "reject3\n";
            // Addr points to some constant data -- it can not race with any writes.
            continue;
         }
      }
      Value *Addr = isa < StoreInst > (*I) ? cast < StoreInst > (I)->getPointerOperand() : cast < LoadInst > (I)->getPointerOperand();
      if (isa < AllocaInst > (GetUnderlyingObject(Addr, DL)) && !PointerMayBeCaptured(Addr, true, true))
      {
         // The variable is addressable but not captured, so it cannot be
         // referenced from a different thread and participate in a data race
         // (see llvm/Analysis/CaptureTracking.h for details).
         //errs() << "reject4\n";
         continue;
      }
      All.push_back(I);
   }
   Local.clear();
}

static bool isAtomic(Instruction *I)
{
   bool res = false;
   if (LoadInst *LI = dyn_cast < LoadInst > (I))
      res = LI->isAtomic() && LI->getSynchScope() == CrossThread;
   if (StoreInst *SI = dyn_cast < StoreInst > (I))
      res = SI->isAtomic() && SI->getSynchScope() == CrossThread;
   if (isa < AtomicRMWInst > (I))
   {
      //errs() << I->getName() << " atomic rmw\n";
      res = true;
   }
   if (isa < AtomicCmpXchgInst > (I))
   {
      //errs() << " atomic xchg\n";
      res = true;
   }
   if (isa < FenceInst > (I))
      res = true;
   //if (res)
     // errs() << "  found with " << I->getName() << " an atomic\n";
   return res;
}

bool ActulTester::runOnFunction(Function &F)
{
   // This is required to prevent instrumenting call to ActulInit from within
   // the module constructor.
   if (&F == ActulCtorFunction)
      return false;
   initializeCallbacks(*F.getParent());
   SmallVector<Instruction*, 8> RetVec;
   SmallVector<Instruction*, 8> AllLoadsAndStores;
   SmallVector<Instruction*, 8> LocalLoadsAndStores;
   SmallVector<Instruction*, 8> AtomicAccesses;
   SmallVector<Instruction*, 8> MemIntrinCalls;

   bool Res = false;
   bool HasCalls = false;
   const DataLayout &DL = F.getParent()->getDataLayout();

   //errs() << "Func " << F.getName() << "\n";
   for (auto &BB : F)
   {
      for (auto &Inst : BB)
      {
         if (isAtomic(&Inst))
            AtomicAccesses.push_back(&Inst);
         else if (isa < LoadInst > (Inst) || isa < StoreInst > (Inst))
            LocalLoadsAndStores.push_back(&Inst);
         else if (isa < ReturnInst > (Inst))
            RetVec.push_back(&Inst);
         else if (isa < CallInst > (Inst) || isa < InvokeInst > (Inst))
         {
            bool isConsch = false;
            if (CallInst *CI = dyn_cast < CallInst > (&Inst))
            {
               if (Function * f = CI->getCalledFunction())
               {
                  std::string n = f->getName().str();
                  long int pos = isReplaceFunction(n.c_str());
                  if (pos > -1)
                  {
                     isConsch = true;
                     //errs() << "replace '" << f->getName() << "' with '"<< std::string("Actul_") << std::string(f->getName()) << "'\n";
                     f->setName(std::string("Actul_") + std::string(f->getName()));
                  }
               }
            }
            else if(InvokeInst *CI = dyn_cast < InvokeInst > (&Inst))
            {
               if (Function * f = CI->getCalledFunction())
               {
                  std::string n = f->getName().str();
                  //errs() << " maybe replace '" << f->getName() << "' with '"<< std::string("Actul_") << std::string(f->getName()) << "'\n";
               }
            }

            if (isa < MemIntrinsic > (Inst) && !isConsch)
               MemIntrinCalls.push_back(&Inst);
            HasCalls = true;
            chooseInstructionsToInstrument(LocalLoadsAndStores, AllLoadsAndStores, DL);
         }
      }
      chooseInstructionsToInstrument(LocalLoadsAndStores, AllLoadsAndStores, DL);
   }


   for (auto Inst : AllLoadsAndStores)
   {
      Res |= instrumentLoadOrStore(Inst, DL);
   }

   for (auto Inst : AtomicAccesses)
   {
      Res |= instrumentAtomic(Inst, DL);
   }

   for (auto Inst : MemIntrinCalls)
   {
      Res |= instrumentMemIntrinsic(Inst);
   }

   // Instrument function entry/exit points if there were instrumented accesses.
   if (Res || HasCalls)
   {
      IRBuilder<> IRB(F.getEntryBlock().getFirstNonPHI());
      Value *ReturnAddress = IRB.CreateCall(Intrinsic::getDeclaration(F.getParent(), Intrinsic::returnaddress), IRB.getInt32(0));
      //Constant *ReturnAddress =  ConstantExpr::getBitCast(dyn_cast<Constant>(&F),Type::getInt8PtrTy(F.getContext()));
      //errs() << "add func entry: " << F.getName() << " on addr " << *ReturnAddress << "\n";
      IRB.CreateCall(ActulFuncEntry, ReturnAddress);
      for (auto RetInst : RetVec)
      {
         IRBuilder<> IRBRet(RetInst);
         //errs() << "add func exit: " << F.getName() << " on addr " << *ReturnAddress << "\n";
         IRBRet.CreateCall(ActulFuncExit, { });
      }
      Res = true;
   }
   return Res;
}

bool ActulTester::instrumentLoadOrStore(Instruction *I, const DataLayout &DL)
{
   IRBuilder<> IRB(I);
   bool IsWrite = isa < StoreInst > (*I);
   Value *Addr = IsWrite ? cast < StoreInst > (I)->getPointerOperand() : cast < LoadInst > (I)->getPointerOperand();
   int Idx = getMemoryAccessFuncIndex(Addr, DL);
   if (Idx < 0)
      return false;
   if (IsWrite && isVtableAccess(I))
   {
      Value *StoredValue = cast < StoreInst > (I)->getValueOperand();
      // StoredValue may be a vector type if we are storing several vptrs at once.
      // In this case, just take the first element of the vector since this is
      // enough to find vptr races.
      if (isa < VectorType > (StoredValue->getType()))
         StoredValue = IRB.CreateExtractElement(StoredValue, ConstantInt::get(IRB.getInt32Ty(), 0));
      if (StoredValue->getType()->isIntegerTy())
         StoredValue = IRB.CreateIntToPtr(StoredValue, IRB.getInt8PtrTy());
      // Call ActulVptrUpdate.
      IRB.CreateCall(ActulVptrUpdate, { IRB.CreatePointerCast(Addr, IRB.getInt8PtrTy()), IRB.CreatePointerCast(StoredValue, IRB.getInt8PtrTy()) });
      return true;
   }
   if (!IsWrite && isVtableAccess(I))
   {
      IRB.CreateCall(ActulVptrLoad, IRB.CreatePointerCast(Addr, IRB.getInt8PtrTy()));
      return true;
   }
   const unsigned Alignment = IsWrite ? cast < StoreInst > (I)->getAlignment() : cast < LoadInst > (I)->getAlignment();
   Type *OrigTy = cast < PointerType > (Addr->getType())->getElementType();
   const uint32_t TypeSize = DL.getTypeStoreSizeInBits(OrigTy);
   Value *OnAccessFunc = nullptr;
   if (Alignment == 0 || Alignment >= 8 || (Alignment % (TypeSize / 8)) == 0)
      OnAccessFunc = IsWrite ? ActulWrite[Idx] : ActulRead[Idx];
   else
      OnAccessFunc = IsWrite ? ActulUnalignedWrite[Idx] : ActulUnalignedRead[Idx];
   IRB.CreateCall(OnAccessFunc, IRB.CreatePointerCast(Addr, IRB.getInt8PtrTy()));
   return true;
}

#define LLVM_FALLTHROUGH

// If a memset intrinsic gets inlined by the code gen, we will miss races on it.
// So, we either need to ensure the intrinsic is not inlined, or instrument it.
// We do not instrument memset/memmove/memcpy intrinsics (too complicated),
// instead we simply replace them with regular function calls, which are then
// intercepted by the run-time.
// Since tsan is running after everyone else, the calls should not be
// replaced back with intrinsics. If that becomes wrong at some point,
// we will need to call e.g. ActulMemset to avoid the intrinsics.
bool ActulTester::instrumentMemIntrinsic(Instruction *I)
{
   IRBuilder<> IRB(I);
   if (MemSetInst *M = dyn_cast < MemSetInst > (I))
   {
      IRB.CreateCall(
            MemsetFn,
            { IRB.CreatePointerCast(M->getArgOperand(0), IRB.getInt8PtrTy()), IRB.CreateIntCast(M->getArgOperand(1), IRB.getInt32Ty(), false), IRB.CreateIntCast(
                  M->getArgOperand(2), IntptrTy, false) });
      I->eraseFromParent();
   } else if (MemTransferInst * M = dyn_cast < MemTransferInst > (I))
   {
      IRB.CreateCall(
            isa < MemCpyInst > (M) ? MemcpyFn : MemmoveFn,
            { IRB.CreatePointerCast(M->getArgOperand(0), IRB.getInt8PtrTy()), IRB.CreatePointerCast(M->getArgOperand(1), IRB.getInt8PtrTy()), IRB.CreateIntCast(M->getArgOperand(2),
                                                                                                                                                                IntptrTy, false) });
      I->eraseFromParent();
   }
   return false;
}

bool ActulTester::instrumentAtomic(Instruction *I, const DataLayout &DL)
{
   IRBuilder<> IRB(I);

   if (LoadInst *LI = dyn_cast < LoadInst > (I))
   {
      Value *Addr = LI->getPointerOperand();
      int Idx = getMemoryAccessFuncIndex(Addr, DL);
      if (Idx < 0)
         return false;

      IRB.CreateCall(ActulAtomicLoad[Idx], IRB.CreatePointerCast(Addr, IRB.getInt8PtrTy()));

   } else if (StoreInst *SI = dyn_cast < StoreInst > (I))
   {
      Value *Addr = SI->getPointerOperand();
      int Idx = getMemoryAccessFuncIndex(Addr, DL);
      if (Idx < 0)
         return false;

      IRB.CreateCall(ActulAtomicStore[Idx], IRB.CreatePointerCast(Addr, IRB.getInt8PtrTy()));

   } else if (AtomicRMWInst *RMWI = dyn_cast < AtomicRMWInst > (I))
   {
      Value *Addr = RMWI->getPointerOperand();
      int Idx = getMemoryAccessFuncIndex(Addr, DL);
      if (Idx < 0)
      {
         //errs() << "    idx fail rmw\n";
         return false;
      }
      Function *F = ActulAtomicRMW[RMWI->getOperation()][Idx];
      if (!F)
      {
         //errs() << "    func fail on (" << RMWI->getOperation() << "," << Idx << ") rmw\n";
         return false;
      }

      IRB.CreateCall(ActulAtomicRMW[RMWI->getOperation()][Idx], IRB.CreatePointerCast(Addr, IRB.getInt8PtrTy()));
      //errs() << "    success rmw\n";
   } else if (AtomicCmpXchgInst *CASI = dyn_cast < AtomicCmpXchgInst > (I))
   {
      Value *Addr = CASI->getPointerOperand();
      int Idx = getMemoryAccessFuncIndex(Addr, DL);
      if (Idx < 0)
         return false;
      IRB.CreateCall(ActulAtomicCAS[Idx], IRB.CreatePointerCast(Addr, IRB.getInt8PtrTy()));

   } else if (FenceInst *FI = dyn_cast < FenceInst > (I))
   {
      Function *F = FI->getSynchScope() == SingleThread ? ActulAtomicSignalFence : ActulAtomicThreadFence;
      IRB.CreateCall(F, IRB.CreatePointerCast(0, IRB.getInt8PtrTy()));
   }
   return true;
}

int ActulTester::getMemoryAccessFuncIndex(Value *Addr, const DataLayout &DL)
{
   Type *OrigPtrTy = Addr->getType();
   Type *OrigTy = cast < PointerType > (OrigPtrTy)->getElementType();
   assert(OrigTy->isSized());
   uint32_t TypeSize = DL.getTypeStoreSizeInBits(OrigTy);
   if (TypeSize != 8 && TypeSize != 16 && TypeSize != 32 && TypeSize != 64 && TypeSize != 128)
   {
      // Ignore all unusual sizes.
      return -1;
   }
   size_t Idx = countTrailingZeros(TypeSize / 8);
   assert(Idx < kNumberOfAccessSizes);
   return Idx;
}

static void registerActulPass(const PassManagerBuilder &, legacy::PassManagerBase &PM)
{
   PM.add(new ActulTester());
}

static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_OptimizerLast, registerActulPass);

static RegisterStandardPasses RegisterMyPass0(PassManagerBuilder::EP_EnabledOnOptLevel0, registerActulPass);

