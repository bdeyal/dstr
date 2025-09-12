# -*- Makefile -*-
#
CFLAGS=-O3 -W -Wall -Wextra -Wshadow -Iinclude
CXXFLAGS += $(CFLAGS) -pedantic -std=c++11 -Isrc
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

PROGRAMS = ./test/dstrtest ./test/dstrtest_pp ./test/test_dgets
DEPS = ./include/dstr/dstr.h
DEPS_PP = ./include/dstr/dstring.hpp ./include/dstr/dstr.h
LIB=./lib64/libdstr.so

all: $(PROGRAMS)

./test/dstrtest: $(LIB) ./test/dstrtest.o
	$(CC) ./test/dstrtest.o -o ./test/dstrtest $(LDFLAGS) -ldstr

./test/dstrtest_pp: $(LIB) ./test/dstrtest_pp.o
	$(CXX) ./test/dstrtest_pp.o -o ./test/dstrtest_pp $(LDFLAGS) -ldstr

./test/test_dgets: $(LIB) ./test/test_dgets.o
	$(CXX) ./test/test_dgets.o -o ./test/test_dgets $(LDFLAGS) -ldstr

$(LIB): ./src/dstr.o
	mkdir -p ./lib64
	gcc -shared -fPIC ./src/dstr.o -o ./lib64/libdstr.so.1.0.0
	ln -sf libdstr.so.1.0.0 ./lib64/libdstr.so.1
	ln -sf libdstr.so.1.0.0 ./lib64/libdstr.so

.PHONY: test
test: $(PROGRAMS) ./test/test_file.txt
	./test/dstrtest_pp
	./test/dstrtest
	./test/test_dgets ./test/test_file.txt

./test/test_file.txt:
	man gcc 2>/dev/null > ./test/test_file.txt

.PHONY: test_various
test_various: ./test/dstr_test.sh
	cd ./test && ./dstr_test.sh

clean:
	rm -rf ./lib64
	rm -f ./test/*.o ./src/*.o ./test/test_file.txt $(PROGRAMS)

./src/dstr.o: ./src/dstr.c $(DEPS)
	$(CC) -c $(CFLAGS) -fPIC -DPIC -DNDEBUG -o $@ $<

./test/dstrtest.o: ./test/dstrtest.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $<

./test/dstrtest_pp.o: ./test/dstrtest_pp.cpp $(DEPS_PP)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

./test/test_dgets.o: ./test/test_dgets.cpp $(DEPS)
	$(CXX) -c $(CXXFLAGS) -o $@ $<
