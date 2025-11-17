
REM To find PCRE2 DLL files
REM
@PATH=C:\PCRE2\bin\%Platform%;%PATH%

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

REM  borland build and test
REM
nmake -f NMakefile clean
nmake -f NMakefile COMP=borland
nmake -f NMakefile COMP=borland test

REM  Clean all
REM
nmake -f NMakefile clean
