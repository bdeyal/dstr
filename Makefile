# -*- Makefile -*-
#
PREFIX=/usr/local
ARCH=x86-64-v3

ifeq ($(COMP),)
	COMP=gcc
endif

ifeq ($(COMP),gcc)
	CC=gcc
	CXX=g++
endif

ifeq ($(COMP),gcc15)
	CC=/opt/gcc-15.2/bin/gcc
	CXX=/opt/gcc-15.2/bin/g++
endif

ifeq ($(COMP),mingw64)
	CC=x86_64-w64-mingw32-gcc
	CXX=x86_64-w64-mingw32-g++
	LDFLAGS += -static
	CFLAGS=-DPCRE2_STATIC
endif

ifeq ($(COMP),mingw32)
	CC=i686-w64-mingw32-gcc
	CXX=i686-w64-mingw32-g++
	LDFLAGS += -static
	ARCH=core2
	CFLAGS=-m32 -msse2 -mssse3 -msse4.1 -msse4.2 -mfpmath=sse -DPCRE2_STATIC
endif

ifeq ($(COMP),clang)
	CC=clang
	CXX=clang++
endif

CFLAGS +=-O3 -march=$(ARCH) -W -Wall -Wextra -Wshadow -Iinclude -flto=auto -ffat-lto-objects
CXXFLAGS += $(CFLAGS) -pedantic -std=c++17
LDFLAGS += -L./lib64 -s

PROGRAMS = \
	./test/dstrtest \
	./test/dstrtest_pp \
	./test/test_hash \
	./test/test_dgets \
	./test/test_map \
	./test/test_vector \
	./test/test_tokens \
	./test/test_algs \
	./test/test_regex \
	./test/test_view

DEPS = ./include/dstr/dstr.h
DEPS_PP = ./include/dstr/dstring.hpp ./include/dstr/dstring_view.hpp ./include/dstr/dstr.h
LIB=./lib64/libdstr.a

all: $(PROGRAMS)

./test/dstrtest: ./test/dstrtest.c $(LIB) $(DEPS)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/dstrtest_pp: ./test/dstrtest_pp.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_dgets: ./test/test_dgets.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_map: ./test/test_map.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) $(NO_UNINIT) -o $@ $< $(LDFLAGS) -ldstr

./test/test_vector: ./test/test_vector.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_tokens: ./test/test_tokens.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_algs: ./test/test_algs.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_hash: ./test/test_hash.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_regex: ./test/test_regex.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr -lpcre2-8

./test/test_view: ./test/test_view.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

$(LIB): ./src/dstring_regex.cpp ./src/dstring.cpp ./src/dstr.c $(DEPS_PP) Makefile
	mkdir -p ./lib64
	$(CC) -c $(CFLAGS) -DNDEBUG ./src/dstr.c -o ./src/dstr.o
	$(CXX) -c $(CXXFLAGS) -DNDEBUG ./src/dstring.cpp -o ./src/dstring.o
	$(CXX) -c $(CXXFLAGS) -DNDEBUG ./src/dstring_regex.cpp -o ./src/dstring_regex.o
	$(AR) rcs $(LIB) ./src/dstr.o ./src/dstring.o ./src/dstring_regex.o

.PHONY: testall
testall: test testvg test_various

.PHONY: test
test: $(PROGRAMS) ./test/test_file.txt
	./test/dstrtest_pp
	./test/dstrtest
	./test/test_dgets ./test/test_file.txt
	./test/test_map
	./test/test_vector
	./test/test_tokens
	./test/test_algs
	./test/test_regex
	./test/test_hash
	./test/test_view

.PHONY: testvg
testvg: $(PROGRAMS) ./test/test_file.txt
	valgrind ./test/dstrtest
	valgrind ./test/dstrtest_pp
	valgrind ./test/test_dgets ./test/test_file.txt
	valgrind ./test/test_map
	valgrind ./test/test_vector
	valgrind ./test/test_tokens
	valgrind ./test/test_algs
	valgrind ./test/test_regex
	valgrind ./test/test_hash
	valgrind ./test/test_view


./test/test_file.txt:
	man gcc 2>/dev/null > ./test/test_file.txt

.PHONY: test_various
test_various: ./test/dstr_test.sh
	cd ./test && ./dstr_test.sh | tee test_various.txt

clean:
	rm -rf ./lib64
	rm -f ./src/dstr.o ./src/dstring.o ./src/dstring_regex.o
	rm -f ./test/test_file.txt ./test/test_various.txt $(PROGRAMS)
	rm -f ./test/*.exe

PREFIX_INCLUDE=$(PREFIX)/include/dstr
PREFIX_LIB=$(PREFIX)/lib64

# Tested on RHEL 10.0 ONLY
#
install: $(LIB)
	/usr/bin/mkdir -p $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 -o root -g root include/dstr/dstr.h $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 -o root -g root include/dstr/dstring.hpp $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 -o root -g root include/dstr/dstring_view.hpp $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 -o root -g root include/dstr/dstr_regex_fwd.h $(PREFIX_INCLUDE)
	/usr/bin/mkdir -p $(PREFIX_LIB)
	/usr/bin/install -m 0644 -o root -g root $(LIB) -t $(PREFIX_LIB)

uninstall:
	/usr/bin/rm -f $(PREFIX_INCLUDE)/dstr.h
	/usr/bin/rm -f $(PREFIX_INCLUDE)/dstring.hpp
	/usr/bin/rm -f $(PREFIX_INCLUDE)/dstring_view.hpp
	/usr/bin/rm -f $(PREFIX_INCLUDE)/dstr_regex_fwd.h
	/usr/bin/rmdir $(PREFIX_INCLUDE)
	/usr/bin/rm -f $(PREFIX_LIB)/libdstr.a
	/usr/bin/rmdir --ignore-fail-on-non-empty $(PREFIX_LIB)
