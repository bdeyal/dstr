
all:
	bcc32 -w- -O2 -I. -P -I..\..\include ..\..\test\test_dstr.c ..\..\src\dstr.c
	bcc32 -w- -O2 -DNO_DSTRING_REGEX -I. -P -I..\..\include ..\..\test\test_dstring.cpp ..\..\src\dstr.c ..\..\src\dstring.cpp

test:
	test_dstr.exe
	test_dstring.exe

clean:
	del *.obj *.exe *.tds *.ilk *~
