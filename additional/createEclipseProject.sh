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
#!/bin/bash

TARGET_DIR="$1" # first argument is the absolute path to the  directory in which the project should created
LLVM_INSTALL="$2"
CMAKE_LOCATION=`pwd` # the script should be called from the location, where the cmake script is

if [ -z "$TARGET_DIR" ]; then
    echo "No target directory specified."
    exit 0
fi


if [ -z "$LLVM_INSTALL" ]; then
    echo "No llvm install directory specified."
    exit 0
fi

mkdir -p $TARGET_DIR
cd $TARGET_DIR
CONTENT=`ls`
if [ -n "$CONTENT" ]; then
    echo "start clean"
    make clean
    echo "end clean"
fi
echo "removing old CMake files"
rm CMakeCache.txt
rm -r CMakeFiles
echo "generating new CMake files"
cmake -G"Eclipse CDT4 - Unix Makefiles" -DECLIPSE=true  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-Wall -pedantic" -DLLVM_INSTALL_DIR="$LLVM_INSTALL"  $CMAKE_LOCATION
cd -
