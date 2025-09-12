# -*- Makefile -*-
#
CFLAGS=-O3 -W -Wall -Wextra -Wshadow -Iinclude
CXXFLAGS += $(CFLAGS) -pedantic -std=c++11 -Isrc
LDFLAGS=-s

ifeq ($(COMP),)
	COMP=gcc
endif

ifeq ($(COMP),gcc)
	CC=gcc
	CXX=g++
endif

ifeq ($(COMP),clang)
	CC=clang
	CXX=clang++
endif

PROGRAMS = ./test/dstrtest ./test/dstrtest_pp ./test/test_dgets
DEPS = ./include/dstr/dstr.h
DEPS_PP = ./include/dstr/dstring.hpp ./include/dstr/dstr.h

all: $(PROGRAMS)

./test/dstrtest: ./src/dstr.o ./test/dstrtest.o
	$(CC) ./src/dstr.o ./test/dstrtest.o -o ./test/dstrtest $(LDFLAGS)

./test/dstrtest_pp: ./src/dstr.o ./test/dstrtest_pp.o
	$(CXX) ./src/dstr.o ./test/dstrtest_pp.o -o ./test/dstrtest_pp $(LDFLAGS)

./test/test_dgets: ./src/dstr.o ./test/test_dgets.o
	$(CXX) ./src/dstr.o ./test/test_dgets.o -o ./test/test_dgets $(LDFLAGS)

test: $(PROGRAMS)
	./test/dstrtest_pp
	./test/dstrtest
	man gcc 2>/dev/null > ./test/test_file.txt
	./test/test_dgets ./test/test_file.txt

test_various: ./test/dstr_test.sh
	cd ./test && ./dstr_test.sh

clean:
	rm -f ./test/*.o ./src/*.o ./test/test_file.txt $(PROGRAMS)

./src/dstr.o: ./src/dstr.c $(DEPS)
	$(CC) -c $(CFLAGS) -DNDEBUG -o $@ $<

./test/dstrtest.o: ./test/dstrtest.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $<

./test/dstrtest_pp.o: ./test/dstrtest_pp.cpp $(DEPS_PP)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

./test/test_dgets.o: ./test/test_dgets.cpp $(DEPS)
	$(CXX) -c $(CXXFLAGS) -o $@ $<
