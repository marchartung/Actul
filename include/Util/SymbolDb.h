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
#ifndef INCLUDE_UTIL_SYMBOLDB_H_
#define INCLUDE_UTIL_SYMBOLDB_H_
#include <llvm/DebugInfo/Symbolize/Symbolize.h>
#include <Util/String.h>

#include <map>

namespace Actul
{

struct FunctionSymbol
{
   String info;

   FunctionSymbol()
      : info("")
   {
   }

   FunctionSymbol(const String & in)
      : info(in)
   {
   }
};

struct VariableSymbol
{
   String info;

   VariableSymbol(const String & in)
      : info(in)
   {
      if(info.size() == 0)
         info = String("[unknown variable]");
   }

};

/**
 * Starts 'llvm-symbolizer' if possible and uses commFile to communicate with the process to get program symbols
 */

class SymbolDb
{
 public:

   SymbolDb(const SymbolDb & db) = delete;

   SymbolDb();

   ~SymbolDb();

   FunctionSymbol getFunctionSymbol(const AddressType & ptr);

   VariableSymbol getVariableSymbol(const AddressType & ptr);


 private:
   llvm::symbolize::LLVMSymbolizer::Options _llvmOpts;
   llvm::symbolize::LLVMSymbolizer _llvmSymbolizer;
   String _programName;

   String sendFunctionRequest(const AddressType & ptr);

   String sendVariableRequest(const AddressType & ptr);

};

} /* namespace Actul */

#endif /* INCLUDE_UTIL_SYMBOLDB_H_ */
