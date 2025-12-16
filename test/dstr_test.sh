#!/bin/sh

set -e
NPROC=8

script_full_name=$(readlink -f $0)
script_dir=$(dirname $script_full_name)
pushd $script_dir

for COMP in gcc clang; do
	echo ">>>> VALGRIND ($COMP) TEST..."
	$COMP -march=x86-64-v3 -I../include -O0 -Og test_dstr.c ../src/dstr.c -o test_dstr
	valgrind --quiet ./test_dstr
	rm -f test_dstr
	echo ">>>> OK"
	echo

	echo ">>>> SANITZE ($COMP) TEST"
	$COMP -march=x86-64-v3 -I../include -fsanitize=address -O0 -Og test_dstr.c ../src/dstr.c -o test_dstr
	./test_dstr
	rm -f test_dstr
	echo ">>>> OK"
	echo
done

# Test with C++ compilation
#
CXXFLAGS="-march=x86-64-v3 -I../include -x c++ -std=c++11 -W -Wall -Wextra"
SRCFILES="test_dstring.cpp ../src/dstring.cpp ../src/dstr.c"

for COMP in g++ clang++; do
	echo ">>>> VALGRIND ($COMP) TEST..."
	$COMP $CXXFLAGS -O0 -Og $SRCFILES -o test_dstring
	valgrind --quiet ./test_dstring
	rm -f test_dstring
	echo ">>>> OK"
	echo

	echo ">>>> SANITZE ($COMP) TEST"
	$COMP $CXXFLAGS -fsanitize=address -O0 -Og $SRCFILES -o test_dstring
	./test_dstring
	rm -f test_dstring
	echo ">>>> OK"
	echo
done

popd

# test build of mingw32
#
make clean
make -j$NPROC COMP=mingw32

# test build of mingw64
#
make clean
make -j$NPROC COMP=mingw64

# test built and run with CLANG
#
make clean
make -j$NPROC COMP=clang test

# test built and run with CLANG and sanitize
#
make clean
make -j$NPROC COMP=clang SANITIZE=1 test

# test built and run with gcc15
#
make clean
gcc15.env make -j$NPROC test

# test built and run with gcc15 and sanitize
#
make clean
gcc15.env make -j$NPROC SANITIZE=1 test

# test build and run with default gcc and sanitize
#
make clean
make -j$NPROC SANITIZE=1 test

# test and run with default gcc - ready for installation
#
make clean
make -j$NPROC test
