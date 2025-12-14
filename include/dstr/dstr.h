/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#ifndef DSTR_H_INCLUDED
#define DSTR_H_INCLUDED

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#if defined(_WIN32)
#define strcasecmp  _stricmp
#define strncasecmp _strnicmp
#endif

// Test for 32 vs 64 bit build
//
#if UINTPTR_MAX > UINT32_MAX
#define DSTR_64BIT
#define DSTR_INITIAL_CAPACITY (32U) //(48U)
#else
#define DSTR_32BIT
#define DSTR_INITIAL_CAPACITY (16U) //(20U)
#endif


// First two members in DSTR_VIEW and DSTR_TYPE must be identical
// in type and order.
//
typedef struct DSTR_VIEW
{
    const char* data;
    uint32_t    length;
#ifdef DSTR_64BIT
    uint32_t    capacity; // unused padding
#endif
} DSTR_VIEW;
/*--------------------------------------------------------------------------*/

typedef struct DSTR_TYPE
{
    char*    data;
    uint32_t length;
    uint32_t capacity;
    char     sso_buffer[DSTR_INITIAL_CAPACITY];
} DSTR_TYPE;
/*--------------------------------------------------------------------------*/

typedef       struct DSTR_TYPE* DSTR;
typedef const struct DSTR_TYPE* CDSTR;
/*--------------------------------------------------------------------------*/

#define DSTR_NPOS        ((size_t)(-1))
#define DSTR_BOOL        bool
#define DSTR_TRUE        true
#define DSTR_FALSE       false
#define DSTR_SUCCESS     1
#define DSTR_FAIL        0

#ifdef __cplusplus
extern "C" {
#endif

/* create an empty DSTR*/
DSTR dstr_create_empty(void);

/* create an empty DSTR with at least LEN reserved storage*/
DSTR dstr_create_reserve(size_t len);

/* from one or more chars*/
DSTR dstr_create_cc(char c, size_t count);

/* create from a c-string*/
DSTR dstr_create_sz(const char* sz);

/* create from another DSTR*/
DSTR dstr_create_ds(CDSTR rhs);

/* create from a substr of another DSTR*/
DSTR dstr_create_substr(CDSTR p, size_t pos, size_t count);

/* create from a buffer and length*/
DSTR dstr_create_bl(const char* buff, size_t len);

/* create from first->last charecter range */
DSTR dstr_create_range(const char* first, const char* last);

/* create by slurp a textfile*/
DSTR dstr_create_fromfile(const char* fname);
int dstr_assign_fromfile(DSTR p, const char* fname);
int dstr_slurp_stream(DSTR p, FILE* fp);

/* create from a [v]sprintf like call*/
DSTR dstr_create_sprintf(const char* fmt, ...);
DSTR dstr_create_vsprintf(const char* fmt, va_list argptr);

/* destroy a DSTR object*/
void dstr_destroy(DSTR p);
void dstr_clean_data(DSTR p);

/*
 *   Assign | Insert | Append | Replace
 *   a DSTR from various sources. returns DSTR_SUCCESS or DSTR_FAIL
 */
int dstr_assign_cc(DSTR dest, char c, size_t count);
int dstr_assign_sz(DSTR dest, const char* value);
int dstr_assign_ds(DSTR dest, CDSTR src);
int dstr_assign_bl(DSTR dest, const char* buff, size_t len);
int dstr_assign_range(DSTR dest, const char* first, const char* last);
int dstr_assign_substr(DSTR dest, CDSTR p, size_t pos, size_t count);
int dstr_assign_sprintf(DSTR dest, const char* fmt, ...);
int dstr_assign_vsprintf(DSTR dest, const char* fmt, va_list argptr);

int dstr_insert_cc(DSTR dest, size_t index, char c, size_t count);
int dstr_insert_sz(DSTR dest, size_t index, const char* value);
int dstr_insert_ds(DSTR dest, size_t index, CDSTR src);
int dstr_insert_bl(DSTR dest, size_t index, const char* buff, size_t len);
int dstr_insert_range(DSTR dest, size_t index, const char* first, const char* last);

int dstr_grow_append_c(DSTR dest, char c);
int dstr_append_cc(DSTR dest, char c, size_t count);
int dstr_append_sz(DSTR dest, const char* value);
int dstr_append_ds(DSTR dest, CDSTR src);
int dstr_append_bl(DSTR dest, const char* buff, size_t len);
int dstr_append_range(DSTR dest, const char* first, const char* last);
int dstr_append_sprintf(DSTR dest, const char* fmt, ...);
int dstr_append_vsprintf(DSTR dest, const char* fmt, va_list argptr);

int dstr_replace_cc(DSTR dest, size_t pos, size_t len, char c, size_t count);
int dstr_replace_sz(DSTR dest, size_t pos, size_t len, const char* value);
int dstr_replace_ds(DSTR dest, size_t pos, size_t len, CDSTR src);
int dstr_replace_bl(DSTR dest, size_t pos, size_t len, const char* buff, size_t buflen);
int dstr_replace_range(DSTR dest, size_t pos, size_t len, const char* first, const char* last);
#define DSTR_REPLACE_ALL DSTR_NPOS
int dstr_replace_all_sz(DSTR dest, const char* oldstr, const char* newstr, size_t count);
int dstr_replace_all_ds(DSTR dest, CDSTR oldstr, CDSTR newstr, size_t count);

/* join ARGV into DEST with separator SEP */
int dstr_join_sz(DSTR dest, const char* sep, const char* argv[], size_t n);
int dstr_join_ds(DSTR dest, CDSTR sep, const char* argv[], size_t n);

/* duplicate self n times like $s x 5 in perl */
int dstr_multiply(DSTR dest, size_t n);

/* replace COUNT occurrences of old with new */
int dstr_expand_tabs(DSTR dest, size_t width);
int dstr_zfill(DSTR dest, size_t width);

/* expand original DEST to be at center of WIDTH characters with fill FILL */
int dstr_align_center(DSTR dest, size_t width, char fill);
int dstr_align_right(DSTR dest, size_t width, char fill);
int dstr_align_left(DSTR dest, size_t width, char fill);

/*
 *  either truncate to LEN or enlarge capacity to at least LEN without
 *  changing content
 */
int dstr_resize(DSTR dest, size_t len);
int dstr_reserve(DSTR p, size_t len);
int dstr_shrink_to_fit(DSTR d);

/*
 *  Following operations manipulate data but no allocation / free
 */
void dstr_ascii_upper(DSTR p);
void dstr_ascii_lower(DSTR p);
void dstr_ascii_swapcase(DSTR p);
void dstr_title(DSTR p);
void dstr_swap(DSTR d1, DSTR d2);
void dstr_reverse(DSTR p);
void dstr_trim_right(DSTR p);
void dstr_trim_left(DSTR p);
void dstr_trim(DSTR p);

/*
 *  Remove functions
 */
void dstr_remove(DSTR p, size_t pos, size_t count);
void dstr_remove_char(DSTR p, char c);
void dstr_remove_any(DSTR p, const char* selectors);
void dstr_remove_prefix(DSTR p, const char* s);
void dstr_remove_suffix(DSTR p, const char* s);
void dstr_iremove_prefix(DSTR p, const char* s);
void dstr_iremove_suffix(DSTR p, const char* s);

/*
 *  Like trim but strip a special char from edges
 */
void dstr_lstrip_c(DSTR p, char c);
void dstr_rstrip_c(DSTR p, char c);

/*
 *  Like above but strip any of sz from edges
 */
void dstr_lstrip_sz(DSTR p, const char* selectors);
void dstr_rstrip_sz(DSTR p, const char* selectors);

/* hash for use in hash table */
size_t dstr_hash(CDSTR src, int seed /*=0*/);

/*   atoi integer conversion
 *
 *   '0b110111' => binary
 *   '0xABCD12' => hex
 *   '\324'     => octal
 *   '123'      => decimal
 *   '0123'     => decimal !
 */
long      dstr_atoi(CDSTR src);
long      dstr_atoll(CDSTR src);
double    dstr_atof(CDSTR src);

/* itos = int to string */
int dstr_itos_ul(DSTR dest, unsigned long long n, unsigned int base);
int dstr_itos(DSTR dest, long long n);

/* find s in p. returns index or DSTR_NPOS if not found*/
size_t dstr_find_c(CDSTR p, size_t pos, char c);
size_t dstr_find_sz(CDSTR p, size_t pos, const char* s);
size_t dstr_ifind_c(CDSTR p, size_t pos, char c);
size_t dstr_ifind_sz(CDSTR p, size_t pos, const char* s);

/* Reverse find s in p. returns index or DSTR_NPOS if not found */
size_t dstr_rfind_c(CDSTR p, size_t pos, char c);
size_t dstr_rfind_sz(CDSTR p, size_t pos, const char* s);
size_t dstr_irfind_c(CDSTR p, size_t pos, char c);
size_t dstr_irfind_sz(CDSTR p, size_t pos, const char* s);

/* count non-overlapping occurrences of s in p */
size_t dstr_count_sz(CDSTR p, const char* s);
size_t dstr_count_ds(CDSTR p, CDSTR s);
size_t dstr_icount_sz(CDSTR p, const char* s);
size_t dstr_icount_ds(CDSTR p, CDSTR s);

DSTR_BOOL dstr_contains_sz(CDSTR p, const char* s);
DSTR_BOOL dstr_icontains_sz(CDSTR p, const char* s);

DSTR_BOOL dstr_isblank(CDSTR p);
DSTR_BOOL dstr_isdigits(CDSTR p);
DSTR_BOOL dstr_isxdigits(CDSTR p);
DSTR_BOOL dstr_isalnum(CDSTR p);
DSTR_BOOL dstr_isalpha(CDSTR p);
DSTR_BOOL dstr_isascii(CDSTR p);
DSTR_BOOL dstr_isdecimal(CDSTR p);
DSTR_BOOL dstr_isidentifier(CDSTR p);
DSTR_BOOL dstr_islower(CDSTR p);
DSTR_BOOL dstr_isnumeric(CDSTR p);
DSTR_BOOL dstr_isprintable(CDSTR p);
DSTR_BOOL dstr_isspace(CDSTR p);
DSTR_BOOL dstr_istitle(CDSTR p);
DSTR_BOOL dstr_isupper(CDSTR p);

/* p has suffix s, 'endswith' */
DSTR_BOOL dstr_suffix_sz(CDSTR p, const char* s);
DSTR_BOOL dstr_isuffix_sz(CDSTR p, const char* s);

/* p has prefix s, 'startswith' */
DSTR_BOOL dstr_prefix_sz(CDSTR p, const char* s);
DSTR_BOOL dstr_iprefix_sz(CDSTR p, const char* s);

/* ffo = find_first of, ffno = find first not of */
size_t dstr_ffo_sz(CDSTR p, size_t pos, const char* selectors);
size_t dstr_ffo_ds(CDSTR p, size_t pos, CDSTR selectors);
size_t dstr_ffno_sz(CDSTR p, size_t pos, const char* selectors);
size_t dstr_ffno_ds(CDSTR p, size_t pos, CDSTR selectors);

/* flo = find_last of, flno = find last not of */
size_t dstr_flo_sz(CDSTR p, size_t pos, const char* selectors);
size_t dstr_flo_ds(CDSTR p, size_t pos, CDSTR selectors);
size_t dstr_flno_sz(CDSTR p, size_t pos, const char* selectors);
size_t dstr_flno_ds(CDSTR p, size_t pos, CDSTR selectors);

/* copy NUMBYTES from INDEX to DEST. returns copied characters*/
size_t dstr_substr(CDSTR p, size_t index, size_t numbytes, char dest[], size_t destsize);

/* I/O functions*/
int dstr_fgets(DSTR d, FILE* fp);
int dstr_fgetline(DSTR d, FILE* fp);

/* translate and squeeze */
void dstr_translate(DSTR dest, const char* arr1, const char* arr2);
void dstr_squeeze(DSTR dest, const char* squeeze);
void dstr_translate_squeeze(DSTR dest, const char* arr1, const char* arr2);

/* similar to ruby's .succ function */
int dstr_increment(DSTR dest);

struct DSTR_PartInfo {
    // l, m, r = left, mid, right
    size_t l_pos;
    size_t l_len;
    size_t m_pos;
    size_t m_len;
    size_t r_pos;
    size_t r_len;
};
size_t dstr_partition(CDSTR p,  const char* s, struct DSTR_PartInfo* pInfo);
size_t dstr_rpartition(CDSTR p, const char* s, struct DSTR_PartInfo* pInfo);

/*
 * conversion funcions
 */
int                dstr_to_int(CDSTR p, size_t* index, int base);
long               dstr_to_long(CDSTR p, size_t* index, int base);
unsigned long      dstr_to_ulong(CDSTR p, size_t* index, int base);
long long          dstr_to_llong(CDSTR p, size_t* index, int base);
unsigned long long dstr_to_ullong(CDSTR p, size_t* index, int base);
float              dstr_to_float(CDSTR p, size_t* index);
double             dstr_to_double(CDSTR p, size_t* index);
long double        dstr_to_ldouble(CDSTR p, size_t* index);

#if !defined(NO_DSTRING_REGEX)
/*
 *  Regexp functions
 */
typedef struct DSTR_Regex_Match {
    // zero based offset (DSTR_NPOS for no match)
    size_t offset;

    // length of substring
    size_t length;

    // name of group
    DSTR name;
} DSTR_Regex_Match;

// Must call free on matches field
//
typedef struct DSTR_Match_Vector {
    size_t            matches_len;
    DSTR_Regex_Match* matches;
} DSTR_Match_Vector;

void   dstr_regex_mvector_free(DSTR_Match_Vector* pVec);
void   dstr_regex_perror(int rc);
void   dstr_regex_strerror(DSTR p, int rc);
bool   dstr_regex_exact(CDSTR p, const char* pattern, size_t offset);
size_t dstr_regex_contains(CDSTR p, const char* pattern, size_t offset);

int dstr_regex_match(CDSTR p, const char* pattern, size_t offset,
                     DSTR_Regex_Match* match, const char* options);

int dstr_regex_match_groups(CDSTR p, const char* pattern, size_t offset,
                            DSTR_Match_Vector* matches, const char* options);

int dstr_regex_substitute(DSTR p, const char* pattern, size_t offset,
                          const char* replacement, const char* options);
#endif
/*-------------------------------------------------------------------------------*/

/* 3way strcmp-like comparison*/
static inline int dstr_compare_sz(CDSTR lhs, const char* sz)
{
    const char* l = lhs ? lhs->data : "";
    const char* r = sz ? sz : "";
    return strcmp(l, r);
}

static inline int dstr_icompare_sz(CDSTR lhs, const char* sz)
{
    const char* l = lhs ? lhs->data : "";
    const char* r = sz ? sz : "";
    return strcasecmp(l, r);
}

static inline int dstr_compare_ds(CDSTR lhs, CDSTR rhs)
{
    const char* l = lhs ? lhs->data : "";
    const char* r = rhs ? rhs->data : "";
    return strcmp(l, r);
}

static inline int dstr_icompare_ds(CDSTR lhs, CDSTR rhs)
{
    const char* l = lhs ? lhs->data : "";
    const char* r = rhs ? rhs->data : "";
    return strcasecmp(l, r);
}

static inline DSTR_BOOL dstr_equal_sz(CDSTR lhs, const char* sz)
{
    return dstr_compare_sz(lhs, sz) == 0;
}

static inline DSTR_BOOL dstr_iequal_sz(CDSTR lhs, const char* sz)
{
    return dstr_icompare_sz(lhs, sz) == 0;
}

static inline DSTR_BOOL dstr_equal_ds(CDSTR lhs, CDSTR rhs)
{
    return
        (lhs->length == rhs->length) &&
        (dstr_compare_ds(lhs, rhs) == 0);
}

static inline DSTR_BOOL dstr_iequal_ds(CDSTR lhs, CDSTR rhs)
{
    return
        (lhs->length == rhs->length) &&
        (dstr_icompare_ds(lhs, rhs) == 0);
}

static inline size_t dstr_length(CDSTR p) {
    return p->length;
}

static inline size_t dstr_capacity(CDSTR p) {
    return p->capacity;
}

static inline DSTR_BOOL dstr_isempty(CDSTR p)
{
    return (dstr_length(p) == 0);
}

static inline const char* dstr_cstr(CDSTR p)
{
    return p->data;
}

static inline uint8_t* dstr_u8ptr(CDSTR p)
{
    return (uint8_t*) p->data;
}

static inline char dstr_getchar(CDSTR p, size_t pos)
{
    return p->data[pos];
}

static inline char dstr_front(CDSTR p)
{
    return p->data[0];
}

static inline char dstr_back(CDSTR p)
{
    size_t index = p->length ? p->length - 1 : 0;
    return p->data[index];
}

static inline void dstr_putchar(DSTR p, size_t pos, char c)
{
    p->data[pos] = c;
}

static inline DSTR_BOOL dstr_valid_index(CDSTR p, size_t pos)
{
    return (pos < dstr_length(p));
}

static inline void dstr_chop(DSTR p)
{
    if (dstr_isempty(p)) return;
    p->data[--p->length] = '\0';
}

static inline char dstr_getchar_safe(CDSTR p, long pos)
{
    if (pos < 0)
        pos += (long) dstr_length(p);

    return ((0 <= pos) && (pos < (long)(p->length))) ?
        dstr_getchar(p, pos) :
        (char)'\0';
}

static inline void dstr_putchar_safe(DSTR p, long pos, char c)
{
    if (pos < 0)
        pos += (long) dstr_length(p);

    if ((0 <= pos) && (pos < (long)(p->length)))
        dstr_putchar(p, pos, c);
}

static inline int dstr_append_char(DSTR p, char c)
{
    if (c == '\0') {
        return DSTR_SUCCESS; }
    else if (p->length + 1 < p->capacity) {
        p->data[p->length] = c;
        p->data[++p->length] = '\0';
        return DSTR_SUCCESS;  }
    else {
        return dstr_grow_append_c(p, c); }
}

static inline void dstr_init_data(DSTR p)
{
    p->length = 0;
    p->capacity = DSTR_INITIAL_CAPACITY;
    p->sso_buffer[0] = '\0';
    p->data = p->sso_buffer;
}

static inline void dstr_clear(DSTR p)
{
    p->length = 0;
    *p->data = '\0';
}

/* BASIC & MFC like: left, mid, right operations */
static inline int dstr_assign_left(DSTR dest, CDSTR src, size_t count)
{
    return dstr_assign_substr(dest, src, 0, count);
}

static inline int dstr_assign_mid(DSTR dest, CDSTR src, size_t pos, size_t count)
{
    return dstr_assign_substr(dest, src, pos, count);
}

static inline int dstr_assign_right(DSTR dest, CDSTR src, size_t count)
{
    return (count >= dstr_length(src)) ?
        dstr_assign_ds(dest, src) :
        dstr_assign_substr(dest, src, (dstr_length(src) - count), count);
}

// Compatibility
//
#if defined(__BORLANDC__)
#if !defined(__clang__) || (__clang_major__ < 15)
#ifndef strnlen
#define strnlen my_strnlen
#endif
static inline size_t my_strnlen(const char* s, size_t maxlen) {
	const char *p = (char*) memchr(s, 0, maxlen);
	return p ? p - s : maxlen;
}
#endif
#endif

#ifdef __cplusplus
}
#endif

/*
 *  Alternative shorter names and convenience macros
 */
#if !defined(DSTR_CLEAN_NAMESPACE)

/* Stack base init */
#define INIT_DSTR(identifier)    \
    struct DSTR_TYPE identifier; \
    dstr_init_data(&identifier)

#define DONE_DSTR(identifier)    \
    dstr_clean_data(&identifier)

// Default name without type are for NULL teminated C strings
//
#define   dstr_create       dstr_create_sz
#define   dstr_find         dstr_find_sz
#define   dstr_ifind        dstr_ifind_sz
#define   dstr_rfind        dstr_rfind_sz
#define   dstr_irfind       dstr_irfind_sz
#define   dstr_count        dstr_count_sz
#define   dstr_icount       dstr_icount_sz
#define   dstr_contains     dstr_contains_sz
#define   dstr_icontains    dstr_icontains_sz
#define   dstr_suffix       dstr_suffix_sz
#define   dstr_isuffix      dstr_isuffix_sz
#define   dstr_prefix       dstr_prefix_sz
#define   dstr_iprefix      dstr_iprefix_sz
#define   dstr_ffo          dstr_ffo_sz
#define   dstr_ffno         dstr_ffno_sz
#define   dstr_flo          dstr_flo_sz
#define   dstr_flno         dstr_flno_sz
#define   dstr_compare      dstr_compare_sz
#define   dstr_icompare     dstr_icompare_sz
#define   dstr_equal        dstr_equal_sz
#define   dstr_iequal       dstr_iequal_sz
#define   dstr_append       dstr_append_sz
#define   dstr_assign       dstr_assign_sz
#define   dstr_replace      dstr_replace_sz
#define   dstr_replace_all  dstr_replace_all_sz
#define   dstr_insert       dstr_insert_sz
#define   dstr_lstrip       dstr_lstrip_sz
#define   dstr_rstrip       dstr_rstrip_sz

/* * * * * * * * * * * * * * * * * * * * * * *
 *
 * Short C-Style names for above functions
 *
 * * * * * * * * * * * * * * * * * * * * * * */

/* Creation functions. DSTR returned by value */
#define dstrnew_empty       dstr_create_empty
#define dstrnew_reserve     dstr_create_reserve
#define dstrnew             dstr_create_sz
#define dstrnew_bl          dstr_create_bl
#define dstrnew_rng         dstr_create_range
#define dstrnew_cc          dstr_create_cc
#define dstrnew_sprintf     dstr_create_sprintf
#define dstrnew_vsprintf    dstr_create_vsprintf
#define dstrnew_ds          dstr_create_ds
#define dstrnew_substr      dstr_create_substr
#define dstrnew_slurp       dstr_create_fromfile

#define dstrdata            dstr_cstr
#define dstrempty           dstr_isempty
#define dstrlen             dstr_length
#define dstrcap             dstr_capacity

/*
 * Search functions. suffix *_i means ignore case
 */
#define dstrchr             dstr_find_c
#define dstrstr             dstr_find_sz
#define dstrchr_i           dstr_ifind_c
#define dstrstr_i           dstr_ifind_sz

#define dstrrchr            dstr_rfind_c
#define dstrrstr            dstr_rfind_sz
#define dstrrchr_i          dstr_irfind_c
#define dstrrstr_i          dstr_irfind_sz

#define dstrcount           dstr_count_sz
#define dstrcount_i         dstr_icount_sz
#define dstrcount_ds        dstr_count_ds
#define dsticount_di        dstr_icount_ds

#define dstrhas             dstr_contains_sz
#define dstrhas_i           dstr_icontains_sz
#define dendswith           dstr_suffix_sz
#define dendswith_i         dstr_isuffix_sz
#define dstartswith         dstr_prefix_sz
#define dstartswith_i       dstr_iprefix_sz
#define dindex_ok           dstr_valid_index

#define dstrffo             dstr_ffo_sz
#define dstrffno            dstr_ffno_sz
#define dstrffo_ds          dstr_ffo_ds
#define dstrffno_ds         dstr_ffno_ds

#define dstrflo             dstr_flo_sz
#define dstrflno            dstr_flno_sz
#define dstrflo_ds          dstr_flo_ds
#define dstrflno_ds         dstr_flno_ds

#define dstrcmp             dstr_compare_sz
#define dstricmp            dstr_icompare_sz
#define dstrcmp_ds          dstr_compare_ds
#define dstricmp_ds         dstr_icompare_ds
#define dstreq              dstr_equal_sz
#define dstreq_i            dstr_iequal_sz
#define dstreq_ds           dstr_equal_ds
#define dstreq_ids          dstr_iequal_ds

#define dstrchop            dstr_chop
#define dstrchomp           dstr_trim_right
#define dsubstr_bl          dstr_substr
#define dstrputc            dstr_putchar
#define dstrgetc            dstr_getchar
#define dstrputc_s          dstr_putchar_safe
#define dstrgetc_s          dstr_getchar_safe
#define dstrfornt           dstr_front
#define dstrback            dstr_back

#define dstrcpy_cc          dstr_assign_cc
#define dstrcpy             dstr_assign_sz
#define dstrcpy_bl          dstr_assign_bl
#define dstrcpy_rng         dstr_assign_range
#define dstrcpy_ds          dstr_assign_ds
#define dstrcpy_substr      dstr_assign_substr
#define dstrcpy_file        dstr_assign_fromfile
#define dstrcpy_strm        dstr_slurp_stream

/* like BASIC functions Left$, Mid$, Right$ */
#define dstrcpy_right       dstr_assign_right
#define dstrcpy_left        dstr_assign_left
#define dstrcpy_mid         dstr_assign_mid

#define dstrcat             dstr_append_sz
#define dstrcat_cc          dstr_append_cc
#define dstrcat_c           dstr_append_char
#define dstrcat_ds          dstr_append_ds
#define dstrcat_bl          dstr_append_bl
#define dstrcat_rng         dstr_append_range

#define dstralign_r         dstr_align_right
#define dstralign_l         dstr_align_left
#define dstralign_c         dstr_align_center

#define dstrfree            dstr_destroy
#define dcleandata          dstr_clean_data
#define dstrclear           dstr_clear
#define dstrresize          dstr_resize
#define dstrerase           dstr_remove
#define dstrshrink          dstr_shrink_to_fit
#define drmchar             dstr_remove_char
#define drmany              dstr_remove_any
#define drmprefix           dstr_remove_prefix
#define drmsuffix           dstr_remove_suffix
#define drmprefix_i         dstr_remove_prefix
#define drmsuffix_i         dstr_remove_suffix

#define dsprintf            dstr_assign_sprintf
#define dvsprintf           dstr_assign_vsprintf
#define dstrcat_sprintf     dstr_append_sprintf
#define dstrcat_vsprintf    dstr_append_vsprintf

#define dstrjoin            dstr_join_sz
#define dstrmult            dstr_multiply

#define dreplace_cc         dstr_replace_cc
#define dreplace            dstr_replace_sz
#define dreplace_ds         dstr_replace_ds
#define dreplace_bl         dstr_replace_bl
#define dreplace_rng        dstr_replace_range
#define dreplaceall         dstr_replace_all_sz
#define dreplaceall_ds      dstr_replace_all_ds
#define dexpandtabs         dstr_expand_tabs
#define dzfill              dstr_zfill

#define dinsert_cc          dstr_insert_cc
#define dinsert             dstr_insert_sz
#define dinsert_ds          dstr_insert_ds
#define dinsert_bl          dstr_insert_bl
#define dinsert_rng         dstr_insert_range

#define dstrtrim            dstr_trim
#define dstrtrim_r          dstr_trim_right
#define dstrtrim_l          dstr_trim_left
#define dlstrip_c           dstr_lstrip_c
#define drstrip_c           dstr_rstrip_c
#define dlstrip             dstr_lstrip_sz
#define drstrip             dstr_rstrip_sz
#define dstrupper           dstr_ascii_upper
#define dstrlower           dstr_ascii_lower
#define dstrswapcase        dstr_ascii_swapcase
#define dstrrev             dstr_reverse
#define dstrswap            dstr_swap
#define dgetline            dstr_fgetline
#define dgets               dstr_fgets
#define dstrtitle           dstr_title

#define disdigits           dstr_isdigits
#define disxdigits          dstr_isxdigits
#define dstrblank           dstr_isblank
#define disalnum            dstr_isalnum
#define disalpha            dstr_isalpha
#define disascii            dstr_isascii
#define disdecimal          dstr_isdecimal
#define disident            dstr_isidentifier
#define dislower            dstr_islower
#define disnumeric          dstr_isnumeric
#define disprint            dstr_isprintable
#define disspace            dstr_isspace
#define distitle            dstr_istitle
#define dsisupper           dstr_isupper
#define dstrinc             dstr_increment

#define dstrhash            dstr_hash

#define datoi               dstr_atoi
#define datol               dstr_atoi
#define datoll              dstr_atoll
#define ditos               dstr_itos
#define dstrtoi             dstr_to_int
#define dstrtol             dstr_to_long
#define dstrtoul            dstr_to_ulong
#define dstrtoll            dstr_to_llong
#define dstrtoull           dstr_to_ullong
#define dstrtof             dstr_to_float
#define dstrtod             dstr_to_double
#define dstrtold            dstr_to_ldouble

#define dstrtrans           dstr_translate
#define dstrsqz             dstr_squeeze
#define dstrtr_s            dstr_translate_squeeze

#define dstrpart            dstr_partition
#define dstrrpart           dstr_rpartition

// Regex functions
//
#define dre_exact           dstr_regex_exact
#define dre_within          dstr_regex_within
#define dre_match           dstr_regex_match
#define dre_groups          dstr_regex_match_groups
#define dre_subst           dstr_regex_substitute
#define dre_perror          dstr_regex_perror
#define dre_strerror        dstr_regex_strerror
#define dre_mvfree          dstr_regex_mvector_free

/* clean namespace */
#endif

#endif /* DSTR_H_INCLUDED */
