
REM  MSVC build and test
REM
nmake -f NMakefile clean
nmake -f NMakefile
nmake -f NMakefile test

REM  clang build and test
REM
nmake -f NMakefile clean
nmake -f NMakefile COMP=clang
nmake -f NMakefile COMP=clang test

REM  Clean all
REM
nmake -f NMakefile distclean
