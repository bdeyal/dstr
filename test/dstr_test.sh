#!/bin/sh

set -e

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
