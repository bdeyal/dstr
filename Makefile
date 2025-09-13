# -*- Makefile -*-
#
CFLAGS=-O3 -W -Wall -Wextra -Wshadow -Iinclude
CXXFLAGS += $(CFLAGS) -pedantic -std=c++11
LDFLAGS=-L./lib64 -Wl,-rpath,./lib64 -s

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
	./test/test_tokens

DEPS = ./include/dstr/dstr.h
DEPS_PP = ./include/dstr/dstring.hpp ./include/dstr/dstr.h
LIB=./lib64/libdstr.so

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

$(LIB): ./src/dstr.c $(DEPS) Makefile
	mkdir -p ./lib64
	gcc $(CFLAGS) -shared -fPIC ./src/dstr.c -o ./lib64/libdstr.so.1.0.0
	ln -sf libdstr.so.1.0.0 ./lib64/libdstr.so.1
	ln -sf libdstr.so.1.0.0 ./lib64/libdstr.so

.PHONY: test
test: $(PROGRAMS) ./test/test_file.txt
	./test/dstrtest_pp
	./test/dstrtest
	./test/test_dgets ./test/test_file.txt
	./test/test_map
	./test/test_vector
	./test/test_tokens

.PHONY: testvg
testvg: $(PROGRAMS) ./test/test_file.txt
	valgrind ./test/dstrtest_pp
	valgrind ./test/dstrtest
	valgrind ./test/test_dgets ./test/test_file.txt
	valgrind ./test/test_map
	valgrind ./test/test_vector
	valgrind ./test/test_tokens

./test/test_file.txt:
	man gcc 2>/dev/null > ./test/test_file.txt

.PHONY: test_various
test_various: ./test/dstr_test.sh
	cd ./test && ./dstr_test.sh

clean:
	rm -rf ./lib64
	rm -f ./test/test_file.txt $(PROGRAMS)
