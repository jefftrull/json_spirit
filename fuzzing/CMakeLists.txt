# CMake build control file for json_spirit fuzzing
# Copyright (C) 2015 Jeff Trull <edaskel@att.net>
#
#   Distributed under the Boost Software License, Version 1.0. (See accompanying
#   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#

set( SANITIZER address )    # "memory" is the other supported option

# FUZZING USAGE:

# libFuzzer (requires Clang and its tooling library):
# cmake -DLLVM_ROOT=/path/to/llvm -DJSON_SPIRIT_FUZZING=ON ..;make
# libs/json/test/fuzzer -max_len=256 -jobs=2 /path/to/corpus

# AFL via Clang "fast" instrumentation
# cmake -DCMAKE_CXX_COMPILER=/path/to/afl/afl-clang-fast++ -DJSON_SPIRIT_FUZZING=ON LLVM_ROOT=/path/to/llvm ..;make
# /path/to/afl/afl-fuzz -i /path/to/corpus -o FINDINGS -m none -- libs/json/test/fuzzer

# AFL via g++
# cmake -DCMAKE_CXX_COMPILER=/path/to/afl/afl-g++ -DJSON_SPIRIT_FUZZING=ON ..;make
# /path/to/afl/afl-fuzz -i /path/to/corpus -o FINDINGS -m none -- libs/json/test/fuzzer

# create a version of the json library with the sanitizer enabled
# for use by the fuzzer executable in the test directory
add_library( json_fuzz STATIC ${SRC_FILES} )
set( FUZZ_TARGET_COMPILE_FLAGS "-fsanitize=${SANITIZER} -g -O1" )
set( FUZZ_TARGET_LINK_FLAGS    "-fsanitize=${SANITIZER}" )
if( SANITIZER STREQUAL "memory" )
  # requires Clang and a custom-built stdlib; for now we only support libc++
  set( FUZZ_TARGET_COMPILE_FLAGS "${FUZZ_TARGET_COMPILE_FLAGS} -fsanitize-memory-track-origins -stdlib=libc++" )
  set( FUZZ_TARGET_LINK_FLAGS    "${FUZZ_TARGET_LINK_FLAGS}    -fsanitize-memory-track-origins -stdlib=libc++" )
endif()

# depending on the compiler and sanitizer, we may need to
# add extra flags
if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
  # are we using the special AFL instrumented compiler?
  string(REGEX MATCH "afl-clang-fast\\+\\+$" AFL ${CMAKE_CXX_COMPILER})
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # or the gcc equivalent?
  string(REGEX MATCH "afl-g\\+\\+$" AFL ${CMAKE_CXX_COMPILER})
else()
  message( FATAL_ERROR "Fuzzing only works with gcc or Clang" )
endif()

if( AFL )
  if( SANITIZER STREQUAL "address" )
    # add "hardening options"
    set( FUZZ_TARGET_COMPILE_FLAGS "${FUZZ_TARGET_COMPILE_FLAGS} -D_FORTIFY_SOURCE=2 -fstack-protector-all" )
  elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
    message( FATAL_ERROR "gcc does not support memory sanitizing" )
  endif()
elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
  # assuming libFuzzer.  Add its coverage flags
  set( FUZZ_TARGET_COMPILE_FLAGS "${FUZZ_TARGET_COMPILE_FLAGS} -fsanitize-coverage=edge" )
  set( FUZZ_TARGET_LINK_FLAGS    "${FUZZ_TARGET_LINK_FLAGS}    -fsanitize-coverage=edge" )
else()
  message( FATAL_ERROR "libFuzzer is not supported under gcc at this time.  You can use AFL (asan only), though, if you set CMAKE_CXX_COMPILER to an afl-instrumented compiler" )
endif()

if( WIN32 AND NOT AFL )
  message( FATAL_ERROR "libFuzzer does not work under Windows, for now" )
endif()

set_target_properties( json_fuzz PROPERTIES COMPILE_FLAGS ${FUZZ_TARGET_COMPILE_FLAGS}
                                            LINK_FLAGS    ${FUZZ_TARGET_LINK_FLAGS} )

# produce the extra fuzzer target
if( AFL )
  add_executable( fuzzer fuzz_afl.cpp )
elseif( ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" ) AND LLVM_ROOT )
  # Build the libFuzzer-based fuzzer
  add_executable( fuzzer fuzz_lib.cpp )
  set_target_properties( fuzzer PROPERTIES COMPILE_FLAGS "-std=c++11 ${FUZZ_TARGET_COMPILE_FLAGS}" )

  # and the library it depends on
  file(GLOB LIBFUZZ_SOURCES ${LLVM_ROOT}/lib/Fuzzer/*.cpp)
  add_library( fuzz ${LIBFUZZ_SOURCES} )
  target_link_libraries( fuzzer fuzz )
  if ( SANITIZER STREQUAL "memory" )
    # For the memory sanitizer we must build libFuzzer with the sanitizer enabled
    set_target_properties( fuzz PROPERTIES COMPILE_FLAGS "-std=c++11 -stdlib=libc++ -fsanitize=memory -O3" )
  endif()

else()
  message( FATAL_ERROR "Don't know which fuzzer to build - afl compilers not in use and Clang+LLVM_ROOT not specified" )
endif()

target_link_libraries( fuzzer json_fuzz ${Boost_LOCALE_LIBRARY} )
set_target_properties( fuzzer PROPERTIES COMPILE_FLAGS ${FUZZ_TARGET_COMPILE_FLAGS}
                                           LINK_FLAGS    ${FUZZ_TARGET_LINK_FLAGS} )
