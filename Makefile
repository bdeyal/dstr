# -*- Makefile -*-
#
PREFIX=/usr/local
ARCH=x86-64-v3

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
endif

ifeq ($(COMP),gcc)
	CC=gcc
	CXX=g++
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
	AR=llvm-ar
	LDFLAGS=-fuse-ld=lld
endif

CFLAGS +=-O2 -g $(SAN) -pthread -march=$(ARCH) -W -Wall -Wextra -Wshadow -Iinclude -flto=auto -ffat-lto-objects
LDFLAGS += -L./lib64

ifeq ($(OS),FreeBSD)
LDFLAGS += -L/usr/local/lib -lstdthreads
CFLAGS += -I/usr/local/include
endif

CXXFLAGS += $(CFLAGS) -pthread -pedantic -std=c++20

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

all: $(PROGRAMS)

./test/test_dstr: ./test/test_dstr.c $(LIB) $(DEPS)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_dstring: ./test/test_dstring.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/test_dstring_regex: ./test/test_dstring_regex.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr -lpcre2-8

./test/test_dstr_regex: ./test/test_dstr_regex.c   $(LIB) $(DEPS_PP)
	$(CC) $(CFLAGS) -o $@ ./test/test_dstr_regex.c $(LDFLAGS) -ldstr -lpcre2-8

./test/test_dstringview: ./test/test_dstringview.cpp $(LIB) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr

$(LIB): $(LIB_O)
	mkdir -p ./lib64
	$(AR) rcs $(LIB) $(LIB_O)

./src/dstr.o: ./src/dstr.c $(DEPS)
	$(CC) -c $(CFLAGS) -DNDEBUG -o $@ $<

./src/dstr_regex.o: ./src/dstr_regex.c $(DEPS)
	$(CC) -c $(CFLAGS) -DNDEBUG -o $@ $<

./src/dstring.o: ./src/dstring.cpp $(DEPS_PP)
	$(CXX) -c $(CXXFLAGS) -DNDEBUG -o $@ $<

./src/dstring_regex.o: ./src/dstring_regex.cpp $(DEPS_PP)
	$(CXX) -c $(CXXFLAGS) -DNDEBUG -o $@ $<

.PHONY: testall
testall: test_various testvg

.PHONY: test
test: $(PROGRAMS)
	./test/test_dstring
	./test/test_dstr
	./test/test_dstring_regex
	./test/test_dstr_regex
	./test/test_dstringview

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
	/usr/bin/mkdir -p $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 -o root -g root include/dstr/dstr.h $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 -o root -g root include/dstr/dstring.hpp $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 -o root -g root include/dstr/dstringstream.hpp $(PREFIX_INCLUDE)
	/usr/bin/mkdir -p $(PREFIX_LIB)
	/usr/bin/install -m 0644 -o root -g root $(LIB) -t $(PREFIX_LIB)

uninstall:
	/usr/bin/rm -f $(PREFIX_INCLUDE)/dstr.h
	/usr/bin/rm -f $(PREFIX_INCLUDE)/dstring.hpp
	/usr/bin/rm -f $(PREFIX_INCLUDE)/dstringstream.hpp
	/usr/bin/rmdir $(PREFIX_INCLUDE)
	/usr/bin/rm -f $(PREFIX_LIB)/libdstr.a
	/usr/bin/rmdir --ignore-fail-on-non-empty $(PREFIX_LIB)
