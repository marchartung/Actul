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
#include <Scheduler/ProcessInfo.h>
#include <Util/Defines.h>
#include <Util/SymbolDb.h>

#include <cstdint>


using namespace llvm;
using namespace symbolize;

namespace Actul
{

FunctionSymbol SymbolDb::getFunctionSymbol(const AddressType & ptr)
{
   String tmp = sendFunctionRequest(ptr);
   return FunctionSymbol(tmp);
}

VariableSymbol SymbolDb::getVariableSymbol(const AddressType & ptr)
{
   String tmp = sendVariableRequest(ptr);
   return VariableSymbol(tmp);
}

SymbolDb::SymbolDb()
      : _llvmOpts(FunctionNameKind::LinkageName),
        _llvmSymbolizer(_llvmOpts),
        _programName(ProcessInfo::getProgramName())
{
}

SymbolDb::~SymbolDb()
{
}

template<typename T>
static bool error(Expected<T> &ResOrErr)
{
   if (ResOrErr)
      return false;
   return true;
}

String SymbolDb::sendFunctionRequest(const AddressType & ptr)
{
   std::stringstream ss;
   ss.clear();
   ss << const_cast<void *>(ptr);
   std::string offset = ss.str();
   int64_t ModuleOffset = 0;
   StringRef(offset.c_str()).getAsInteger(0, ModuleOffset);
   auto ResOrErr = _llvmSymbolizer.symbolizeCode(_programName.c_str(), ModuleOffset);
   DILineInfo info = error(ResOrErr) ? DILineInfo() : ResOrErr.get();
   ss.str("");
   ss << info.FunctionName << " in " << info.FileName << ":" << info.Line;
   return String(ss.str().c_str());
}

String SymbolDb::sendVariableRequest(const AddressType & ptr)
{
   std::stringstream stream;
   stream.clear();
   stream << const_cast<void *>(ptr);
   std::string offset = stream.str();
   int64_t ModuleOffset = 0;
   StringRef(offset.c_str()).getAsInteger(0, ModuleOffset);
   auto ResOrErr = _llvmSymbolizer.symbolizeData(_programName.c_str(), ModuleOffset);
   DIGlobal info = error(ResOrErr) ? DIGlobal() : ResOrErr.get();
   stream.str("");
   stream << info.Name;
   return String(stream.str().c_str());
}

} /* namespace Actul */
