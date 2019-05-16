# top level sconscript file -*- Python -*-

plat = ARGUMENTS.get('OS', Platform())
platform = str(plat)

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
   #LAGS='-x c -Wall -g -Os -D_GNU_SOURCE'
   FLAGS='-Wall -Wextra -Ofast -DNDEBUG  -D_GNU_SOURCE'
   #LIBRARIES=['gcov']
   LIBRARIES=[]
   #LIBRARIES=['gc']

Library('dstr' ,
        CC='gcc',
        CFLAGS=Split(FLAGS),
        LIBS=Split(LIBRARIES),
        source = ['dstr.c'])

Program ( target = 'dstrtest',
          source = ['dstr_test.cpp'],
          LIBPATH = ['.'],
          LIBS=['dstr'],
          CXX='gcc'
        )
