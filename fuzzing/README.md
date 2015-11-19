## Instructions for building and running fuzzers on json_spirit

It is easy and fun to fuzz **json_spirit**.  Defining `JSON_SPIRIT_FUZZING` to cmake will enable fuzzing targets.  For a given build you just have to choose:

1. Which fuzzing system to use: AFL or libFuzzer
2. Which sanitizer to enable: **address** (catches out-of-range accesses) or **memory** (finds initialization problems)

The `SANITIZER` definition to cmake selects which sanitizer to enable ("address" or "memory") and sets up the compile options appropriately.

### To use AFL (American Fuzzy Lop)
Building fuzzers for AFL requires the use of a special compiler wrapper, `afl-clang-fast++`.  It inserts a special pass in Clang/LLVM that instruments the code appropriately.  If you supply this special wrapper as the value of `CMAKE_CXX_COMPILER`, AFL mode will be enabled.  Using `afl-g++` will enable a similar but slower method via gcc.

### To use LLVM's libFuzzer
Specifying `LLVM_ROOT` to cmake, with Clang as the compiler, will enable this mode.  Point it to your checked-out llvm that contains libFuzzer.

### Special instructions for the memory sanitizer
Doing a memory sanitizer build requires that library dependencies such as the C++ standard library, as well as any other high-level C++ libraries, be compiled with memory sanitizing.  In the case of **json_spirit**, this means Boost.Locale, ICU (a dependency of Boost.Locale), and libc++.  You provide paths to these on the cmake command line with the values `BOOST_ROOT`, `ICU_MSAN_PATH`, and `LIBCXX_MSAN_PATH`.  For more information, see [the Sanitizer FAQ](https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo).

### Examples
(All tested on Ubuntu 15.10)
* Address sanitizing with AFL and gcc
```
cmake -DCMAKE_CXX_COMPILER=afl-g++ -DBOOST_ROOT=/path/to/boost_1_59_0 -DJSON_SPIRIT_FUZZING=ON ..
make fuzzer
afl-fuzz -i CORPUS -o FINDINGS -m none -- fuzzing/fuzzer
```
* Address sanitizing with AFL and Clang
```
cmake -DCMAKE_CXX_COMPILER=afl-clang-fast++ -DBOOST_ROOT=/path/to/boost_1_59_0_clang -DJSON_SPIRIT_FUZZING=ON -DSANITIZER=address ..
make fuzzer
afl-fuzz -i CORPUS -o FINDINGS -m none -- fuzzing/fuzzer
```
* Memory sanitizing with AFL and Clang
```
cmake -DCMAKE_CXX_COMPILER=afl-clang-fast++ -DBOOST_ROOT=/path/to/boost_1_59_0_clang_msan -DJSON_SPIRIT_FUZZING=ON -DSANITIZER=memory -DICU_MSAN_PATH=/path/to/icu-56-install -DLIBCXX_MSAN_PATH=/path/to/llvm_msan/build ..
make fuzzer
afl-fuzz -i CORPUS -o FINDINGS -m none -- fuzzing/fuzzer
```
* Address sanitizing with libFuzzer
```
cmake -DCMAKE_CXX_COMPILER=clang++ -DBOOST_ROOT=/path/to/boost_1_59_0_clang -DJSON_SPIRIT_FUZZING=ON -DLLVM_ROOT=/path/to/oss/llvm -DSANITIZER=address ..
make fuzzer
fuzzing/fuzzer ./CORPUS
```
* Memory sanitizing with libFuzzer
```
cmake -DCMAKE_CXX_COMPILER=clang++ -DBOOST_ROOT=/path/to/boost_1_59_0_clang_msan -DJSON_SPIRIT_FUZZING=ON -DSANITIZER=memory -DLLVM_ROOT=/path/to/oss/llvm -DICU_MSAN_PATH=/path/to/icu-56-install -DLIBCXX_MSAN_PATH=/path/to/oss/llvm_msan/build ..
make fuzzer
fuzzing/fuzzer ./CORPUS
```
