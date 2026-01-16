# Copyright 2026 Nihilai Collective Corp
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License

cmake_minimum_required(VERSION 3.20)

project(OACC 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Order Agnostic Constexpr Configuration"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(OACC_GENERATE_ASSEMBLY "Generate assembly output for examples" ON)
option(OACC_VERBOSE_BUILD "Enable verbose build output" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(MSVC_COMPILE_OPTIONS 
    /std:c++latest
    /Zc:preprocessor
    /Zc:__cplusplus
    /permissive-
    /EHsc
    /O2
    /Ob3
)

set(CLANG_COMPILE_OPTIONS 
    $<$<CONFIG:Release>:-O3>
)

set(GNU_COMPILE_OPTIONS     
    $<$<CONFIG:Release>:-O3>
)

set(ASSEMBLY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/assembly_output")
file(MAKE_DIRECTORY ${ASSEMBLY_OUTPUT_DIR})
        
get_filename_component(EXAMPLE_NAME ./src/oacc_example.cpp NAME_WE)
            
add_executable(oacc ${CMAKE_SOURCE_DIR}/src/oacc_example.cpp)

target_compile_options(oacc PUBLIC
    $<$<CXX_COMPILER_ID:Clang>:${CLANG_COMPILE_OPTIONS}>
    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_COMPILE_OPTIONS}>
    $<$<CXX_COMPILER_ID:GNU>:${GNU_COMPILE_OPTIONS}>            
)

install(FILES $<TARGET_FILE:oacc>
    DESTINATION lib/cmake/OACC
)
            
set_target_properties(oacc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)
            
if(OACC_VERBOSE_BUILD)
    message(STATUS "Configured example: oacc")
endif()

message(${ASSEMBLY_OUTPUT_DIR})

if(OACC_GENERATE_ASSEMBLY)
    set(ASSEMBLY_FILE "${ASSEMBLY_OUTPUT_DIR}/oacc.s")
                
    if(MSVC)
        add_custom_command(
            TARGET oacc POST_BUILD
            COMMAND ${CMAKE_CXX_COMPILER}
                /c
                /FAcs
                /Fa${ASSEMBLY_FILE}
                ${CMAKE_SOURCE_DIR}/src/oacc_example.cpp
                /I${CMAKE_CURRENT_SOURCE_DIR}/include
                ${MSVC_COMPILE_OPTIONS}
            BYPRODUCTS ${ASSEMBLY_FILE}
            COMMENT "Generating assembly for oacc (MSVC)"
            VERBATIM
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_custom_command(
            TARGET oacc POST_BUILD
            COMMAND ${CMAKE_CXX_COMPILER}
                -S
                -masm=intel
                -fverbose-asm
                -O3
                -march=native
                -std=c++23
                -I${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/src/oacc_example.cpp
                -o ${ASSEMBLY_FILE}
            BYPRODUCTS ${ASSEMBLY_FILE}
            COMMENT "Generating assembly for oacc (Clang)"
            VERBATIM
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_custom_command(
            TARGET oacc POST_BUILD
            COMMAND ${CMAKE_CXX_COMPILER}
                -S
                -masm=intel
                -fverbose-asm
                -O3
                -march=native
                -std=c++23
                -I${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/src/oacc_example.cpp
                -o ${ASSEMBLY_FILE}
            BYPRODUCTS ${ASSEMBLY_FILE}
            COMMENT "Generating assembly for oacc (GCC)"
            VERBATIM
        )
    endif()
                
    if(OACC_VERBOSE_BUILD)
        message(STATUS "  Assembly output: ${ASSEMBLY_FILE}")
    endif()
endif()
        
message(STATUS "OACC Examples configured:")
message(STATUS "  Build directory: ${CMAKE_BINARY_DIR}/bin")
if(OACC_GENERATE_ASSEMBLY)
    message(STATUS "  Assembly output: ${ASSEMBLY_OUTPUT_DIR}")
endif()

message(STATUS "")
message(STATUS "OACC Configuration Summary")
message(STATUS "================================")
message(STATUS "Version:          ${PROJECT_VERSION}")
message(STATUS "Build Type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Standard:     C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build Examples:   ${OACC_BUILD_EXAMPLES}")
message(STATUS "Generate ASM:     ${OACC_GENERATE_ASSEMBLY}")
message(STATUS "Install Prefix:   ${CMAKE_INSTALL_PREFIX}")
message(STATUS "================================")
message(STATUS "")