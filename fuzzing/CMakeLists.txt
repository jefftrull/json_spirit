# CMake build control file for json_spirit fuzzing
# Copyright (C) 2015 Jeff Trull <edaskel@att.net>
#
#   Distributed under the Boost Software License, Version 1.0. (See accompanying
#   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#

set( SANITIZER "address" CACHE STRING "which sanitizer to use with fuzzing - address or memory" )
set( LLVM_ROOT "" CACHE PATH "path to the LLVM build containing libFuzzer" )
# These settings are only needed for MSan builds:
set( ICU_MSAN_PATH "" CACHE PATH "path to the memory sanitized ICU build" )
set( LIBCXX_MSAN_PATH "" CACHE PATH "path to the memory sanitized libc++ build" )

# set up directory-wide fuzzer build settings
add_compile_options( "-fsanitize=${SANITIZER}" "-g" "-O1" )
if( SANITIZER STREQUAL "address" )
  add_compile_options( "-fstack-protector-all" )
  add_definitions( "-D_FORTIFY_SOURCE=2" )
  set( FUZZ_TARGET_LINK_FLAGS "-fsanitize=${SANITIZER}" )
elseif( SANITIZER STREQUAL "memory" )
  # requires Clang and a custom-built stdlib; for now we only support libc++
  add_compile_options( "-fsanitize-memory-track-origins" "-stdlib=libc++" )
  set( FUZZ_TARGET_LINK_FLAGS "-fsanitize=${SANITIZER} -fsanitize-memory-track-origins -stdlib=libc++" )
  if( ICU_MSAN_PATH )
    link_directories( ${ICU_MSAN_PATH}/lib )
  endif()
  if( LIBCXX_MSAN_PATH )
    link_directories( ${LIBCXX_MSAN_PATH}/lib )
  endif()
else()
  message( FATAL_ERROR "unknown sanitizer ${SANITIZER}" )
endif()

# Check for unsupported combinations
if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
  # are we using the special AFL instrumented compiler?
  string(REGEX MATCH "afl-clang-fast\\+\\+$" AFL ${CMAKE_CXX_COMPILER})
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # or the gcc equivalent?
  string(REGEX MATCH "afl-g\\+\\+$" AFL ${CMAKE_CXX_COMPILER})
else()
  message( FATAL_ERROR "Fuzzing only works with gcc or Clang" )
endif()

if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
  if( SANITIZER STREQUAL "memory" )
    message( FATAL_ERROR "gcc does not support memory sanitizing" )
  endif()
  if( NOT AFL ) 
    # libFuzzer requires the sanitize-coverage option, which is not present in gcc
    message( FATAL_ERROR "libFuzzer is not supported under gcc at this time."
                         "You can use AFL (asan only), though, if you set CMAKE_CXX_COMPILER to an afl-instrumented compiler" )
  endif()
endif()

if( WIN32 AND NOT AFL )
  message( FATAL_ERROR "libFuzzer does not work under Windows, for now" )
endif()

# produce the fuzzer driver target
if( AFL )
  # driver executable supplied to afl-fuzz gets input from stdin
  add_executable( fuzzer fuzz_afl.cpp )
elseif( ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" ) AND LLVM_ROOT )
  # the libFuzzer-based binary is built from libFuzzer and a special driver function
  add_executable( fuzzer fuzz_lib.cpp )

  # the library it depends on
  file(GLOB LIBFUZZ_SOURCES ${LLVM_ROOT}/lib/Fuzzer/*.cpp)
  add_library( fuzz ${LIBFUZZ_SOURCES} )
  target_link_libraries( fuzzer fuzz )
  if ( SANITIZER STREQUAL "address" )
    # Don't apply sanitizers to libFuzzer, except for "memory".  Override with defaults:
    string( REPLACE " " ";" LIBFUZZ_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}" )
    set_target_properties( fuzz PROPERTIES COMPILE_OPTIONS "${LIBFUZZ_CXX_FLAGS}" )
  endif()
  target_include_directories( fuzz SYSTEM PUBLIC ${LLVM_ROOT}/lib/Fuzzer )

else()
  message( FATAL_ERROR "Don't know which fuzzer to build - afl compilers not in use and Clang+LLVM_ROOT not specified" )
endif()

# Create a version of the JSON library with sanitizers and coverage enabled
add_library( json_fuzz STATIC ${SRC_FILES} )

target_link_libraries( fuzzer json_fuzz  ${Boost_LOCALE_LIBRARY} )
set_target_properties( fuzzer PROPERTIES
  LINK_FLAGS ${FUZZ_TARGET_LINK_FLAGS}
  INSTALL_RPATH_USE_LINK_PATH ON )    # pulls ICU path into executable's RPATH
