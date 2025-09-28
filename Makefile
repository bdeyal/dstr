# -*- Makefile -*-
#
PREFIX=/usr/local
ARCH=x86-64-v3

CFLAGS=-O3 -march=$(ARCH) -W -Wall -Wextra -Wshadow -Iinclude
CXXFLAGS += $(CFLAGS) -pedantic -std=c++11
LDFLAGS=-L./lib64 -s

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

PROGRAMS = \
	./test/dstrtest \
	./test/dstrtest_pp \
	./test/test_dgets \
	./test/test_map \
	./test/test_vector \
	./test/test_tokens \
	./test/test_algs

DEPS = ./include/dstr/dstr.h
DEPS_PP = ./include/dstr/dstring.hpp ./include/dstr/dstr.h
LIB=./lib64/libdstr.a

all: $(PROGRAMS)

./test/dstrtest: ./test/dstrtest.c $(LIB) $(DEPS)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/dstrtest_pp: ./test/dstrtest_pp.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_dgets: ./test/test_dgets.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_map: ./test/test_map.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_vector: ./test/test_vector.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_tokens: ./test/test_tokens.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_algs: ./test/test_algs.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

$(LIB): ./src/dstring.cpp ./src/dstr.c $(DEPS_PP) Makefile
	mkdir -p ./lib64
	$(CC) -c $(CFLAGS) -DNDEBUG ./src/dstr.c -o ./src/dstr.o
	$(CXX) -c $(CXXFLAGS) -DNDEBUG ./src/dstring.cpp -o ./src/dstring.o
	$(AR) rcs $(LIB) ./src/dstr.o ./src/dstring.o

.PHONY: test
test: $(PROGRAMS) ./test/test_file.txt
	./test/dstrtest_pp
	./test/dstrtest
	./test/test_dgets ./test/test_file.txt
	./test/test_map
	./test/test_vector
	./test/test_tokens
	./test/test_algs

.PHONY: testvg
testvg: $(PROGRAMS) ./test/test_file.txt
	valgrind ./test/dstrtest
	valgrind ./test/dstrtest_pp
	valgrind ./test/test_dgets ./test/test_file.txt
	valgrind ./test/test_map
	valgrind ./test/test_vector
	valgrind ./test/test_tokens
	valgrind ./test/test_algs

./test/test_file.txt:
	man gcc 2>/dev/null > ./test/test_file.txt

.PHONY: test_various
test_various: ./test/dstr_test.sh
	cd ./test && ./dstr_test.sh

clean:
	rm -rf ./lib64
	rm -f ./src/dstr.o ./src/dstring.o
	rm -f ./test/test_file.txt $(PROGRAMS)

PREFIX_INCLUDE=$(PREFIX)/include/dstr
PREFIX_LIB=$(PREFIX)/lib64

# Tested on RHEL 10.0 ONLY
#
install: $(LIB)
	/usr/bin/mkdir -p $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 -o root -g root include/dstr/dstr.h $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 -o root -g root include/dstr/dstring.hpp $(PREFIX_INCLUDE)
	/usr/bin/mkdir -p $(PREFIX_LIB)
	/usr/bin/install -m 0644 -o root -g root $(LIB) -t $(PREFIX_LIB)

uninstall:
	/usr/bin/rm -f $(PREFIX_INCLUDE)/dstr.h
	/usr/bin/rm -f $(PREFIX_INCLUDE)/dstring.hpp
	/usr/bin/rmdir $(PREFIX_INCLUDE)
	/usr/bin/rm -f $(PREFIX_LIB)/libdstr.a
	/usr/bin/rmdir --ignore-fail-on-non-empty $(PREFIX_LIB)
