## BSD 2-Clause License
## 
## Copyright (c) 2019, ZHOU He
## All rights reserved.
## 
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
## 
## 1. Redistributions of source code must retain the above copyright notice, this
##    list of conditions and the following disclaimer.
## 
## 2. Redistributions in binary form must reproduce the above copyright notice,
##    this list of conditions and the following disclaimer in the documentation
##    and/or other materials provided with the distribution.
## 
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
## DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
## FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
## DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
## SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
## CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
## OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.11)
project(MetaCompiler)

set(LLVM_GIT "" CACHE PATH "LLVM's git path")
set(LLVM_BUILD "" CACHE PATH "LLVM's build path")
if(NOT LLVM_GIT STREQUAL "" AND NOT LLVM_BUILD STREQUAL "")
	file(GLOB MetaCompilerSrc *.cxx *.h)
	add_executable(metaCompiler ${MetaCompilerSrc})
	file(GLOB llvm_libs_debug ${LLVM_BUILD}/Debug/lib/*.lib)
	string(REPLACE ";" ";debug;" llvm_libs_debug "${llvm_libs_debug}")
	file(GLOB llvm_libs_optimized ${LLVM_BUILD}/RelWithDebInfo/lib/*.lib)
	string(REPLACE ";" ";optimized;" llvm_libs_optimized "${llvm_libs_optimized}")
	if(NOT llvm_libs_debug STREQUAL "")
		target_link_libraries(metaCompiler PUBLIC debug ${llvm_libs_debug})
	endif()
	if(NOT llvm_libs_optimized STREQUAL "")
		target_link_libraries(metaCompiler PUBLIC optimized ${llvm_libs_optimized})
	endif()
	if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
		target_link_libraries(metaCompiler PUBLIC mincore version)
	endif()
	set_target_properties(metaCompiler PROPERTIES CXX_STANDARD 17)
	target_include_directories(metaCompiler PUBLIC ${LLVM_GIT}/llvm/include ${LLVM_BUILD}/include ${LLVM_GIT}/clang/include ${LLVM_BUILD}/tools/clang/include)
	set_target_properties( metaCompiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/meta-exe/${CMAKE_SYSTEM_NAME})
else()
	message("Fatal Error:")
	message("You should set 'LLVM_GIT' and 'LLVM_BUILD' variables for cmake manually!")
	message("LLVM_GIT is llvm's git clone path.")
	message("LLVM_BUILD is llvm's cmake build path.")
	message("You should build ClangTooling before building MetaCompiler.")
endif()
