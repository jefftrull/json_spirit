/**
 *  Copyright (C) 2015 Jeff Trull <edaskel@att.net>
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

// "handle one input" standalone program for AFL
// very similar to the libFuzzer version

#include <string>
#include <iostream>
extern int fuzz_onecase(std::string const&);

int main() {
    // stream std::cin into a string
    std::string s((std::istreambuf_iterator<char>(std::cin)),
                  (std::istreambuf_iterator<char>()));
    return fuzz_onecase(s);
}
