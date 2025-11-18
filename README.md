# Copyright
Copyright (C) 2025 Eyal Ben-David (bdeyal@gmail.com).
This code is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3, or (at your option) any later version. See LICENSE file

# dstr
Dynamic C strings, similar to C++ std::string with a lean C++ wrapper. This is a fast and safe Python and Ruby inspired string library for C and C++. It is SSO-optimized and has no dependencies except for optional PCRE2 regex in the C++ part. This library could be used as a replacement for std::string in text handling code, with the benefit of many methods borrowed from Python and Ruby. See usage examples in the test file under 'test' folder.

# Build
Just run make
On Windows run nmake -f NMakefile

# Clean
make clean OR
nmake -f NMakefile clean

# Test
## Linux
make test OR
nmake -f NMakefile test
