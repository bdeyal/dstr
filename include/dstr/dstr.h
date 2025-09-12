/*
 * dstr.h
 */
#ifndef DSTR_H_INCLUDED
#define DSTR_H_INCLUDED

#include <stdio.h>
#include <stdarg.h>

#if !defined(__cplusplus)
    #if defined(NO_STDBOOL)
        #if !defined(bool)
            typedef enum {false_ = 0, true_ = 1} bool_;
            #define bool bool_
            #define false false_
            #define true  true_
        #endif
    #else
        #include <stdbool.h>
    #endif
#endif
/*--------------------------------------------------------------------------*/

typedef struct DSTR_IMP
{
    size_t length;
    size_t capacity;
    char*  data;
} *DSTR;
/*--------------------------------------------------------------------------*/

#define BASE(p)       (p)
#define DBUF(p)       BASE(p)->data
#define DLEN(p)       BASE(p)->length
#define DCAP(p)       BASE(p)->capacity
#define DVAL(p, i)    DBUF(p)[(i)]
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

/* create by slurp a textfile*/
DSTR dstr_create_fromfile(const char* fname);
DSTR dstr_assign_fromfile(DSTR p, const char* fname);

/* create from a [v]sprintf like call*/
DSTR dstr_create_sprintf(const char* fmt, ...);
DSTR dstr_create_vsprintf(const char* fmt, va_list argptr);

/* destroy a DSTR object*/
void dstr_destroy(DSTR p);

/*
 *  transfer data to c string, destory object, caller must free result
 *  if PLEN is not NULL, write length to it.
 */
char* dstr_move_destroy(DSTR p, size_t* plen);

/*
 *   Assign | Insert | Append | Replace
 *   a DSTR from various sources. returns DSTR_SUCCESS or DSTR_FAIL
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

/*
 *  either truncate to LEN or enlarge capacity to at least LEN without
 *  changing content
 */
int  dstr_resize(DSTR dest, size_t len);

/*
 *  Following operations manipulate data but no allocation / free
 */
void    dstr_ascii_upper(DSTR p);
void    dstr_ascii_lower(DSTR p);
void    dstr_swap(DSTR d1, DSTR d2);
void    dstr_reverse(DSTR p);
void    dstr_trim_right(DSTR p);
void    dstr_trim_left(DSTR p);
void    dstr_trim_both(DSTR p);
void    dstr_truncate(DSTR p);
void    dstr_remove(DSTR p, size_t pos, size_t count);

/* 31 bit hash value */
unsigned int dstr_hash(const DSTR src);

/*   atoi integer conversion
 *
 *   '0b110111' => binary
 *   '0xABCD12' => hex
 *   '\324'     => octal
 *   '123'      => decimal
 *   '0123'     => decimal !
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

/*   ffo    = find_first of, ffno = find first not of */
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

inline size_t dstr_length(const DSTR p) {
    return DLEN(p);
}

inline size_t dstr_capacity(const DSTR p) {
    return DCAP(p);
}

inline DSTR_BOOL dstr_isempty(const DSTR p) {
    return (DLEN(p) == 0);
}

inline const char* dstr_cstring(const DSTR p) {
    return DBUF(p);
}

inline char dstr_getchar(const DSTR p, size_t pos) {
    return DVAL(p, pos);
}

inline void dstr_putchar(DSTR p, size_t pos, char c) {
    DVAL(p, pos) = c;
}

inline DSTR_BOOL dstr_valid_index(const DSTR p, size_t pos) {
    return (pos < DLEN(p));
}

inline char dstr_getchar_safe(const DSTR p, long pos) {
    if (pos < 0) pos += (long) DLEN(p);
    return dstr_valid_index(p, pos) ? DVAL(p, pos) : (char)'\0';
}

inline void dstr_putchar_safe(DSTR p, long pos, char c) {
    if (pos < 0) pos += (long) DLEN(p);
    if (dstr_valid_index(p, pos))
        DVAL(p, pos) = c;
}

inline void dstr_chop(DSTR p) {
    if (DLEN(p) > 0) {
        DLEN(p) -= 1;
        DVAL(p, DLEN(p)) = '\0';
    }
}

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
#define dstrcpy_ds          dstr_assign_ds
#define dstrcpy_substr      dstr_assign_substr

#define dstrcat_sz          dstr_append_sz
#define dstrcat_cc          dstr_append_cc
#define dstrcat_c           dstr_append_c
#define dstrcat_ds          dstr_append_ds
#define dstrcat_bl          dstr_append_bl

#define dstrfree            dstr_destroy
#define dstrmove(p)         dstr_move_destroy((p), NULL)
#define dstrtrunc           dstr_truncate
#define dstrresize          dstr_resize
#define dstrerase           dstr_remove

#define dsprintf            dstr_assign_sprintf
#define dvsprintf           dstr_assign_vsprintf
#define dstrcat_sprintf     dstr_append_sprintf
#define dstrcat_vsprintf    dstr_append_vsprintf

#define dreplace_cc         dstr_replace_cc
#define dreplace_sz         dstr_replace_sz
#define dreplace_ds         dstr_replace_ds
#define dreplace_bl         dstr_replace_bl

#define dinsert_cc          dstr_insert_cc
#define dinsert_sz          dstr_insert_sz
#define dinsert_ds          dstr_insert_ds
#define dinsert_bl          dstr_insert_bl

#define dstrtrim            dstr_trim_both
#define dstrtrim_r          dstr_trim_right
#define dstrtrim_l          dstr_trim_left
#define dstrupper           dstr_ascii_upper
#define dstrlower           dstr_ascii_lower
#define dstrrev             dstr_reverse
#define dstrswap            dstr_swap
#define dgetline            dstr_fgetline
#define dgets               dstr_fgets

#define datoi               dstr_atoi
#define datol               dstr_atoi
#define datoll              dstr_atoll
#define ditoa               dstr_itoa
#define disdigits           dstr_isdigits
#define disxdigits          dstr_isxdigits
#define dstrhash            dstr_hash

/* clean namespace */
#endif


#endif /* DSTR_H_ */
