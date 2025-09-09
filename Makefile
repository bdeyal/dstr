# -*- Makefile -*-
#
CFLAGS=-O3 -W -Wall -Wextra -Wshadow
CXXFLAGS += $(CFLAGS) -pedantic -std=c++11
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

PROGRAMS = dstrtest dstrtest_pp test_dgets
DEPS = dstr.h
DEPS_PP = dstring.hpp dstr.h

all: $(PROGRAMS)

dstrtest: dstr.o dstrtest.o
	$(CC) dstr.o dstrtest.o -o dstrtest $(LDFLAGS)

dstrtest_pp: dstr.o dstrtest_pp.o
	$(CXX) dstr.o dstrtest_pp.o -o dstrtest_pp $(LDFLAGS)

test_dgets: dstr.o test_dgets.o
	$(CXX) dstr.o test_dgets.o -o test_dgets $(LDFLAGS)

test: $(PROGRAMS)
	./dstrtest_pp
	./dstrtest
	man gcc 2>/dev/null > test_file.txt
	./test_dgets test_file.txt

test_various: dstr_test.sh
	./dstr_test.sh

clean:
	rm -f *.o test_file.txt $(PROGRAMS)

dstr.o: dstr.c $(DEPS)
	$(CC) -c $(CFLAGS) -DNDEBUG -o $@ $<

dstrtest.o: dstrtest.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $<

dstrtest_pp.o: dstrtest_pp.cpp $(DEPS_PP)
	$(CXX) -c $(CXXFLAGS) -o $@ $<
