
all:
	bcc32 -w- -O2 -I. -P -I..\..\include ..\..\test\dstrtest.c ..\..\src\dstr.c
	bcc32 -w- -O2 -I. -P -I..\..\include ..\..\test\dstrtest_pp.cpp ..\..\src\dstr.c ..\..\src\dstring.cpp

test:
	dstrtest.exe
	dstrtest_pp.exe

clean:
	del *.obj *.exe *.tds *.ilk *~
