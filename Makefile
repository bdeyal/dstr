# -*- Makefile -*-
#
MKDIR=/bin/mkdir -p
PREFIX=/usr/local
ARCH ?= x86-64-v3

OS := $(shell uname)

ifeq ($(COMP),)
	COMP=cc
endif

ifeq ($(SANITIZE),1)
	SAN=-fsanitize=address
endif

ifeq ($(COMP),cc)
	CC=cc
	CXX=c++
	AR=ar
endif

ifeq ($(COMP),gcc)
	CC=gcc
	CXX=g++
	AR=ar
endif

ifeq ($(COMP),mingw64)
	CC=x86_64-w64-mingw32-gcc
	CXX=x86_64-w64-mingw32-g++
	AR=x86_64-w64-mingw32-ar
	LDFLAGS += -static
	CFLAGS=-DPCRE2_STATIC
endif

ifeq ($(COMP),mingw32)
	CC=i686-w64-mingw32-gcc
	CXX=i686-w64-mingw32-g++
	AR=i686-w64-mingw32-ar
	LDFLAGS += -static
	ARCH=core2
	CFLAGS=-m32 -msse2 -mssse3 -msse4.1 -msse4.2 -mfpmath=sse -DPCRE2_STATIC
endif

ifeq ($(COMP),clang)
	CC=clang
	CXX=clang++
	AR=llvm-ar
	LDFLAGS=-fuse-ld=lld
endif

CFLAGS +=-O2 $(SAN) -pthread -march=$(ARCH) -W -Wall -Wextra -Wshadow -Iinclude -flto=auto -ffat-lto-objects
LDFLAGS += -L./lib64

ifeq ($(OS),FreeBSD)
LDFLAGS += -L/usr/local/lib -lstdthreads
CFLAGS += -I/usr/local/include
endif

CXXFLAGS += $(CFLAGS) -pedantic -std=c++20

# Object files for libdstr.a
#
LIB_O = \
	./src/dstr.o \
	./src/dstr_regex.o \
	./src/dstring.o \
	./src/dstring_regex.o

PROGRAMS = \
	./test/test_dstr \
	./test/test_dstring \
	./test/test_dstr_regex \
	./test/test_dstring_regex \
	./test/test_dstringview

DEPS = ./include/dstr/dstr.h Makefile
DEPS_PP = \
	./include/dstr/dstring.hpp \
	./include/dstr/dstringstream.hpp \
	$(DEPS)

LIB=./lib64/libdstr.a

.DEFAULT_GOAL := all

all: $(PROGRAMS)

./test/test_dstr: ./test/test_dstr.c $(LIB) $(DEPS)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_dstring: ./test/test_dstring.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_dstring_regex: ./test/test_dstring_regex.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr -lpcre2-8

./test/test_dstr_regex: ./test/test_dstr_regex.c   $(LIB) $(DEPS)
	$(CC) $(CFLAGS) -o $@ ./test/test_dstr_regex.c $(LDFLAGS) -ldstr -lpcre2-8

./test/test_dstringview: ./test/test_dstringview.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

$(LIB): $(LIB_O)
	$(MKDIR) ./lib64
	$(AR) rcs $(LIB) $(LIB_O)

./src/dstr.o: ./src/dstr.c $(DEPS)
	$(CC) -c $(CFLAGS) -DNDEBUG -o $@ $<

./src/dstr_regex.o: ./src/dstr_regex.c $(DEPS)
	$(CC) -c $(CFLAGS) -DNDEBUG -o $@ $<

./src/dstring.o: ./src/dstring.cpp $(DEPS_PP)
	$(CXX) -c $(CXXFLAGS) -DNDEBUG -o $@ $<

./src/dstring_regex.o: ./src/dstring_regex.cpp $(DEPS_PP)
	$(CXX) -c $(CXXFLAGS) -DNDEBUG -o $@ $<

.PHONY: test
test: $(PROGRAMS)
	./test/test_dstring
	./test/test_dstr
	./test/test_dstring_regex
	./test/test_dstr_regex
	./test/test_dstringview

.PHONY: check
check: test

.PHONY: testvg
testvg: $(PROGRAMS)
	valgrind ./test/test_dstr
	valgrind ./test/test_dstring
	valgrind ./test/test_dstring_regex
	valgrind ./test/test_dstr_regex
	valgrind ./test/test_dstringview

.PHONY: test_various
test_various: ./test/dstr_test.sh
	./test/dstr_test.sh | tee ./test/test_various.txt

clean:
	rm -rf ./lib64
	rm -f ./test/*.o ./src/*.o
	rm -f ./test/test_various.txt $(PROGRAMS)
	rm -f ./test/*.exe

PREFIX_INCLUDE=$(PREFIX)/include/dstr
PREFIX_LIB=$(PREFIX)/lib64

# Tested on RHEL 10.0 ONLY
#
install: $(LIB)
	$(MKDIR) $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 include/dstr/dstr.h $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 include/dstr/dstring.hpp $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 include/dstr/dstringstream.hpp $(PREFIX_INCLUDE)
	$(MKDIR) $(PREFIX_LIB)
	/usr/bin/install -m 0644 $(LIB) $(PREFIX_LIB)

uninstall:
	/bin/rm -f $(PREFIX_INCLUDE)/dstr.h
	/bin/rm -f $(PREFIX_INCLUDE)/dstring.hpp
	/bin/rm -f $(PREFIX_INCLUDE)/dstringstream.hpp
	/bin/rmdir $(PREFIX_INCLUDE) 2>/dev/null || true
	/bin/rm -f $(PREFIX_LIB)/libdstr.a
	/bin/rmdir $(PREFIX_LIB) 2>/dev/null || true
