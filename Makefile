# -*- Makefile -*-
#
PREFIX=/usr/local
ARCH=x86-64-v3

LIBDIR_FULLPATH=$(shell readlink -f ./lib64)
CFLAGS=-O3 -march=$(ARCH) -W -Wall -Wextra -Wshadow -Iinclude
CXXFLAGS += $(CFLAGS) -pedantic -std=c++11
LDFLAGS=-L$(LIBDIR_FULLPATH) -Wl,-rpath,$(LIBDIR_FULLPATH) -s
LDFLAGS_STATIC=-L$(LIBDIR_FULLPATH) -s

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
	./test/test_algs \
	./test/dstrtest_s \
	./test/dstrtest_pp_s \
	./test/test_dgets_s \
	./test/test_map_s \
	./test/test_vector_s \
	./test/test_tokens_s \
	./test/test_algs_s

DEPS = ./include/dstr/dstr.h
DEPS_PP = ./include/dstr/dstring.hpp ./include/dstr/dstr.h
LIB=./lib64/libdstr.so.1.0.0
LIBPP=./lib64/libdstr++.so.1.0.0
LIB_S=./lib64/libdstr_static.a

all: $(PROGRAMS)

./test/dstrtest: ./test/dstrtest.c $(LIB) $(DEPS)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -ldstr

./test/dstrtest_pp: ./test/dstrtest_pp.cpp $(LIBPP) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr++ -ldstr

./test/test_dgets: ./test/test_dgets.cpp $(LIBPP) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr++ -ldstr

./test/test_map: ./test/test_map.cpp $(LIBPP) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr++ -ldstr

./test/test_vector: ./test/test_vector.cpp $(LIBPP) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr++ -ldstr

./test/test_tokens: ./test/test_tokens.cpp $(LIBPP) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr++ -ldstr

./test/test_algs: ./test/test_algs.cpp $(LIBPP) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -ldstr++ -ldstr

# linking with static library (.a)
#
./test/dstrtest_s: ./test/dstrtest.c $(LIB_S) $(DEPS)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_STATIC) -ldstr_static

./test/dstrtest_pp_s: ./test/dstrtest_pp.cpp $(LIB_S) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS_STATIC) -ldstr_static

./test/test_dgets_s: ./test/test_dgets.cpp $(LIB_S) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS_STATIC) -ldstr_static

./test/test_map_s: ./test/test_map.cpp $(LIB_S) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS_STATIC) -ldstr_static

./test/test_vector_s: ./test/test_vector.cpp $(LIB_S) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS_STATIC) -ldstr_static

./test/test_tokens_s: ./test/test_tokens.cpp $(LIB_S) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS_STATIC) -ldstr_static

./test/test_algs_s: ./test/test_algs.cpp $(LIB_S) $(DEPS_PP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS_STATIC) -ldstr_static


$(LIB): ./src/dstr.c $(DEPS) Makefile
	mkdir -p ./lib64
	$(CC) $(CFLAGS) -DNDEBUG -shared -DPIC -fPIC ./src/dstr.c -o ./lib64/libdstr.so.1.0.0
	ln -sf libdstr.so.1.0.0 ./lib64/libdstr.so.1
	ln -sf libdstr.so.1.0.0 ./lib64/libdstr.so

$(LIBPP): ./src/dstring.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -DNDEBUG -shared \
-DPIC -fPIC ./src/dstring.cpp \
-L$(LIBDIR_FULLPATH) -o $(LIBPP) -ldstr
	ln -sf libdstr++.so.1.0.0 ./lib64/libdstr++.so.1
	ln -sf libdstr++.so.1.0.0 ./lib64/libdstr++.so

$(LIB_S): ./src/dstring.cpp ./src/dstr.c
	$(CC) -c $(CFLAGS) -DNDEBUG ./src/dstr.c -o ./src/dstr.o
	$(CXX) -c $(CXXFLAGS) -DNDEBUG ./src/dstring.cpp -o ./src/dstring.o
	$(AR) rcs $(LIB_S) ./src/dstr.o ./src/dstring.o

.PHONY: test
test: $(PROGRAMS) ./test/test_file.txt
	./test/dstrtest_pp
	./test/dstrtest
	./test/test_dgets ./test/test_file.txt
	./test/test_map
	./test/test_vector
	./test/test_tokens
	./test/test_algs
	./test/dstrtest_pp_s
	./test/dstrtest_s
	./test/test_dgets_s ./test/test_file.txt
	./test/test_map_s
	./test/test_vector_s
	./test/test_tokens_s
	./test/test_algs_s

.PHONY: testvg
testvg: $(PROGRAMS) ./test/test_file.txt
	valgrind ./test/dstrtest
	valgrind ./test/dstrtest_pp
	valgrind ./test/test_dgets ./test/test_file.txt
	valgrind ./test/test_map
	valgrind ./test/test_vector
	valgrind ./test/test_tokens
	valgrind ./test/test_algs
	valgrind ./test/dstrtest_pp_s
	valgrind ./test/dstrtest_s
	valgrind ./test/test_dgets_s ./test/test_file.txt
	valgrind ./test/test_map_s
	valgrind ./test/test_vector_s
	valgrind ./test/test_tokens_s
	valgrind ./test/test_algs_s

./test/test_file.txt:
	man gcc 2>/dev/null > ./test/test_file.txt

.PHONY: test_various
test_various: ./test/dstr_test.sh
	cd ./test && ./dstr_test.sh

clean:
	rm -rf ./lib64
	rm -f ./test/test_file.txt $(PROGRAMS)


PREFIX_INCLUDE=$(PREFIX)/include/dstr
PREFIX_LIB=$(PREFIX)/lib64/dstr

# Tested on RHEL 10.0 ONLY
#
install: $(LIB)
	/usr/bin/mkdir -p $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 -o root -g root include/dstr/dstr.h $(PREFIX_INCLUDE)
	/usr/bin/install -m 0644 -o root -g root include/dstr/dstring.hpp $(PREFIX_INCLUDE)
	/usr/bin/mkdir -p $(PREFIX_LIB)
	/usr/bin/install -m 0755 -o root -g root --strip $(LIB) -t $(PREFIX_LIB)
	/usr/bin/ln -sf libdstr.so.1.0.0  $(PREFIX_LIB)/libdstr.so.1
	/usr/bin/ln -sf libdstr.so.1.0.0  $(PREFIX_LIB)/libdstr.so
	/usr/bin/echo $(PREFIX_LIB) > /etc/ld.so.conf.d/dstr.conf
	/usr/bin/install -m 0755 -o root -g root --strip $(LIBPP) -t $(PREFIX_LIB)
	/usr/bin/ln -sf libdstr++.so.1.0.0  $(PREFIX_LIB)/libdstr++.so.1
	/usr/bin/ln -sf libdstr++.so.1.0.0  $(PREFIX_LIB)/libdstr++.so
	/usr/bin/install -m 0644 -o root -g root $(LIB_S) -t $(PREFIX_LIB)
	/usr/bin/echo $(PREFIX_LIB) > /etc/ld.so.conf.d/dstr.conf
	/usr/sbin/ldconfig

uninstall:
	/usr/bin/rm -f $(PREFIX_INCLUDE)/dstr.h
	/usr/bin/rm -f $(PREFIX_INCLUDE)/dstring.hpp
	/usr/bin/rmdir $(PREFIX_INCLUDE)
	/usr/bin/rm -f $(PREFIX_LIB)/libdstr.so
	/usr/bin/rm -f $(PREFIX_LIB)/libdstr.so.1
	/usr/bin/rm -f $(PREFIX_LIB)/libdstr.so.1.0.0
	/usr/bin/rm -f $(PREFIX_LIB)/libdstr++.so
	/usr/bin/rm -f $(PREFIX_LIB)/libdstr++.so.1
	/usr/bin/rm -f $(PREFIX_LIB)/libdstr++.so.1.0.0
	/usr/bin/rm -f $(PREFIX_LIB)/libdstr_static.a
	/usr/bin/rmdir --ignore-fail-on-non-empty $(PREFIX_LIB)
	/usr/bin/rm -f /etc/ld.so.conf.d/dstr.conf
	/usr/sbin/ldconfig
