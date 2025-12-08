#ifndef DSTR_STDBOOL_INCLUDED
#define DSTR_STDBOOL_INCLUDED

#include <ctype.h>

// Borland C++ 5.x missing stdbool.h
// Added it here in addition to some missing stuff in BCC 5.x on 32 bit code
// Compilation is in C++ mode only (since dstr.c is C99 and above)
//
#if !defined(__cplusplus)
#error BCC5.x C++ mode only!
#endif

#define strtoll  strtol
#define strtoull strtoul
#define strtof   strtod
#define strtold  strtod

#define va_copy(a, b)  ((a) = (b))

#define _stricmp  stricmp
#define _strnicmp strnicmp

#if __BORLANDC__ <= 0x0560
static inline int isblank(int c) { return isspace(c); }
#endif

#define nullptr (NULL)
#define noexcept

#define SKIP_DSTRINGSTREAM

#endif
