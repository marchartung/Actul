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

cmake_minimum_required( VERSION 3.1 )
project( ActulProject )

#optional add c++11 support for eclipse
if(ECLIPSE)
	add_definitions(-D__cplusplus=201103L)
endif(ECLIPSE)

add_definitions(-DACTUL_DEBUG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

if(LLVM_INSTALL_DIR)
        set(LLVM_CONFIG_EXECUTABLE "${LLVM_INSTALL_DIR}/bin/llvm-config")
        set(CLANG_EXECUTABLE "${LLVM_INSTALL_DIR}/bin/clang")
else(LLVM_INSTALL_DIR)
		message(FATAL_ERROR "Please specify -DLLVM_INSTALL_DIR=/path/to/llvm/install (for example /usr)")
endif(LLVM_INSTALL_DIR)

if(LLVM_OLD)
        add_definitions(-DUSE_LLVM_OLD)
endif(LLVM_OLD)

find_package(LLVM REQUIRED)


#set clang compiler
#set(CMAKE_C_COMPILER "${CLANG_EXECUTABLE}")
#set(CMAKE_CXX_COMPILER "${CLANG_EXECUTABLE}++")
#message(STATUS "Set CXX compiler to ${CMAKE_CXX_COMPILER}")


file (GLOB_RECURSE CPP_TARGETS "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/IntrumentationRoutines )

#libraries:
set(LIBRARIES pthread ${LLVM_LIBS} ${LLVM_SYSTEM_LIBS})
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_definitions(${LLVM_CXXFLAGS})
set(INCLUDE_DIR ${INCLUDE_DIR} ${LLVM_INCLUDE_DIR})


	#add function pass
add_library(LLVMActul SHARED "${CMAKE_CURRENT_SOURCE_DIR}/llvm_function_pass/ActulInstrumentation.cpp")
target_include_directories(LLVMActul PUBLIC ${INCLUDE_DIR})
target_link_libraries( LLVMActul ${LIBRARIES_PASS})
target_compile_features(LLVMActul PUBLIC cxx_range_for)
install(TARGETS LLVMActul DESTINATION lib)

#build actul runtime library
add_library(actulRTL STATIC ${CPP_TARGETS})
target_compile_features(actulRTL PUBLIC cxx_range_for)
target_link_libraries( actulRTL "${LIBRARIES}")
link_directories( actulRTL "${LLVM_LIBRARY_DIR}")
target_include_directories(actulRTL PUBLIC "${INCLUDE_DIR}" )
install(TARGETS actulRTL DESTINATION lib) 

#generate test build wrapper
set(ACTUL_LIB_DIR "${PROJECT_BINARY_DIR}")
set(LLVM_ROOT_DIR "${LLVM_INSTALL_DIR}")
set(CLANG_ROOT_DIR "${LLVM_INSTALL_DIR}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/wrapper/bash_c_wrapper.in" test_actul @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/wrapper/bash_cxx_wrapper.in" test_actul++ @ONLY)

#generate actul compiler/linker wrapper
set(ACTUL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/wrapper/bash_c_wrapper.in" actul @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/actul DESTINATION bin)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/wrapper/bash_cxx_wrapper.in" actul++ @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/actul++ DESTINATION bin)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/wrapper/bash_linker_wrapper.in" actulld @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/actulld DESTINATION bin)


# add tests
file( GLOB CPP_TARGETS "${CMAKE_CURRENT_SOURCE_DIR}/test/*.dcpp")
foreach( testfile ${CPP_TARGETS} )
	get_filename_component( outputname ${testfile} NAME)
	string( REPLACE ".cpp" "" outputname ${outputname} )
	message(STATUS "Building test ${outputname}")
	add_custom_command(
    OUTPUT
        ${outputname}
    COMMAND
        bash ${PROJECT_BINARY_DIR}/test_actul++ -g -o ${outputname} ${testfile}
    VERBATIM
    ) 
    add_custom_target( "${outputname}_t" ALL COMMAND bash ${PROJECT_BINARY_DIR}/test_actul++ -o ${outputname} ${testfile} )
    add_dependencies( "${outputname}_t" actulRTL LLVMActul )
endforeach( testfile ${CPP_TARGETS} )


