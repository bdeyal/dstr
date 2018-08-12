# top level sconscript file -*- Python -*-

platform = str(Platform())

CC = ARGUMENTS.get('compiler', "gcc")

# Compile as C++ for better type checks
#
if platform == 'win32':
   #FLAGS='-nologo -W2 -Od -Zi -MDd -W2'
   FLAGS='-nologo -W2 -Ox -MD -DNDEBUG'
   #LIBRARIES=[]
else:
   #FLAGS='-x c++ -Wall -O0 -g -DGC_DEBUG -fno-rtti -fno-exceptions'
   #FLAGS='-x c++ -Wall -g -O0 -fprofile-arcs -ftest-coverage -fno-rtti -fno-exceptions'
   #FLAGS='-x c++ -Wall -O2 -fno-rtti -fno-exceptions'
   #FLAGS='-x c -std=c89 -Wall -g -O0 -fprofile-arcs -ftest-coverage'
   FLAGS='-Wall -O2 -DNDEBUG -D_GNU_SOURCE'
   #LIBRARIES=['gcov']
   LIBRARIES=[]
   #LIBRARIES=['gc']

Library('dstr' ,
        CC=CC,
        CFLAGS=Split(FLAGS),
        LIBS=Split(LIBRARIES),
        source = ['dstr.c'])

Program ( target = 'dstrtest',
          source = ['dstr_test.cpp'],
          LIBPATH = ['.'],
          LIBS=['dstr'],
          CXX=CC
        )
