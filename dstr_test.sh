#!/bin/sh

set -e

for CXX in g++ clang++; do
	echo ">>>> VALGRIND ($CXX) TEST..."
	$CXX -O0 -Og dstr_test.cpp -x c dstr.c -o dstrtest
	valgrind --quiet ./dstrtest > /dev/null
	rm -f dstrtest
	echo ">>>> OK"
	echo

	echo ">>>> SANITZE ($CXX) TEST"
	$CXX -fsanitize=address -O0 -Og dstr_test.cpp -x c dstr.c -o dstrtest
	./dstrtest	 > /dev/null
	rm -f dstrtest
	echo ">>>> OK"
	echo

	echo ">>>> GC ($CXX) TEST"
	$CXX -DGC_DEBUG -O0 -Og dstr_test.cpp -x c dstr.c -o dstrtest -lgc
	./dstrtest	 > /dev/null
	rm -f dstrtest
	echo ">>>> GC OK"
	echo
done
