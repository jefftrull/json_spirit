/**
 *  Copyright (C) 2015 Jeff Trull <edaskel@att.net>
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

// Use the LLVM fuzzing library libFuzzer to try to find bugs

#include <string>
extern int fuzz_onecase(std::string const&);

// the one function you need to provide
extern "C" int LLVMFuzzerTestOneInput(const unsigned char *data, unsigned long size) {

   std::string s((const char*)data, size);

   return fuzz_onecase(s);
}
