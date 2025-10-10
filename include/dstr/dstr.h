/*
 * dstr.h
 */
#ifndef DSTR_H_INCLUDED
#define DSTR_H_INCLUDED

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* allocation size at creation */
#define DSTR_INITIAL_CAPACITY (16U)

typedef struct DSTR_TYPE
{
    uint32_t length;
    uint32_t capacity;
    char*  data;
    char   sso_buffer[DSTR_INITIAL_CAPACITY];
} DSTR_TYPE;

typedef       DSTR_TYPE*  DSTR;
typedef const DSTR_TYPE*  CDSTR;
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
DSTR dstr_create_ds(CDSTR rhs);

/* create from a substr of another DSTR*/
DSTR dstr_create_substr(CDSTR p, size_t pos, size_t count);

/* create from a buffer and length*/
DSTR dstr_create_bl(const char* buff, size_t len);

/* create from first->last charecter range */
DSTR dstr_create_range(const char* first, const char* last);

/* create by slurp a textfile*/
DSTR dstr_create_fromfile(const char* fname);
DSTR dstr_assign_fromfile(DSTR p, const char* fname);
DSTR dstr_slurp_stream(DSTR p, FILE* fp);

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

/* private function for use in C++ header */
//int dstr_insert_imp(DSTR p, size_t index, const char* buff, size_t len);
DSTR dstr_grow_ctor(DSTR p, size_t len);

int dstr_insert_cc(DSTR dest, size_t index, char c, size_t count);
int dstr_insert_sz(DSTR dest, size_t index, const char* value);
int dstr_insert_ds(DSTR dest, size_t index, CDSTR src);
int dstr_insert_bl(DSTR dest, size_t index, const char* buff, size_t len);
int dstr_insert_range(DSTR dest, size_t index, const char* first, const char* last);

int dstr_append_c(DSTR dest, char c);
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

/* expand original DEST to be at center of WIDTH characters with fill FILL */
int dstr_align_center(DSTR dest, size_t width, char fill);

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
void dstr_swap(DSTR d1, DSTR d2);
void dstr_reverse(DSTR p);
void dstr_trim_right(DSTR p);
void dstr_trim_left(DSTR p);
void dstr_trim_both(DSTR p);
//void dstr_truncate(DSTR p);
void dstr_remove(DSTR p, size_t pos, size_t count);

/* 31 bit hash value */
unsigned int dstr_hash(CDSTR src);

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
int       dstr_itoa(DSTR dest, long n);
double    dstr_atof(CDSTR src);
DSTR_BOOL dstr_isdigits(CDSTR src);
DSTR_BOOL dstr_isxdigits(CDSTR src);

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

DSTR_BOOL dstr_contains_sz(CDSTR p, const char* s);
DSTR_BOOL dstr_icontains_sz(CDSTR p, const char* s);
DSTR_BOOL dstr_isblank(CDSTR p);

/* p has suffix s, 'endswith' */
DSTR_BOOL dstr_suffix_sz(CDSTR p, const char* s);
DSTR_BOOL dstr_isuffix_sz(CDSTR p, const char* s);

/* p has prefix s, 'startswith' */
DSTR_BOOL dstr_prefix_sz(CDSTR p, const char* s);
DSTR_BOOL dstr_iprefix_sz(CDSTR p, const char* s);

/* ffo = find_first of, ffno = find first not of */
size_t dstr_ffo_sz(CDSTR p, size_t pos, const char* s);
size_t dstr_ffo_ds(CDSTR p, size_t pos, CDSTR s);
size_t dstr_ffno_sz(CDSTR p, size_t pos, const char* s);
size_t dstr_ffno_ds(CDSTR p, size_t pos, CDSTR s);

/* flo = find_last of, flno = find last not of */
size_t dstr_flo_sz(CDSTR p, size_t pos, const char* s);
size_t dstr_flo_ds(CDSTR p, size_t pos, CDSTR s);
size_t dstr_flno_sz(CDSTR p, size_t pos, const char* s);
size_t dstr_flno_ds(CDSTR p, size_t pos, CDSTR s);

/* copy NUMBYTES from INDEX to DEST. returns copied characters*/
size_t dstr_substr(CDSTR p, size_t index, size_t numbytes, char dest[], size_t destsize);

/* 3way strcmp-like comparison*/
int dstr_compare_sz(CDSTR lhs, const char* sz);
int dstr_compare_ds(CDSTR lhs, CDSTR rhs);

/* is equal*/
DSTR_BOOL dstr_equal_sz(CDSTR lhs, const char* sz);
DSTR_BOOL dstr_iequal_sz(CDSTR lhs, const char* sz);
DSTR_BOOL dstr_equal_ds(CDSTR lhs, CDSTR rhs);

/* I/O functions*/
int dstr_fgets(DSTR d, FILE* fp);
int dstr_fgetline(DSTR d, FILE* fp);

static inline size_t dstr_length(CDSTR p) {
    return p->length;
}

static inline size_t dstr_capacity(CDSTR p) {
    return p->capacity;
}

static inline DSTR_BOOL dstr_isempty(CDSTR p) {
    return (dstr_length(p) == 0);
}

static inline const char* dstr_cstring(CDSTR p) {
    return p->data;
}

static inline char dstr_getchar(CDSTR p, size_t pos) {
    return p->data[pos];
}

static inline void dstr_putchar(DSTR p, size_t pos, char c) {
    p->data[pos] = c;
}

static inline DSTR_BOOL dstr_valid_index(CDSTR p, size_t pos) {
    return (pos < dstr_length(p));
}

static inline void dstr_chop(DSTR p) {
    if (dstr_length(p) > 0) {
        p->data[--p->length] = '\0';
    }
}

static inline char dstr_getchar_safe(CDSTR p, long pos) {
    if (pos < 0)
        pos += (long) dstr_length(p);

    return dstr_valid_index(p, pos) ?
        dstr_getchar(p, pos) :
        (char)'\0';
}

static inline void dstr_putchar_safe(DSTR p, long pos, char c) {
    if (pos < 0)
        pos += (long) dstr_length(p);

    if (dstr_valid_index(p, pos))
        dstr_putchar(p, pos, c);
}

static inline int dstr_append_inline(DSTR p, char c) {
    if (c == '\0')
        return DSTR_SUCCESS;

    if (dstr_length(p) + 1 < dstr_capacity(p)) {
        p->data[p->length] = c;
        p->data[++p->length] = '\0';
        return DSTR_SUCCESS;
    }
    /*else*/
    return dstr_append_c(p, c);
}

static inline void dstr_init_data(DSTR p)
{
    p->length = 0;
    p->capacity = DSTR_INITIAL_CAPACITY;
    p->sso_buffer[0] = '\0';
    p->data = &p->sso_buffer[0];
}

static inline void dstr_truncate(DSTR p)
{
    p->length = 0;
    *p->data = '\0';
}

/* BASIC & MFC like: left, mid, right operations */
static inline int dstr_assign_left(DSTR dest, CDSTR src, size_t count) {
    return dstr_assign_substr(dest, src, 0, count);
}

static inline int dstr_assign_mid(DSTR dest, CDSTR src, size_t pos, size_t count) {
    return dstr_assign_substr(dest, src, pos, count);
}

static inline int dstr_assign_right(DSTR dest, CDSTR src, size_t count) {
    return (count >= dstr_length(src)) ?
        dstr_assign_ds(dest, src) :
        dstr_assign_substr(dest, src, (dstr_length(src) - count), count);
}

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


/*-------------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

/*
 *  Alternative shorter names
 */
#if !defined(DSTR_CLEAN_NAMESPACE)

/* Creation functions. DSTR returned by value */
#define dstrnew             dstr_create
#define dstrnew_reserve     dstr_create_reserve
#define dstrnew_sz          dstr_create_sz
#define dstrnew_bl          dstr_create_bl
#define dstrnew_rng         dstr_create_range
#define dstrnew_cc          dstr_create_cc
#define dstrnew_sprintf     dstr_create_sprintf
#define dstrnew_vsprintf    dstr_create_vsprintf
#define dstrnew_ds          dstr_create_ds
#define dstrnew_substr      dstr_create_substr
#define dstrnew_slurp       dstr_create_fromfile

#define dstrdata            dstr_cstring
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

#define dstrhas             dstr_contains_sz
#define dstrhas_i           dstr_icontains_sz
#define dendswith           dstr_suffix_sz
#define dendswith_i         dstr_isuffix_sz
#define dstartswith         dstr_prefix_sz
#define dstartswith_i       dstr_iprefix_sz
#define dindex_ok           dstr_valid_index
#define dstrblank           dstr_isblank

#define dstrffo_sz          dstr_ffo_sz
#define dstrffo_ds          dstr_ffo_ds
#define dstrffno_sz         dstr_ffno_sz
#define dstrffno_ds         dstr_ffno_ds

#define dstrflo_sz          dstr_flo_sz
#define dstrflo_ds          dstr_flo_ds
#define dstrflno_sz         dstr_flno_sz
#define dstrflno_ds         dstr_flno_ds

#define dstrcmp_sz          dstr_compare_sz
#define dstrcmp_ds          dstr_compare_ds
#define dstreq_sz           dstr_equal_sz
#define dstreq_i            dstr_iequal_sz
#define dstreq_ds           dstr_equal_ds

#define dstrchop            dstr_chop
#define dstrchomp           dstr_trim_right
#define dsubstr_bl          dstr_substr
#define dstrputc            dstr_putchar
#define dstrgetc            dstr_getchar
#define dstrputc_s          dstr_putchar_safe
#define dstrgetc_s          dstr_getchar_safe

#define dstrcpy_cc          dstr_assign_cc
#define dstrcpy_sz          dstr_assign_sz
#define dstrcpy_bl          dstr_assign_bl
#define dstrcpy_rng         dstr_assign_range
#define dstrcpy_ds          dstr_assign_ds
#define dstrcpy_substr      dstr_assign_substr
#define dstrcpy_file        dstr_assign_fromfile
#define dstrcpy_strm        dstr_slurp_stream

#define dstrcat_sz          dstr_append_sz
#define dstrcat_cc          dstr_append_cc
#define dstrcat_c           dstr_append_inline
#define dstrcat_ds          dstr_append_ds
#define dstrcat_bl          dstr_append_bl
#define dstrcat_rng         dstr_append_range
#define dstrcenter          dstr_align_center

#define dstrfree            dstr_destroy
#define dstrtrunc           dstr_truncate
#define dstrresize          dstr_resize
#define dstrerase           dstr_remove
#define dstrshrink          dstr_shrink_to_fit

#define dsprintf            dstr_assign_sprintf
#define dvsprintf           dstr_assign_vsprintf
#define dstrcat_sprintf     dstr_append_sprintf
#define dstrcat_vsprintf    dstr_append_vsprintf

#define dreplace_cc         dstr_replace_cc
#define dreplace_sz         dstr_replace_sz
#define dreplace_ds         dstr_replace_ds
#define dreplace_bl         dstr_replace_bl
#define dreplace_rng        dstr_replace_range

#define dinsert_cc          dstr_insert_cc
#define dinsert_sz          dstr_insert_sz
#define dinsert_ds          dstr_insert_ds
#define dinsert_bl          dstr_insert_bl
#define dinsert_rng         dstr_insert_range

#define dstrtrim            dstr_trim_both
#define dstrtrim_r          dstr_trim_right
#define dstrtrim_l          dstr_trim_left
#define dstrupper           dstr_ascii_upper
#define dstrlower           dstr_ascii_lower
#define dstrrev             dstr_reverse
#define dstrswap            dstr_swap
#define dgetline            dstr_fgetline
#define dgets               dstr_fgets

#define disdigits           dstr_isdigits
#define disxdigits          dstr_isxdigits
#define dstrhash            dstr_hash

#define datoi               dstr_atoi
#define datol               dstr_atoi
#define datoll              dstr_atoll
#define ditoa               dstr_itoa
#define dstrtoi             dstr_to_int
#define dstrtol             dstr_to_long
#define dstrtoul            dstr_to_ulong
#define dstrtoll            dstr_to_llong
#define dstrtoull           dstr_to_ullong
#define dstrtof             dstr_to_float
#define dstrtod             dstr_to_double
#define dstrtold            dstr_to_ldouble



/* clean namespace */
#endif


#endif /* DSTR_H_ */
