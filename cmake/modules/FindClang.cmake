#-------------------------------------------------------------------------------
# Copyright 2017-2018 Zuse Institute Berlin
# 
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy
# of the License at
# 
#   http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
# 
# Contributors:
# 	Marc Hartung
#-------------------------------------------------------------------------------
# copied from https://github.com/rizsotto/Constantine
# output:
#   CLANG_FOUND
#   CLANG_INCLUDE_DIRS
#   CLANG_DEFINITIONS
#   CLANG_EXECUTABLE

function(set_clang_definitions config_cmd)
  execute_process(
    COMMAND ${config_cmd} --cppflags
    OUTPUT_VARIABLE llvm_cppflags
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX MATCHALL "(-D[^ ]*)" dflags ${llvm_cppflags})
  string(REGEX MATCHALL "(-U[^ ]*)" uflags ${llvm_cppflags})
  list(APPEND cxxflags ${dflags})
  list(APPEND cxxflags ${uflags})
  list(APPEND cxxflags -fno-rtti)
  list(APPEND cxxflags -fno-exceptions)

  set(CLANG_DEFINITIONS ${cxxflags} PARENT_SCOPE)
endfunction()

function(is_clang_installed config_cmd)
  execute_process(
    COMMAND ${config_cmd} --includedir
    OUTPUT_VARIABLE include_dirs
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(
    COMMAND ${config_cmd} --src-root
    OUTPUT_VARIABLE llvm_src_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(FIND ${include_dirs} ${llvm_src_dir} result)

  set(CLANG_INSTALLED ${result} PARENT_SCOPE)
endfunction()

function(set_clang_include_dirs config_cmd)
  is_clang_installed(${config_cmd})
  if(CLANG_INSTALLED)
    execute_process(
      COMMAND ${config_cmd} --includedir
      OUTPUT_VARIABLE include_dirs
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  else()
    execute_process(
      COMMAND ${config_cmd} --src-root
      OUTPUT_VARIABLE llvm_src_dir
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(
      COMMAND ${config_cmd} --obj-root
      OUTPUT_VARIABLE llvm_obj_dir
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    list(APPEND include_dirs "${llvm_src_dir}/include")
    list(APPEND include_dirs "${llvm_obj_dir}/include")
    list(APPEND include_dirs "${llvm_src_dir}/tools/clang/include")
    list(APPEND include_dirs "${llvm_obj_dir}/tools/clang/include")
  endif()

  set(CLANG_INCLUDE_DIRS ${include_dirs} PARENT_SCOPE)
endfunction()


find_program(LLVM_CONFIG_EXECUTABLE
  NAMES llvm-config-3.8 llvm-config-3.7 llvm-config
  PATHS ENV LLVM_PATH)
if(LLVM_CONFIG_EXECUTABLE)
  message(STATUS "llvm-config found : ${LLVM_CONFIG_EXECUTABLE}")
else()
  message(FATAL_ERROR "Can't found program: llvm-config")
endif()

find_program(CLANG_EXECUTABLE
  NAMES clang-3.8 clang-3.7 clang
  PATHS ENV LLVM_PATH)
if(CLANG_EXECUTABLE)
  message(STATUS "clang found : ${CLANG_EXECUTABLE}")
else()
  message(FATAL_ERROR "Can't found program: clang")
endif()

set_clang_definitions(${LLVM_CONFIG_EXECUTABLE})
set_clang_include_dirs(${LLVM_CONFIG_EXECUTABLE})

message(STATUS "llvm-config filtered cpp flags : ${CLANG_DEFINITIONS}")
message(STATUS "llvm-config filtered include dirs : ${CLANG_INCLUDE_DIRS}")

set(CLANG_FOUND 1)
