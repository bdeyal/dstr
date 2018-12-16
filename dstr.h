/*
 * dstr.h
 */
#ifndef DSTR_H_INCLUDED
#define DSTR_H_INCLUDED

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "dstr_impl.h"

/*
 *  Dynamic string type. See 'dstr_impl.h'
 */
typedef struct DSTR_IMP* DSTR;

/*--------------------------------------------------------------------------*/

#define DSTR_NPOS        ((size_t)(-1))
#define DSTR_NULL        NULL
#define DSTR_IS_NULL(p)  ((p) == NULL)
#define DSTR_BOOL        bool
#define DSTR_TRUE        true
#define DSTR_FALSE       false
#define DSTR_SUCCESS     DSTR_TRUE
#define DSTR_FAIL        DSTR_FALSE

#ifdef __cplusplus
extern "C" {
#endif

/* create an empty DSTR*/
DSTR dstr_create(void);

/* create an empty DSTR with at least LEN reserved storage*/
DSTR dstr_create_reserve(size_t len);

/* from one or more chars*/
DSTR dstr_create_cc(char c, size_t count);

/* create from a c-string*/
DSTR dstr_create_sz(const char* sz);

/* create from another DSTR*/
DSTR dstr_create_ds(const DSTR rhs);

/* create from a substr of another DSTR*/
DSTR dstr_create_substr(const DSTR p, size_t pos, size_t count);

/* create from a buffer and length*/
DSTR dstr_create_bl(const char* buff, size_t len);

/* TODO: create from textfile*/
/*DSTR dstr_create_fromfile(const char* fname);*/

/* create from a [v]sprintf like call*/
DSTR dstr_create_sprintf(const char* fmt, ...);
DSTR dstr_create_vsprintf(const char* fmt, va_list argptr);

/* destroy a DSTR object*/
void  dstr_destroy(DSTR p);

/* transfer data to c string, destory object, caller must free result
   if PLEN is not NULL, write length to it.
 */
char* dstr_move_destory(DSTR p, size_t* plen);

/*
 *  Assign | Insert | Append | Replace
 *  a DSTR from various sources. returns DSTR_SUCCESS or DSTR_FAIL
 */
int  dstr_assign_cc(DSTR dest, char c, size_t count);
int  dstr_assign_sz(DSTR dest, const char* value);
int  dstr_assign_ds(DSTR dest, const DSTR src);
int  dstr_assign_bl(DSTR dest, const char* buff, size_t len);
int  dstr_assign_substr(DSTR dest, const DSTR p, size_t pos, size_t count);
int  dstr_assign_sprintf(DSTR dest, const char* fmt, ...);
int  dstr_assign_vsprintf(DSTR dest, const char* fmt, va_list argptr);

int  dstr_insert_cc(DSTR dest, size_t index, char c, size_t count);
int  dstr_insert_sz(DSTR dest, size_t index, const char* value);
int  dstr_insert_ds(DSTR dest, size_t index, const DSTR src);
int  dstr_insert_bl(DSTR dest, size_t index, const char* buff, size_t len);

int  dstr_append_c(DSTR dest, char c);
int  dstr_append_cc(DSTR dest, char c, size_t count);
int  dstr_append_sz(DSTR dest, const char* value);
int  dstr_append_ds(DSTR dest, const DSTR src);

int  dstr_append_bl(DSTR dest, const char* buff, size_t len);
int  dstr_append_sprintf(DSTR dest, const char* fmt, ...);
int  dstr_append_vsprintf(DSTR dest, const char* fmt, va_list argptr);

int  dstr_replace_cc(DSTR dest, size_t pos, size_t len, char c, size_t count);
int  dstr_replace_sz(DSTR dest, size_t pos, size_t len, const char* value);
int  dstr_replace_ds(DSTR dest, size_t pos, size_t len, const DSTR src);
int  dstr_replace_bl(DSTR dest, size_t pos, size_t len, const char* buff, size_t buflen);

/* either truncate to LEN or enlarge capacity to at least LEN without
 * changing content */
int  dstr_resize(DSTR dest, size_t len);

/* Following operations manipulate data but no allocation / free
 */
void  dstr_ascii_upper(DSTR p);
void  dstr_ascii_lower(DSTR p);
void  dstr_swap(DSTR d1, DSTR d2);
void  dstr_reverse(DSTR p);
void  dstr_trim_right(DSTR p);
void  dstr_trim_left(DSTR p);
void  dstr_trim_both(DSTR p);
void  dstr_truncate(DSTR p);
void  dstr_remove(DSTR p, size_t pos, size_t count);

/*
 * trivial operations implemented inline. see "dstr_inlines.h"
 */
static __inline const char*  dstr_cstring(const DSTR p);
static __inline size_t       dstr_length(const DSTR p);
static __inline size_t       dstr_capacity(const DSTR p);
static __inline DSTR_BOOL    dstr_isempty(const DSTR p);
static __inline void         dstr_chop(DSTR p);

/* negative indexes are from the end */
static __inline DSTR_BOOL    dstr_valid_index(const DSTR p, size_t pos);
static __inline char         dstr_getchar(const DSTR p, size_t pos);
static __inline void         dstr_putchar(DSTR p, size_t pos, char c);

/*
 * 'safe' won't read or write beyond dstr limits.
 * Negetiv index are from the end. i.e last character is an pos -1
 */
static __inline char         dstr_getchar_safe(const DSTR p, long pos);
static __inline void         dstr_putchar_safe(DSTR p, long pos, char c);

/* 31 bit hash value */
unsigned int dstr_hash(const DSTR src);

/*  atoi integer conversion
 *
 *  '0b110111' => binary
 *  '0xABCD12' => hex
 *  '\324'     => octal
 *  '123'      => decimal
 *  '0123'     => decimal !
 */
long      dstr_atoi(const DSTR src);
long      dstr_atoll(const DSTR src);
int       dstr_itoa(DSTR dest, long n);
DSTR_BOOL dstr_isdigits(const DSTR src);
DSTR_BOOL dstr_isxdigits(const DSTR src);

/* find s in p. returns index or DSTR_NPOS if not found*/
size_t dstr_find_c(const DSTR p, size_t pos, char c);
size_t dstr_find_sz(const DSTR p, size_t pos, const char* s);
size_t dstr_ifind_c(const DSTR p, size_t pos, char c);
size_t dstr_ifind_sz(const DSTR p, size_t pos, const char* s);

DSTR_BOOL dstr_contains_sz(const DSTR p, const char* s);
DSTR_BOOL dstr_icontains_sz(const DSTR p, const char* s);
DSTR_BOOL dstr_isblank(const DSTR p);

/* p has suffix s, 'endswith' */
DSTR_BOOL dstr_suffix_sz(const DSTR p, const char* s);
DSTR_BOOL dstr_isuffix_sz(const DSTR p, const char* s);

/* p has prefix s, 'startswith' */
DSTR_BOOL dstr_prefix_sz(const DSTR p, const char* s);
DSTR_BOOL dstr_iprefix_sz(const DSTR p, const char* s);

/*  ffo  = find_first of, ffno = find first not of */
size_t dstr_ffo_sz(const DSTR p, size_t pos, const char* s);
size_t dstr_ffo_ds(const DSTR p, size_t pos, const DSTR s);
size_t dstr_ffno_sz(const DSTR p, size_t pos, const char* s);
size_t dstr_ffno_ds(const DSTR p, size_t pos, const DSTR s);

/* copy NUMBYTES from INDEX to DEST. returns copied characters*/
size_t dstr_substr(const DSTR p, size_t index, size_t numbytes, char dest[], size_t destsize);

/* 3way strcmp-like comparison*/
int dstr_compare_sz(const DSTR lhs, const char* sz);
int dstr_compare_ds(const DSTR lhs, const DSTR rhs);

/* is equal*/
DSTR_BOOL dstr_equal_sz(const DSTR lhs, const char* sz);
DSTR_BOOL dstr_iequal_sz(const DSTR lhs, const char* sz);
DSTR_BOOL dstr_equal_ds(const DSTR lhs, const DSTR rhs);

/* I/O functions*/
int dstr_fgets(DSTR d, FILE* fp);
int dstr_fgetline(DSTR d, FILE* fp);


#ifdef __cplusplus
}
#endif

/* implement trivial operations inline */
#include "dstr_inlines.h"

/* Include alternate names for the above functions */
#include "dstr_altnames.h"

#endif /* DSTR_H_ */
