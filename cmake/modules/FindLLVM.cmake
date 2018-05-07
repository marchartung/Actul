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
# Copied from https://llvm.org/svn/llvm-project/llbrowse/trunk/cmake_modules/
# Find the native LLVM includes and library
#
#  LLVM_INCLUDE_DIR - where to find llvm include files
#  LLVM_LIBRARY_DIR - where to find llvm libs
#  LLVM_CFLAGS      - llvm compiler flags
#  LLVM_CXXFLAGS      - llvm compiler flags
#  LLVM_LDFLAGS      - llvm linker flags
#  LLVM_LIBS      - llvm libraries
#  LLVM_SYSTEM_LIBS      - llvm system libraries
#  LLVM_BIN_DIR     - directory of llvm executables
#  LLVM_FOUND       - True if llvm found.

find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config DOC "llvm-config executable")

if (LLVM_CONFIG_EXECUTABLE)
  message(STATUS "LLVM llvm-config found at: ${LLVM_CONFIG_EXECUTABLE}")
else (LLVM_CONFIG_EXECUTABLE)
  message(FATAL_ERROR "Could NOT find LLVM executable. Please set -DLLVM_CONFIG_EXECUTABLE=/path/to/llvm-config")
endif (LLVM_CONFIG_EXECUTABLE)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir
  OUTPUT_VARIABLE LLVM_INCLUDE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir
  OUTPUT_VARIABLE LLVM_LIBRARY_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --cppflags
  OUTPUT_VARIABLE LLVM_CFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags
  OUTPUT_VARIABLE LLVM_CXXFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
  OUTPUT_VARIABLE LLVM_LDFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs
  OUTPUT_VARIABLE LLVM_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --system-libs
  OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --bindir
  OUTPUT_VARIABLE LLVM_BIN_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

