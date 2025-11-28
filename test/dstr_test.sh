#!/bin/sh

set -e
NPROC=8

script_full_name=$(readlink -f $0)
script_dir=$(dirname $script_full_name)
pushd $script_dir

for COMP in gcc clang; do
	echo ">>>> VALGRIND ($COMP) TEST..."
	$COMP -march=x86-64-v3 -I../include -O0 -Og dstrtest.c ../src/dstr.c -o dstrtest
	valgrind --quiet ./dstrtest
	rm -f dstrtest
	echo ">>>> OK"
	echo

	echo ">>>> SANITZE ($COMP) TEST"
	$COMP -march=x86-64-v3 -I../include -fsanitize=address -O0 -Og dstrtest.c ../src/dstr.c -o dstrtest
	./dstrtest
	rm -f dstrtest
	echo ">>>> OK"
	echo

	echo ">>>> GC ($COMP) TEST"
	$COMP -march=x86-64-v3 -I../include -DGC_DEBUG -O0 -Og dstrtest.c ../src/dstr.c -o dstrtest -lgc
	./dstrtest
	rm -f dstrtest
	echo ">>>> GC OK"
	echo
done

# Test with C++ compilation
#
CXXFLAGS="-march=x86-64-v3 -I../include -x c++ -std=c++11 -W -Wall -Wextra"
SRCFILES="dstrtest_pp.cpp ../src/dstring.cpp ../src/dstr.c"

for COMP in g++ clang++; do
	echo ">>>> VALGRIND ($COMP) TEST..."
	$COMP $CXXFLAGS -O0 -Og $SRCFILES -o dstrtest
	valgrind --quiet ./dstrtest
	rm -f dstrtest
	echo ">>>> OK"
	echo

	echo ">>>> SANITZE ($COMP) TEST"
	$COMP $CXXFLAGS -fsanitize=address -O0 -Og $SRCFILES -o dstrtest
	./dstrtest
	rm -f dstrtest
	echo ">>>> OK"
	echo

	echo ">>>> GC ($COMP) TEST"
	$COMP $CXXFLAGS -DGC_DEBUG -O0 -Og $SRCFILES -o dstrtest -lgc
	./dstrtest
	rm -f dstrtest
	echo ">>>> GC OK"
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
