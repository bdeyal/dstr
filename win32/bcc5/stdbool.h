#ifndef DSTR_STDBOOL_INCLUDED
#define DSTR_STDBOOL_INCLUDED

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
#define strnlen   my_strnlen

static inline size_t my_strnlen(const char* s, size_t maxlen) {
	const char *p = (char*) memchr(s, 0, maxlen);
	return p ? p - s : maxlen;
}

static inline int isblank(int c) { return isspace(c); }

#define nullptr ((void*)0)
#define noexcept

#endif
