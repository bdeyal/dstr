#!/bin/sh

set -e

for COMP in gcc clang; do
	echo ">>>> VALGRIND ($COMP) TEST..."
	$COMP -O0 -Og dstrtest.c dstr.c -o dstrtest
	valgrind --quiet ./dstrtest > /dev/null
	rm -f dstrtest
	echo ">>>> OK"
	echo

	echo ">>>> SANITZE ($COMP) TEST"
	$COMP -fsanitize=address -O0 -Og dstrtest.c dstr.c -o dstrtest
	./dstrtest > /dev/null
	rm -f dstrtest
	echo ">>>> OK"
	echo

	echo ">>>> GC ($COMP) TEST"
	$COMP -DGC_DEBUG -O0 -Og dstrtest.c dstr.c -o dstrtest -lgc
	./dstrtest > /dev/null
	rm -f dstrtest
	echo ">>>> GC OK"
	echo
done


# Test with C++ compilation (No modern C++)
#
CXXFLAGS="-x c++ -std=c++98 -W -Wall -Wextra"

for COMP in g++ clang++; do
	echo ">>>> VALGRIND ($COMP) TEST..."
	$COMP $CXXFLAGS -O0 -Og dstrtest_pp.cpp dstr.c -o dstrtest
	valgrind --quiet ./dstrtest > /dev/null
	rm -f dstrtest
	echo ">>>> OK"
	echo

	echo ">>>> SANITZE ($COMP) TEST"
	$COMP $CXXFLAGS -fsanitize=address -O0 -Og dstrtest_pp.cpp dstr.c -o dstrtest
	./dstrtest > /dev/null
	rm -f dstrtest
	echo ">>>> OK"
	echo

	echo ">>>> GC ($COMP) TEST"
	$COMP $CXXFLAGS -DGC_DEBUG -O0 -Og dstrtest_pp.cpp dstr.c -o dstrtest -lgc
	./dstrtest > /dev/null
	rm -f dstrtest
	echo ">>>> GC OK"
	echo
done
