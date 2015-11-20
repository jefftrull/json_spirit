## Instructions for building and running fuzzers on json_spirit

### What is "fuzzing" and why should we do it?
Fuzzing is a technique for finding bugs first popularized in the security research community.  It involves using massive amounts of random inputs to try to find vulnerable code.  Recently tools have been developed that use branch coverage information to better direct the random inputs.  This approach greatly reduces the amount of time required to find a problem.  Two popular implementations of this approach are LLVM's [libFuzzer](http://llvm.org/docs/LibFuzzer.html) and the standalone tool [American Fuzzy Lop](http://lcamtuf.coredump.cx/afl).

### Prerequisites (Ubuntu 15.10)
* Boost 1.59 or later, built suitably for your compiler and sanitizer choices
* If using Clang, a fresh build installed in your PATH and LD_LIBRARY_PATH
* The package **afl-clang** if you want to use AFL

It is easy and fun to fuzz **json_spirit**.  Defining `JSON_SPIRIT_FUZZING` to cmake will enable fuzzing targets.  For a given build you just have to choose:

1. Which fuzzing system to use: AFL or libFuzzer
  * using an AFL compiler wrapper (such as `afl-clang-fast++` or `afl-g++`) selects AFL mode
  * using Clang and setting `LLVM_ROOT` selects libFuzzer
2. Which sanitizer to enable: **address** (catches out-of-range accesses) or **memory** (finds initialization problems)
  * Setting `SANITIZER` selects which sanitizer to enable ("address" or "memory")

### To use AFL (American Fuzzy Lop)
Building fuzzers for AFL requires the use of a special compiler wrapper, such as `afl-clang-fast++`.  It inserts a special pass in Clang/LLVM that instruments the code appropriately.  If you supply this special wrapper as the value of `CMAKE_CXX_COMPILER`, AFL mode will be enabled.  Using `afl-g++` will enable a similar but slower method via gcc.

#### Examples
* Address sanitizing with AFL and gcc
```
cmake -DCMAKE_CXX_COMPILER=afl-g++ -DJSON_SPIRIT_FUZZING=ON -DSANITIZER=address -DBOOST_ROOT=/path/to/boost_1_59_0  ..
make fuzzer
afl-fuzz -i CORPUS -o FINDINGS -m none -- fuzzing/fuzzer
```
* Address sanitizing with AFL and Clang
```
cmake -DCMAKE_CXX_COMPILER=afl-clang-fast++ -DJSON_SPIRIT_FUZZING=ON -DSANITIZER=address -DBOOST_ROOT=/path/to/boost_1_59_0_clang ..
make fuzzer
afl-fuzz -i CORPUS -o FINDINGS -m none -- fuzzing/fuzzer
```
### To use LLVM's libFuzzer
Specifying `LLVM_ROOT` to cmake, with Clang as the compiler, will enable this mode.  Point it to your checked-out llvm that contains libFuzzer.

#### Example
* Address sanitizing with libFuzzer
```
cmake -DCMAKE_CXX_COMPILER=clang++ -DJSON_SPIRIT_FUZZING=ON -DSANITIZER=address -DLLVM_ROOT=/path/to/oss/llvm -DBOOST_ROOT=/path/to/boost_1_59_0_clang ..
make fuzzer
fuzzing/fuzzer ./CORPUS
```

### Special instructions for the memory sanitizer
Doing a memory sanitizer build requires that library dependencies such as the C++ standard library, as well as any other high-level C++ libraries, be compiled with memory sanitizing.  In the case of **json_spirit**, this means Boost.Locale, ICU (a dependency of Boost.Locale), and libc++.  You provide paths to these on the cmake command line with the values `BOOST_ROOT`, `ICU_MSAN_PATH`, and `LIBCXX_MSAN_PATH`.  For more information, see [the Sanitizer FAQ](https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo).

### Examples
* Memory sanitizing with AFL and Clang
```
cmake -DCMAKE_CXX_COMPILER=afl-clang-fast++ -DJSON_SPIRIT_FUZZING=ON -DSANITIZER=memory -DBOOST_ROOT=/path/to/boost_1_59_0_clang_msan -DICU_MSAN_PATH=/path/to/icu-56-install -DLIBCXX_MSAN_PATH=/path/to/llvm_msan/build ..
make fuzzer
afl-fuzz -i CORPUS -o FINDINGS -m none -- fuzzing/fuzzer
```
* Memory sanitizing with libFuzzer
```
cmake -DCMAKE_CXX_COMPILER=clang++ -DJSON_SPIRIT_FUZZING=ON -DSANITIZER=memory -DLLVM_ROOT=/path/to/oss/llvm -DBOOST_ROOT=/path/to/boost_1_59_0_clang_msan -DICU_MSAN_PATH=/path/to/icu-56-install -DLIBCXX_MSAN_PATH=/path/to/oss/llvm_msan/build ..
make fuzzer
fuzzing/fuzzer ./CORPUS
```

### Final Note
At this time (November 2015), there is a Spirit-related issue in handling non-ASCII characters that impacts **json_spirit**.  If you see an assertion failure related to `isascii()` that is probably the cause.  Take a look at [the Spirit development branch](https://github.com/boostorg/spirit) for the latest updates.
