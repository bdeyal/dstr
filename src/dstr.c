/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <limits.h>

#include <dstr/dstr.h>
#include "dstr_internal.h"

#if !defined(__cplusplus)
#if __STDC_VERSION__ < 199901L
#error dstr requires at least a C99 compiler
#endif
#endif

/*
 *   garbage collector / memory checker
 */
#ifdef GC_DEBUG
#include <gc/gc.h>
#define malloc  GC_MALLOC
#define free    GC_FREE
#define realloc GC_REALLOC
#endif
/*-------------------------------------------------------------------------------*/

/*
 *  For convenience, make code shorter
 */
#define BASE(p)       (p)
#define DBUF(p)       (BASE(p)->data)
#define DLEN(p)       (BASE(p)->length)
#define DVAL(p, i)    DBUF(p)[(i)]
#define DCAP(p)       (BASE(p)->capacity)
#define D_SSO_BUF(p)  (&(p)->sso_buffer[0])
#define D_IS_SSO(p)   (DBUF(p) == D_SSO_BUF(p))
/*--------------------------------------------------------------------------*/

// Checks for CDSTR type (a.k.s const DSTR_TYPE*)
//
#define dstr_assert_view(p) do {                                \
        assert((p) != NULL);                                    \
        assert(DBUF(p) != NULL);                                \
        assert(DVAL(p, DLEN(p)) == '\0');                       \
        assert(DLEN(p) == strlen(DBUF(p)));                     \
} while(0)

// Checks for DSTR type (a.k.s DSTR_TYPE*)
//
#define dstr_assert_valid(p) do {                               \
        dstr_assert_view(p);                                    \
        assert(DLEN(p) < DCAP(p));                              \
        assert((DCAP(p) % DSTR_INITIAL_CAPACITY) == 0);         \
    } while(0)
/*--------------------------------------------------------------------------*/

static const char* my_strcasechr(const char* s, int c)
{
    if (!s) return NULL;

    unsigned char uc = (unsigned char) toupper(c);
    unsigned char lc = (unsigned char) tolower(c);

    for (; *s != '\0'; ++s) {
        unsigned char curr = *s;
        if (curr == lc || curr == uc)
            return s;
    }

    return NULL;
}
/*-------------------------------------------------------------------------------*/

static const char* my_strcasestr(const char* haystack, const char* needle)
{
    if (*needle == '\0')
        return haystack;

    for ( ; *haystack; ++haystack) {
        const char* s1 = haystack;
        const char* s2 = needle;

        while (*s1 && *s2 && (*s1 == *s2 || toupper(*s1) == toupper(*s2))) {
            ++s1;
            ++s2;
        }

        if (*s2 == '\0')
            return haystack;
    }

    return NULL;
}
/*-------------------------------------------------------------------------------*/

static inline size_t min_2(size_t a, size_t b)
{
    return a <= b ? a : b;
}
/*-------------------------------------------------------------------------------*/

static inline size_t min_3(size_t a, size_t b, size_t c)
{
    return min_2(min_2(a, b), c);
}
/*-------------------------------------------------------------------------------*/

static inline char* dstr_address(DSTR p, size_t pos)
{
    return (DBUF(p) + pos);
}
/*-------------------------------------------------------------------------------*/

static inline const char* dstr_address_c(CDSTR p, size_t pos)
{
    return (DBUF(p) + pos);
}
/*-------------------------------------------------------------------------------*/

static inline char* dstr_tail(DSTR p)
{
    return dstr_address(p, DLEN(p));
}
/*-------------------------------------------------------------------------------*/

static inline const char* dstr_end_of_storage(DSTR p)
{
    return dstr_address_c(p, DCAP(p));
}
/*-------------------------------------------------------------------------------*/

static void dstr_out_of_memory(void)
{
    fprintf(stderr, "DSTR library: malloc/realloc failed. Out of memory!\n");
    abort();
}
/*-------------------------------------------------------------------------------*/

static DSTR dstr_alloc_empty(void)
{
    DSTR p = (DSTR) malloc(sizeof(struct DSTR_TYPE));
    if (!p) {
        errno = ENOMEM;
        dstr_out_of_memory();
        return NULL;
    }

    dstr_init_data(p);
    dstr_assert_valid(p);
    return p;
}
/*-------------------------------------------------------------------------------*/

// Not static since used in c++ side too
//
int dstr_grow_ctor(DSTR p, size_t len)
{
    assert(len >= DSTR_INITIAL_CAPACITY);

    size_t new_capacity = DSTR_INITIAL_CAPACITY;
    while (new_capacity <= len)
        new_capacity *= 2;

    if (new_capacity > UINT32_MAX) {
        errno = ENOMEM;
        dstr_out_of_memory();
        return DSTR_FAIL;
    }

    char* newbuff = (char*) malloc(new_capacity);
    if (!newbuff) {
        errno = ENOMEM;
        dstr_out_of_memory();
        return DSTR_FAIL;
    }

    newbuff[0] = '\0';
    p->capacity = new_capacity;
    p->data = newbuff;
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

static int dstr_grow(DSTR p, size_t len)
{
    char* newbuff;

    assert(p != NULL);

    if (p->capacity > len) {
        return DSTR_SUCCESS;
    }

    size_t new_capacity = p->capacity;
    while (new_capacity <= len)
        new_capacity *= 2;

    if (new_capacity > UINT32_MAX) {
        errno = ENOMEM;
        dstr_out_of_memory();
        return DSTR_FAIL;
    }

    if (D_IS_SSO(p)) {
        if ((newbuff = (char*) malloc(new_capacity)) == NULL) {
            errno = ENOMEM;
            dstr_out_of_memory();
            return DSTR_FAIL;
        }

        if (p->length)
            memcpy(newbuff, p->data, p->length + 1);
        else
            *newbuff = '\0';

        p->data[0] = '\0';
    }
    else {
        if ((newbuff = (char*) realloc(p->data, new_capacity)) == NULL) {
            errno = ENOMEM;
            dstr_out_of_memory();
            return DSTR_FAIL;
        }
    }

    p->capacity = new_capacity;
    p->data = newbuff;

    dstr_assert_valid(p);
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

static inline int dstr_grow_by(DSTR p, size_t n)
{
    return dstr_grow(p, n + DLEN(p));
}
/*-------------------------------------------------------------------------------*/

static inline void dstr_truncate_imp(DSTR p, size_t len)
{
    assert(len <= DLEN(p));
    DLEN(p) = len;
    DVAL(p, len) = '\0';
}
/*-------------------------------------------------------------------------------*/

static inline DSTR_BOOL is_overlap(DSTR p, const char* value)
{
    return (DBUF(p) <= value && value < dstr_end_of_storage(p));
}
/*-------------------------------------------------------------------------------*/

static int dstr_insert_imp(DSTR p, size_t index, const char* buff, size_t len)
{
    if (buff == NULL || len == 0)
        return DSTR_SUCCESS;

    // different handling if source data within the DSTR allocated
    // buffer
    //
    ptrdiff_t overlap = -1;
    char* first = DBUF(p);

    if (is_overlap(p, buff))
        overlap = buff - first;

    if (!dstr_grow_by(p, len))
        return DSTR_FAIL;

    // in case of overlap, check if realloc() moved the buffer
    //
    if (overlap >= 0 && (DBUF(p) != first))
        buff = DBUF(p) + overlap;

    index = min_2(index, DLEN(p));
    size_t bytes_to_move = DLEN(p) - index;

    if (bytes_to_move > 0) {
        assert(index + len + bytes_to_move < DCAP(p));
        char* to = dstr_address(p, index + len);
        const char* from  = dstr_address(p, index);
        memmove(to, from, bytes_to_move);
    }

    memcpy(dstr_address(p, index), buff, len);
    DLEN(p) += len;
    DVAL(p, DLEN(p)) = '\0';

    dstr_assert_valid(p);
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

/*  For cases we are SURE that 'buff' is not in 'dest' buffer range
 */
static int dstr_append_no_overlap(DSTR dest, const char* buff, size_t len)
{
    if (!dstr_grow_by(dest, len))
        return DSTR_FAIL;

    memcpy(dstr_tail(dest), buff, len);
    DLEN(dest) += len;
    DVAL(dest, DLEN(dest)) = '\0';
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

static int dstr_append_imp(DSTR p, const char* buff, size_t len)
{
    if (len == 0)
        return DSTR_SUCCESS;

    if (is_overlap(p, buff)) {
        const char* first = DBUF(p);
        ptrdiff_t overlap = buff - first;

        if (!dstr_grow_by(p, len))
            return DSTR_FAIL;

        // realloc in dstr_grow_by might change the base pointer.
        // In case of overlap we must update
        //
        if (DBUF(p) != first)
            buff = DBUF(p) + overlap;
    }
    else {
        if (!dstr_grow_by(p, len)) {
            return DSTR_FAIL; }}

    memcpy(dstr_tail(p), buff, len);
    DLEN(p) += len;
    DVAL(p, DLEN(p)) = '\0';
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

static int dstr_insert_cc_imp(DSTR p, size_t index, char c, size_t count)
{
    if (count == 0)
        return DSTR_SUCCESS;

    index = min_2(index, DLEN(p));

    if (c == '\0') {
        dstr_truncate_imp(p, index);
        return DSTR_SUCCESS;
    }

    if (!dstr_grow_by(p, count)) {
        return DSTR_FAIL;
    }

    size_t bytes_to_move = DLEN(p) - index;
    if (bytes_to_move > 0) {
        char* to = dstr_address(p, index + count);
        const char* from = dstr_address(p, index);
        memmove(to, from, bytes_to_move);
    }

    memset(dstr_address(p, index), c, count);
    DLEN(p) += count;
    DVAL(p,DLEN(p)) = '\0';

    dstr_assert_valid(p);
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

static void dstr_remove_imp(DSTR p, size_t pos, size_t count)
{
    if (count == 0)
        return;

    if (pos >= DLEN(p))
        return;

    count = min_2(count, DLEN(p) - pos);
    size_t bytes_to_move = DLEN(p) - pos - count;

    if (bytes_to_move > 0) {
        char* to = dstr_address(p, pos);
        const char* from = dstr_address(p, pos + count);
        memmove(to, from, bytes_to_move);
    }

    DLEN(p) -= count;
    DVAL(p,DLEN(p)) = '\0';

    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

static int dstr_replace_imp(DSTR p,
                            size_t pos,
                            size_t count,
                            const char* buff,
                            size_t buflen)
{
    bool nothing_to_replace =
        (buff == NULL) || \
        (buflen == 0)  || \
        ((buflen = strnlen(buff, buflen)) == 0);

    if (nothing_to_replace) {
        dstr_remove_imp(p, pos, count);
        return DSTR_SUCCESS;
    }

    int result = DSTR_FAIL;

    // in case of overlap we copy to a tmp DSTR
    //
    if (is_overlap(p, buff)) {
        INIT_DSTR(tmp);
        if (!dstr_append_no_overlap(&tmp, buff, buflen))
            return DSTR_FAIL;

        dstr_remove_imp(p, pos, count);
        buff = DBUF(&tmp);
        result = dstr_insert_imp(p, pos, buff, buflen);
        DONE_DSTR(tmp);
    }
    else {
        dstr_remove_imp(p, pos, count);
        result = dstr_insert_imp(p, pos, buff, buflen);
    }

    return result;
}
/*-------------------------------------------------------------------------------*/

static inline int dstr_replace_cc_imp(DSTR p,
                                      size_t pos,
                                      size_t count_old,
                                      char c,
                                      size_t count_new)
{
    dstr_remove_imp(p, pos, count_old);
    return dstr_insert_cc_imp(p, pos, c, count_new);
}
/*-------------------------------------------------------------------------------*/

static inline int dstr_assign_imp(DSTR p, const char* value, size_t len)
{
    if (DLEN(p) > 0)
        dstr_clear(p);

    return dstr_append_imp(p, value, len);
}
/*-------------------------------------------------------------------------------*/

static inline DSTR dstr_create_len_imp(size_t len)
{
    DSTR p = dstr_alloc_empty();
    if (!p) {
        return NULL; }

    if (len < DSTR_INITIAL_CAPACITY) {
        return p; }

    if (!dstr_grow_ctor(p, len)) {
        dstr_destroy(p);
        return NULL; }

    return p;
}
/*-------------------------------------------------------------------------------*/

static DSTR dstr_create_buff_imp(const char* buff, size_t len)
{
    if (!buff || len == 0)
        return dstr_alloc_empty();

    // verify no embedded nulls
    //
    len = strnlen(buff, len);

    // Now allocate
    //
    DSTR p;
    if ((p = dstr_create_len_imp(len)) == NULL)
        return NULL;

    // copy buffer and set len and null terminator
    //
    memcpy(DBUF(p), buff, len);
    DVAL(p,len) = '\0';
    DLEN(p) = len;

    dstr_assert_valid(p);
    return p;
}
/*-------------------------------------------------------------------------------*/

static size_t dstr_find_sz_imp(CDSTR p,
                               size_t pos,
                               const char* s,
                               int ignore_case)
{
    const char* search_loc;
    const char* found_loc;

    dstr_assert_view(p);

    if (pos >= DLEN(p))
        return DSTR_NPOS;

    search_loc = DBUF(p) + pos;

    if (ignore_case)
        found_loc = my_strcasestr(search_loc, s);
    else
        found_loc = strstr(search_loc, s);

    if (found_loc == NULL)
        return DSTR_NPOS;

    return (size_t)(found_loc - DBUF(p));
}
/*-------------------------------------------------------------------------------*/

static size_t dstr_rfind_sz_imp(CDSTR p,
                                size_t pos,
                                const char* s,
                                int ignore_case)
{
    const char* found_loc = NULL;

    dstr_assert_view(p);

    size_t slen = strlen(s);
    if (slen > DLEN(p))
        return DSTR_NPOS;

    if (pos >= DLEN(p))
        pos = DLEN(p);

    if (slen == 0)
        return pos;

    for (const char* search_loc = dstr_address_c(p, pos) ;
         search_loc >= DBUF(p) ;
         --search_loc)
    {
        if (ignore_case) {
            if (strncasecmp(search_loc, s, slen) == 0) {
                found_loc = search_loc;
                break;
            }
        }
        else {
            if (strncmp(search_loc, s, slen) == 0) {
                found_loc = search_loc;
                break;
            }
        }
    }

    if (found_loc == NULL)
        return DSTR_NPOS;

    return (size_t)(found_loc - DBUF(p));
}
/*-------------------------------------------------------------------------------*/

static ptrdiff_t dstr_suffix_sz_imp(CDSTR p,
                                    const char* s,
                                    int ignore_case)
{
    size_t compare_len;
    const char* compare_addr;

    dstr_assert_view(p);
    assert(s != NULL);

    if (!p || !s) {
        return -1; }

    if ((compare_len = strlen(s)) > DLEN(p)) {
        return -1; }

    compare_addr = DBUF(p) + (DLEN(p) - compare_len);

    if (ignore_case) {
        if (strcasecmp(compare_addr, s) != 0) {
            return -1; } }
    else {
        if (strcmp(compare_addr, s) != 0) {
            return -1; } }

    return (compare_addr - DBUF(p));
}
/*-------------------------------------------------------------------------------*/

static ptrdiff_t dstr_prefix_sz_imp(CDSTR p,
                                    const char* s,
                                    int ignore_case)
{
    const char* pbuf;

    dstr_assert_view(p);
    assert(s != NULL);

    pbuf = DBUF(p);
    if (ignore_case) {
        while (*s && ((*pbuf == *s) || (toupper(*pbuf) == toupper(*s)))) {
            ++pbuf;
            ++s;
        }
    }
    else {
        while (*s && (*pbuf == *s)) {
            ++pbuf;
            ++s;
        }
    }
    return (*s == '\0') ? (pbuf - DBUF(p)) : -1;
}
/*-------------------------------------------------------------------------------*/

static size_t dstr_find_c_imp(CDSTR p,
                              size_t pos,
                              char c,
                              int ignore_case)
{
    const char* search_loc;
    const char* found_loc;

    dstr_assert_view(p);

    if (pos >= DLEN(p))
        return DSTR_NPOS;

    search_loc = DBUF(p) + pos;

    if (ignore_case)
        found_loc = my_strcasechr(search_loc, c);
    else
        found_loc = strchr(search_loc, c);

    if (found_loc == NULL)
        return DSTR_NPOS;

    return (size_t)(found_loc - DBUF(p));
}
/*-------------------------------------------------------------------------------*/

static size_t dstr_rfind_c_imp(CDSTR p,
                               size_t pos,
                               char c,
                               int ignore_case)
{
    const char* found_loc = NULL;

    dstr_assert_view(p);

    if (DLEN(p) == 0)
        return DSTR_NPOS;

    if (pos >= DLEN(p))
        pos = DLEN(p) - 1;

    char C_ = ignore_case ? toupper(c) : c;

    for (const char* search_loc = dstr_address_c(p, pos);
         search_loc >= DBUF(p);
         --search_loc)
    {
        if (*search_loc == c) {
            found_loc = search_loc;
            break;
        }

        if (ignore_case && toupper(*search_loc) == C_) {
            found_loc = search_loc;
            break;
        }
    }

    if (found_loc == NULL)
        return DSTR_NPOS;

    return (size_t)(found_loc - DBUF(p));
}
/*-------------------------------------------------------------------------------*/

static size_t dstr_ffo_imp(CDSTR p,
                           size_t pos,
                           const char* pattern,
                           int not_off)
{
    size_t result = pos;

    dstr_assert_view(p);
    assert(pattern != NULL);

    if (pattern == NULL)
        return DSTR_NPOS;

    if (pos >= DLEN(p))
        return DSTR_NPOS;

    if (not_off)
        result += strspn(DBUF(p) + pos, pattern);
    else
        result += strcspn(DBUF(p) + pos, pattern);

    if (result == DLEN(p))
        result = DSTR_NPOS;

    return result;
}
/*-------------------------------------------------------------------------------*/

static size_t dstr_flo_imp(CDSTR p,
                           size_t pos,
                           const char* pattern,
                           int not_off)
{
    dstr_assert_view(p);

    if (pattern == NULL || *pattern == '\0' || DLEN(p) == 0)
        return DSTR_NPOS;

    // lookup table for char in pattern
    //
    unsigned char tbl[256] = { 0 };
    for (; *pattern; ++pattern)
        tbl[(unsigned char) *pattern] = 1;

    if (pos >= DLEN(p))
        pos = DLEN(p) - 1;

    for (size_t i = 0; i <= pos; ++i) {
        size_t index = pos - i;
        int ch = DVAL(p, index);
        bool ch_in_pattern = (tbl[ch] == 1);

        if (not_off) {
            if (!ch_in_pattern) {
                return index;
            }
        }
        else {
            if (ch_in_pattern) {
                return index;
            }
        }
    }

    return DSTR_NPOS;
}
/*-------------------------------------------------------------------------------*/

static DSTR_BOOL dstr_isdigits_imp(CDSTR src, DSTR_BOOL is_hex)
{
    size_t n;

    dstr_assert_view(src);

    if (DLEN(src) == 0)
        return DSTR_FALSE;

    if (is_hex) {
        for (n = 0; n < DLEN(src); ++n) {
            if (!isxdigit(DVAL(src, n)))
                return DSTR_FALSE;
        }
    }
    else {
        for (n = 0; n < DLEN(src); ++n) {
            if (!isdigit(DVAL(src, n)))
                return DSTR_FALSE;
        }
    }

    return DSTR_TRUE;
}
/*-------------------------------------------------------------------------------*/

/*
 * * * * * * * * * * * * * * * * * *
 *
 *       P U B L I C    A P I
 *
 * * * * * * * * * * * * * * * * * *
 */
DSTR dstr_create_empty(void)
{
    return dstr_alloc_empty();
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_reserve(size_t len)
{
    return dstr_create_len_imp(len);
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_bl(const char* buff, size_t len)
{
    // functon checks for NULL or len==0
    //
    return dstr_create_buff_imp(buff, len);
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_range(const char* first, const char* last)
{
    size_t len = (size_t)(last - first);

    // functon checks for NULL or len==0
    //
    return dstr_create_buff_imp(first, len);
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_sz(const char* sz)
{
    if (!sz)
        return dstr_alloc_empty();

    return dstr_create_buff_imp(sz, strlen(sz));
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_ds(CDSTR rhs)
{
    if (!rhs)
        return dstr_alloc_empty();

    dstr_assert_view(rhs);
    return dstr_create_buff_imp(DBUF(rhs), DLEN(rhs));
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_substr(CDSTR p, size_t pos, size_t count)
{
    if (!p || DLEN(p) <= pos) {
        return dstr_alloc_empty(); }

    dstr_assert_view(p);
    if ((count = min_2(count, DLEN(p) - pos)) == 0)
        return dstr_alloc_empty();

    return dstr_create_buff_imp(DBUF(p) + pos, count);
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_cc(char ch, size_t count)
{
    DSTR p;

    if (count == 0 || ch == '\0')
        return dstr_alloc_empty();

    if ((p = dstr_create_len_imp(count)) == NULL)
        return NULL;

    memset(DBUF(p), ch, count);
    DVAL(p, count) = '\0';
    DLEN(p) = count;

    dstr_assert_valid(p);
    return p;
}
/*-------------------------------------------------------------------------------*/

int dstr_slurp_stream(DSTR p, FILE* fp)
{
    dstr_assert_valid(p);

    if (!fp) {
        errno = EBADF;
        return DSTR_FAIL;
    }

    char chunk[4096];
    for (;;) {
        size_t len = fread(chunk, sizeof(char), sizeof(chunk), fp);
        if (len < sizeof(chunk)) {
            if (ferror(fp)) {
                dstr_clear(p);
                return DSTR_FAIL;
            }
        }

        if (len) {
            // We don't allow null bytes in DSTR
            //
            if (memchr(chunk, '\0', len)) {
                errno = EINVAL;
                dstr_clear(p);
                return DSTR_FAIL;
            }

            if (!dstr_append_no_overlap(p, chunk, len)) {
                dstr_clear(p);
                return DSTR_FAIL;
            }
        }

        if (feof(fp)) {
            break;
        }
    }

    dstr_assert_valid(p);
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

int dstr_assign_fromfile(DSTR p, const char* fname)
{
    FILE* fp;

    if ((fp = fopen(fname, "r")) == NULL) {
        return DSTR_FAIL;
    }

    dstr_clear(p);
    int rc = dstr_slurp_stream(p, fp);
    fclose(fp);
    dstr_assert_valid(p);
    return rc;
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_fromfile(const char* fname)
{
    DSTR p = dstr_alloc_empty();

    // if (!p) errno is already set
    //
    if (!p)
        return NULL;

    if (dstr_assign_fromfile(p, fname) == DSTR_FAIL) {
        dstr_destroy(p);
        return NULL;
    }

    return p;
}
/*-------------------------------------------------------------------------------*/

void dstr_clean_data(DSTR p)
{
    if (p && !D_IS_SSO(p)) {
        free(p->data);
    }
}
/*-------------------------------------------------------------------------------*/

void dstr_destroy(DSTR p)
{
    if (p) {
        dstr_clean_data(p);
        free(p);
    }
}
/*-------------------------------------------------------------------------------*/

int dstr_reserve(DSTR p, size_t len)
{
    return dstr_grow(p, len);
}
/*-------------------------------------------------------------------------------*/

int dstr_resize(DSTR p, size_t len)
{
    int result = DSTR_SUCCESS;

    dstr_assert_valid(p);

    if (len == DLEN(p)) {
        /* do nothing */
    }
    else if (len < DLEN(p)) {
        dstr_truncate_imp(p, len);
        dstr_assert_valid(p);
    }
    else {
        if (!dstr_grow(p, len))
            result = DSTR_FAIL;
    }

    return result;
}
/*-------------------------------------------------------------------------------*/

int dstr_shrink_to_fit(DSTR p)
{
    dstr_assert_valid(p);

    // If nothing to shrink, return immediately
    //
    if (D_IS_SSO(p))
        return DSTR_SUCCESS;

    if (DCAP(p)/2 < DLEN(p))
        return DSTR_SUCCESS;

    // Otherwise create a fresh new and swap representation
    //
    INIT_DSTR(tmp);
    if (!dstr_assign_ds(&tmp, p))
        return DSTR_FAIL;

    dstr_swap(p, &tmp);
    DONE_DSTR(tmp);

    dstr_assert_valid(p);
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

void dstr_remove(DSTR p, size_t pos, size_t count)
{
    dstr_assert_valid(p);
    dstr_remove_imp(p, pos, count);
    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

int dstr_assign_ds(DSTR dest, CDSTR src)
{
    dstr_assert_valid(dest);
    dstr_assert_view(src);

    return
        (dest == src) ?
        DSTR_SUCCESS :
        dstr_assign_imp(dest, DBUF(src), DLEN(src));
}
/*-------------------------------------------------------------------------------*/

int dstr_assign_sz(DSTR p, const char* value)
{
    dstr_assert_valid(p);

    if (value == NULL) {
        dstr_clear(p);
        return DSTR_SUCCESS;
    }

    size_t len = strlen(value);
    return dstr_assign_imp(p, value, len);
}
/*-------------------------------------------------------------------------------*/

int dstr_assign_bl(DSTR p, const char* buff, size_t len)
{
    dstr_assert_valid(p);

    if (buff == NULL || len == 0) {
        dstr_clear(p);
        return DSTR_SUCCESS;
    }

    len = strnlen(buff, len);
    return dstr_assign_imp(p, buff, len);
}
/*-------------------------------------------------------------------------------*/

int dstr_assign_range(DSTR p, const char* first, const char* last)
{
    dstr_assert_valid(p);

    size_t len = last - first;

    if (first == NULL || len == 0) {
        dstr_clear(p);
        return DSTR_SUCCESS;
    }

    len = strnlen(first, len);
    return dstr_assign_imp(p, first, len);
}
/*-------------------------------------------------------------------------------*/

int dstr_assign_substr(DSTR dest, CDSTR p, size_t pos, size_t count)
{
    dstr_assert_view(p);

    if (pos >= DLEN(p))
        return DSTR_SUCCESS;

    if (count == 0)
        return DSTR_SUCCESS;

    const char* buff = DBUF(p) + pos;
    size_t len = min_2(count, DLEN(p)-pos);
    return dstr_assign_imp(dest, buff, len);
}
/*-------------------------------------------------------------------------------*/

int dstr_assign_cc(DSTR p, char c, size_t count)
{
    dstr_assert_valid(p);
    dstr_clear(p);
    return dstr_insert_cc_imp(p, DLEN(p), c, count);
}
/*-------------------------------------------------------------------------------*/

int dstr_append_cc(DSTR p, char c, size_t count)
{
    dstr_assert_valid(p);
    return dstr_insert_cc_imp(p, DLEN(p), c, count);
}
/*-------------------------------------------------------------------------------*/

int dstr_grow_append_c(DSTR p, char c)
{
    dstr_assert_valid(p);

    if (c == '\0') return DSTR_SUCCESS;

    if (!dstr_grow_by(p, 1)) {
        return DSTR_FAIL;
    }

    DVAL(p, DLEN(p)) = c;
    DLEN(p) += 1;
    DVAL(p, DLEN(p)) = '\0';

    dstr_assert_valid(p);
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

int dstr_append_ds(DSTR dest, CDSTR src)
{
    dstr_assert_valid(dest);
    dstr_assert_view(src);

    return dstr_append_imp(dest, DBUF(src), DLEN(src));
}
/*-------------------------------------------------------------------------------*/

int dstr_append_sz(DSTR p, const char* value)
{
    dstr_assert_valid(p);

    if (value == NULL)
        return DSTR_SUCCESS;

    size_t len = strlen(value);
    return dstr_append_imp(p, value, len);
}
/*-------------------------------------------------------------------------------*/

int dstr_append_bl(DSTR p, const char* buff, size_t len)
{
    dstr_assert_valid(p);

    if (buff == NULL)
        return DSTR_SUCCESS;

    len = strnlen(buff, len);
    return dstr_append_imp(p, buff, len);
}
/*-------------------------------------------------------------------------------*/

int dstr_append_range(DSTR p, const char* first, const char* last)
{
    dstr_assert_valid(p);

    if (first == NULL)
        return DSTR_SUCCESS;

    size_t len = strnlen(first, (last - first));
    return dstr_append_imp(p, first, len);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_find_sz(CDSTR p, size_t pos, const char* s)
{
    return dstr_find_sz_imp(p, pos, s, DSTR_FALSE); /* don't ignore case */
}
/*-------------------------------------------------------------------------------*/

size_t dstr_ifind_sz(CDSTR p, size_t pos, const char* s)
{
    return dstr_find_sz_imp(p, pos, s, DSTR_TRUE);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_find_c(CDSTR p, size_t pos, char c)
{
    return dstr_find_c_imp(p, pos, c, DSTR_FALSE);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_ifind_c(CDSTR p, size_t pos, char c)
{
    return dstr_find_c_imp(p, pos, c, DSTR_TRUE);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_rfind_sz(CDSTR p, size_t pos, const char* s)
{
    return dstr_rfind_sz_imp(p, pos, s, DSTR_FALSE); /* don't ignore case */
}
/*-------------------------------------------------------------------------------*/

size_t dstr_irfind_sz(CDSTR p, size_t pos, const char* s)
{
    return dstr_rfind_sz_imp(p, pos, s, DSTR_TRUE);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_rfind_c(CDSTR p, size_t pos, char c)
{
    return dstr_rfind_c_imp(p, pos, c, DSTR_FALSE);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_irfind_c(CDSTR p, size_t pos, char c)
{
    return dstr_rfind_c_imp(p, pos, c, DSTR_TRUE);
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_contains_sz(CDSTR p, const char* s)
{
    return dstr_find_sz(p, 0, s) != DSTR_NPOS;
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_icontains_sz(CDSTR p, const char* s)
{
    return dstr_ifind_sz(p, 0, s) != DSTR_NPOS;
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_suffix_sz(CDSTR p, const char* s)
{
    /* don't ignore case */
    return dstr_suffix_sz_imp(p, s, 0) >= 0;
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_isuffix_sz(CDSTR p, const char* s)
{
    /* ignore case */
    return dstr_suffix_sz_imp(p, s, 1) >= 0;
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_prefix_sz(CDSTR p, const char* s)
{
    return dstr_prefix_sz_imp(p, s, 0) >= 0;
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_iprefix_sz(CDSTR p, const char* s)
{
    return dstr_prefix_sz_imp(p, s, 1) >= 0;
}
/*-------------------------------------------------------------------------------*/

size_t dstr_ffo_sz(CDSTR p, size_t pos, const char* pattern)
{
    return dstr_ffo_imp(p, pos, pattern, /*not_of*/DSTR_FALSE);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_ffo_ds(CDSTR p, size_t pos, CDSTR pattern)
{
    return dstr_ffo_imp(p, pos, DBUF(pattern), DSTR_FALSE);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_ffno_sz(CDSTR p, size_t pos, const char* s)
{
    return dstr_ffo_imp(p, pos, s, DSTR_TRUE);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_ffno_ds(CDSTR p, size_t pos, CDSTR s)
{
    return dstr_ffo_imp(p, pos, DBUF(s), DSTR_TRUE);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_flo_sz(CDSTR p, size_t pos, const char* pattern)
{
    return dstr_flo_imp(p, pos, pattern, /*not_of*/DSTR_FALSE);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_flo_ds(CDSTR p, size_t pos, CDSTR pattern)
{
    return dstr_flo_imp(p, pos, DBUF(pattern), DSTR_FALSE);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_flno_sz(CDSTR p, size_t pos, const char* s)
{
    return dstr_flo_imp(p, pos, s, DSTR_TRUE);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_flno_ds(CDSTR p, size_t pos, CDSTR s)
{
    return dstr_flo_imp(p, pos, DBUF(s), DSTR_TRUE);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_substr(CDSTR p,
                   size_t pos,
                   size_t count,
                   char dest[],
                   size_t destsize)
{
    dstr_assert_view(p);

    if (pos >= DLEN(p))
        return 0;

    if (count == 0)
        return 0;

    if (destsize == 0)
        return 0;

    /* allow space for zero byte*/
    count = min_3( count, destsize - 1, DLEN(p) - pos );

    if (count > 0)
        memcpy(dest, dstr_address_c(p, pos), count);

    dest[count] = '\0';
    return count + 1;
}
/*-------------------------------------------------------------------------------*/

int dstr_append_vsprintf(DSTR p, const char* fmt, va_list argptr)
{
    int len;
    va_list argptr2;
    char buff[512];

    va_copy(argptr2, argptr);
    len = vsnprintf(buff, sizeof buff, fmt, argptr2);
    va_end(argptr2);

    if (len < 0)
        return DSTR_FAIL;

    // formatting was successful on tmp buffer
    //
    if ((size_t)len < sizeof(buff)) {
        dstr_append_no_overlap(p, buff, (size_t) len);
    }
    else {
        // Second pass with enough buffer size
        //
        if (!dstr_grow_by(p, len))
            return DSTR_FAIL;

        if ((len = vsnprintf(dstr_tail(p), len + 1, fmt, argptr)) < 0) {
            return DSTR_FAIL;
        }

        DLEN(p) += len;
    }

    dstr_assert_valid(p);
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

int dstr_append_sprintf(DSTR p, const char* fmt, ...)
{
    int result;
    va_list argptr;

    va_start(argptr, fmt);
    result = dstr_append_vsprintf(p, fmt, argptr);
    va_end(argptr);

    return result;
}
/*-------------------------------------------------------------------------------*/

int dstr_assign_vsprintf(DSTR p, const char* fmt, va_list argptr)
{
    dstr_assert_valid(p);
    dstr_clear(p);
    return dstr_append_vsprintf(p, fmt, argptr);
}
/*-------------------------------------------------------------------------------*/

int dstr_assign_sprintf(DSTR p, const char* fmt, ...)
{
    int result;
    va_list argptr;

    va_start(argptr, fmt);
    result = dstr_assign_vsprintf(p, fmt, argptr);
    va_end(argptr);

    return result;
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_vsprintf(const char* fmt, va_list argptr)
{
    DSTR result = dstr_alloc_empty();
    if (!result)
        return NULL;

    if (dstr_append_vsprintf(result, fmt, argptr) == DSTR_FAIL) {
        dstr_destroy(result);
        return NULL;
    }

    return result;
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_sprintf(const char* fmt, ...)
{
    DSTR result;
    va_list argptr;

    va_start(argptr, fmt);
    result = dstr_create_vsprintf(fmt, argptr);
    va_end(argptr);

    return result;
}
/*-------------------------------------------------------------------------------*/

int dstr_insert_cc(DSTR p, size_t index, char c, size_t count)
{
    dstr_assert_valid(p);
    return dstr_insert_cc_imp(p, index, c, count);
}
/*-------------------------------------------------------------------------------*/

int dstr_insert_sz(DSTR p, size_t index, const char* value)
{
    dstr_assert_valid(p);

    if (value == NULL)
        return DSTR_SUCCESS;

    return dstr_insert_imp(p, index, value, strlen(value));
}
/*-------------------------------------------------------------------------------*/

int dstr_insert_ds(DSTR dest, size_t index, CDSTR src)
{
    dstr_assert_valid(dest);
    dstr_assert_view(src);
    return dstr_insert_imp(dest, index, DBUF(src), DLEN(src));
}
/*-------------------------------------------------------------------------------*/

int dstr_insert_bl(DSTR p, size_t index, const char* buff, size_t len)
{
    dstr_assert_valid(p);

    /* do nothing*/
    if (buff == NULL || len == 0)
        return DSTR_SUCCESS;

    if (len > PTRDIFF_MAX) len = PTRDIFF_MAX;
    len = strnlen(buff, len);
    return dstr_insert_imp(p, index, buff, len);
}
/*-------------------------------------------------------------------------------*/

int dstr_insert_range(DSTR p, size_t index, const char* first, const char* last)
{
    dstr_assert_valid(p);

    size_t len = last - first;
    if (first == NULL || len == 0)
        return DSTR_SUCCESS;

    len = strnlen(first, len);

    return dstr_insert_imp(p, index, first, len);
}
/*-------------------------------------------------------------------------------*/

int dstr_replace_cc(DSTR p, size_t pos, size_t count_old, char c, size_t count_new)
{
    dstr_assert_valid(p);
    return dstr_replace_cc_imp(p, pos, count_old, c, count_new);
}
/*-------------------------------------------------------------------------------*/

int dstr_replace_sz(DSTR p, size_t pos, size_t len, const char* value)
{
    dstr_assert_valid(p);
    return dstr_replace_imp(p, pos, len, value, (value ? strlen(value) : 0));
}
/*-------------------------------------------------------------------------------*/

int dstr_replace_ds(DSTR dest, size_t pos, size_t len, CDSTR src)
{
    dstr_assert_valid(dest);
    dstr_assert_view(src);
    return dstr_replace_imp(dest, pos, len, DBUF(src), DLEN(src));
}
/*-------------------------------------------------------------------------------*/

int dstr_replace_bl(DSTR p, size_t pos, size_t count, const char* buff, size_t len)
{
    dstr_assert_valid(p);

    /* check for null inside */
    return dstr_replace_imp(p, pos, count, buff, len);
}
/*-------------------------------------------------------------------------------*/

int dstr_replace_range(DSTR p, size_t pos, size_t count, const char* first, const char* last)
{
    dstr_assert_valid(p);

    /* check for null inside */
    return dstr_replace_imp(p, pos, count, first, (last -first));
}
/*-------------------------------------------------------------------------------*/

void dstr_trim_right(DSTR p)
{
    size_t len;
    dstr_assert_valid(p);

    if ((len = DLEN(p)) == 0)
        return;

    while (len > 0 && isspace(DVAL(p,len - 1)))
        --len;

    if (len < DLEN(p)) {
        DVAL(p, len) = '\0';
        DLEN(p) = len;
    }

    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

void dstr_trim_left(DSTR p)
{
    size_t pos = 0;
    dstr_assert_valid(p);

    while (pos < DLEN(p) && isspace(DVAL(p, pos)))
        ++pos;

    if (pos > 0) {
        if (pos == DLEN(p))
            dstr_clear(p);
        else
            dstr_remove_imp(p, 0, pos);
    }

    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

void dstr_trim(DSTR p)
{
    dstr_trim_left(p);
    dstr_trim_right(p);
}
/*-------------------------------------------------------------------------------*/

void dstr_rstrip_c(DSTR p, char c)
{
    size_t len;
    dstr_assert_valid(p);

    if ((len = DLEN(p)) == 0)
        return;

    while (len > 0 && (DVAL(p,len - 1) == c))
        --len;

    if (len < DLEN(p)) {
        DVAL(p, len) = '\0';
        DLEN(p) = len;
    }

    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

void dstr_lstrip_c(DSTR p, char c)
{
    size_t pos = 0;
    dstr_assert_valid(p);

    while (pos < DLEN(p) && (DVAL(p, pos) == c))
        ++pos;

    if (pos > 0) {
        if (pos == DLEN(p))
            dstr_clear(p);
        else
            dstr_remove_imp(p, 0, pos);
    }
    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

void dstr_rstrip_sz(DSTR p, const char* sz)
{
    dstr_assert_valid(p);

    size_t pos = dstr_flno_sz(p, DLEN(p), sz);
    if (pos == DSTR_NPOS) {
        dstr_clear(p);
    }
    else if (pos < DLEN(p)) {
        DVAL(p, pos + 1) = '\0';
        DLEN(p) = pos + 1;
    }

    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

void dstr_lstrip_sz(DSTR p, const char* sz)
{
    dstr_assert_valid(p);

    size_t pos = dstr_ffno_sz(p, 0, sz);
    if (pos == DSTR_NPOS) {
        dstr_clear(p);
    }
    else {
        dstr_remove_imp(p, 0, pos);
    }

    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

void dstr_ascii_upper(DSTR p)
{
    char* s;

    dstr_assert_valid(p);

    for (s = DBUF(p); *s; ++s)
        *s = (char)toupper(*s);

    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

void dstr_ascii_lower(DSTR p)
{
    char* s;

    dstr_assert_valid(p);

    for (s = DBUF(p); *s; ++s)
        *s = (char)tolower(*s);

    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

void dstr_ascii_swapcase(DSTR p)
{
    char* s;

    dstr_assert_valid(p);

    for (s = DBUF(p); *s; ++s) {
        char c = *s;

        if (!isalpha(c))
            continue;

        if (islower(c)) {
            *s = toupper(c);
        }
        else if (isupper(c)) {
            *s = tolower(c);
        }
    }

    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

void dstr_reverse(DSTR p)
{
    size_t first;
    size_t last;

    dstr_assert_valid(p);

    first = 0;
    last = DLEN(p) - 1;

    while (first < last) {
        char c = DVAL(p, first);
        DVAL(p, first) = DVAL(p, last);
        DVAL(p, last) = c;
        ++first;
        --last;
    }

    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

static inline void bitcopy_dstr(DSTR_TYPE* dest, const DSTR_TYPE* src)
{
    *dest = *src;

    if (src->capacity == DSTR_INITIAL_CAPACITY) {
        dest->capacity = src->capacity;
        dest->data = dest->sso_buffer;
    }
}
/*-------------------------------------------------------------------------------*/

void dstr_swap(DSTR d1, DSTR d2)
{
    dstr_assert_valid(d1);
    dstr_assert_valid(d2);

    DSTR_TYPE tmp;
    bitcopy_dstr(&tmp, d1);
    bitcopy_dstr(d1, d2);
    bitcopy_dstr(d2, &tmp);

    dstr_assert_valid(d1);
    dstr_assert_valid(d2);
}
/*-------------------------------------------------------------------------------*/

int dstr_fgets(DSTR p, FILE* fp)
{
    int c;
    char buf[32];
    size_t bindex = 0;

    dstr_assert_valid(p);
    assert(fp != NULL);

    dstr_clear(p);

    /* skip blanks */
    do {
        if ((c = fgetc(fp)) == EOF)
            return EOF;
    } while (isspace(c) || c == '\0');

    /* read until next blank characters */
    for (;;) {
        buf[bindex] = c;
        if (++bindex == sizeof(buf)) {
            if (!dstr_append_no_overlap(p, buf, bindex))
                goto clear_fail;
            bindex = 0;
        }

        if ((c = fgetc(fp)) == EOF)
            break;

        if (isspace(c) || c == '\0') {
            ungetc(c, fp);
            break;
        }
    }

    if (bindex) {
        if (!dstr_append_no_overlap(p, buf, bindex))
            goto clear_fail;
    }

    return DSTR_SUCCESS;

clear_fail:
    dstr_clear(p);
    return EOF;
}
/*-------------------------------------------------------------------------------*/

int dstr_fgetline(DSTR p, FILE* fp)
{
    int c;

    dstr_assert_valid(p);
    dstr_clear(p);

    // We append to DSTR in chunks. In most cases only one chunk will be
    // needed
    //
    char buf[128];
    size_t bindex = 0;

    while ((c = fgetc(fp)) != EOF && c != '\n' && c != '\0') {
        buf[bindex] = c;
        if (++bindex == sizeof(buf)) {
            if (!dstr_append_no_overlap(p, buf, bindex)) {
                dstr_clear(p);
                return EOF;
            }
            bindex = 0;
        }
    }

    if (bindex) {
        if (!dstr_append_no_overlap(p, buf, bindex)) {
            dstr_clear(p);
            return EOF;
        }
    }

    if (DLEN(p) > 0 || (c == '\n') || (c == '\0'))
        return DSTR_SUCCESS;

    return EOF;
}
/*-------------------------------------------------------------------------------*/

// FNV-1 hash algorithm
//
// See: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
//
unsigned long dstr_hash(CDSTR src, int seed)
{
    dstr_assert_view(src);

    unsigned long h = seed;

    size_t len = DLEN(src);
    const char* data = DBUF(src);

    h ^= 2166136261;

    for (unsigned int i = 0; i < len; i++) {
        h ^= data[i];
        h *= 16777619;
    }

    return h;
}
/*-------------------------------------------------------------------------------*/

static char* itos_aux(char* last, unsigned long long n, unsigned int base)
{
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    do {
        *--last = digits[n % base];
        n /= base;
    } while (n);

    return last;
}
/*-------------------------------------------------------------------------------*/

int dstr_itos_ul(DSTR dest, unsigned long long n, unsigned int base)
{
    char buf[64];
    char* last = buf + sizeof buf;
    char* first = itos_aux(last, n, base);
    size_t len = last - first;
    if (DLEN(dest)) dstr_clear(dest);
    return dstr_append_no_overlap(dest, first, len);

}
//---------------------------------------------------------

int dstr_itos(DSTR dest, long long n)
{
    DSTR_BOOL negative = 0;

    if (n < 0) {
        n = -n;
        negative = 1;
    }

    char buf[64];
    char* last  = buf + sizeof buf;
    char* first = itos_aux(last, n, 10);

    if (negative)
        *--first = '-';

    size_t len = last - first;

    if (DLEN(dest))
        dstr_clear(dest);

    return dstr_append_no_overlap(dest, first, len);
}
/*-------------------------------------------------------------------------------*/

long dstr_atoi(CDSTR src)
{
    const char* p;
    int base = 10;

    dstr_assert_view(src);

    p = DBUF(src);

    if (*p == '\\') {
        ++p;
        base = 8;
    }
    else if (*p == '0') {
        if (p[1] == 'b' || p[1] == 'B') {
            p += 2;
            base = 2;
        }
        else if (p[1] == 'x' || p[1] == 'X') {
            p += 2;
            base = 16;
        }
        else {
            /* skip all leading zero's */
            do { ++p; } while (*p == '0');
        }
    }

    return strtol(p, NULL, base);
}
/*-------------------------------------------------------------------------------*/

double dstr_atof(CDSTR src)
{
    dstr_assert_view(src);
    return atof(DBUF(src));
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_isdigits(CDSTR src)
{
    return dstr_isdigits_imp(src, DSTR_FALSE);
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_isxdigits(CDSTR src)
{
    return dstr_isdigits_imp(src, DSTR_TRUE);
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_isalnum(CDSTR p)
{
    dstr_assert_view(p);

    if (DLEN(p) == 0)
        return DSTR_FALSE;

    for (size_t n = 0; n < DLEN(p); ++n) {
        if (!isalnum(DVAL(p, n)))
            return DSTR_FALSE;
    }

    return DSTR_TRUE;
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_isalpha(CDSTR p)
{
    dstr_assert_view(p);

    if (DLEN(p) == 0)
        return DSTR_FALSE;

    for (size_t n = 0; n < DLEN(p); ++n) {
        if (!isalpha(DVAL(p, n)))
            return DSTR_FALSE;
    }

    return DSTR_TRUE;
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_isascii(CDSTR p)
{
    dstr_assert_view(p);

    if (DLEN(p) == 0)
        return DSTR_FALSE;

    for (size_t n = 0; n < DLEN(p); ++n) {
        if (!isascii(DVAL(p, n)))
            return DSTR_FALSE;
    }

    return DSTR_TRUE;
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_isblank(CDSTR p)
{
    dstr_assert_view(p);

    if (DLEN(p) == 0)
        return DSTR_FALSE;

    for (size_t n = 0; n < DLEN(p); ++n) {
        if (!isblank(DVAL(p, n)))
            return DSTR_FALSE;
    }

    return DSTR_TRUE;
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_isdecimal(CDSTR p)
{
    return dstr_isdigits(p);
}
/*-------------------------------------------------------------------------------*/

static inline bool is_ident_(char c) { return isalnum(c) || c == '_'; }

DSTR_BOOL dstr_isidentifier(CDSTR p)
{
    dstr_assert_view(p);

    if (DLEN(p) == 0)
        return DSTR_FALSE;

    if (isdigit(DVAL(p, 0)))
        return DSTR_FALSE;

    for (size_t n = 0; n < DLEN(p); ++n) {
        if (!is_ident_(DVAL(p, n)))
            return DSTR_FALSE;
    }

    return DSTR_TRUE;
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_isnumeric(CDSTR p)
{
    return dstr_isdigits(p);
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_isprintable(CDSTR p)
{
    dstr_assert_view(p);

    if (DLEN(p) == 0)
        return DSTR_FALSE;

    for (size_t n = 0; n < DLEN(p); ++n) {
        if (!isprint(DVAL(p, n)))
            return DSTR_FALSE;
    }

    return DSTR_TRUE;
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_isspace(CDSTR p)
{
    dstr_assert_view(p);

    if (DLEN(p) == 0)
        return DSTR_FALSE;

    for (size_t n = 0; n < DLEN(p); ++n) {
        if (!isspace(DVAL(p, n)))
            return DSTR_FALSE;
    }

    return DSTR_TRUE;
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_istitle(CDSTR p)
{
    bool curr_is_alpha = false;

    for (size_t pos = 0; pos < DLEN(p); ++pos)
    {
        char ch = DVAL(p, pos);
        bool prev_is_alpha = curr_is_alpha;
        curr_is_alpha = isalpha(ch);

        if (curr_is_alpha  &&
            !prev_is_alpha &&
            !isupper(ch))
        {
            return DSTR_FALSE;
        }
    }

    return DSTR_TRUE;
}
/*-------------------------------------------------------------------------------*/

//  01234   -> false
//  01234AB -> true
//
DSTR_BOOL dstr_isupper(CDSTR p)
{
    dstr_assert_view(p);

    if (DLEN(p) == 0)
        return DSTR_FALSE;

    size_t upper_count = 0;
    for (size_t n = 0; n < DLEN(p); ++n) {
        char c = DVAL(p, n);
        if (isalpha(c)) {
            if (!isupper(c)) {
                return DSTR_FALSE; }
            else {
                ++upper_count; }
        }
    }

    return upper_count > 0;
}
/*-------------------------------------------------------------------------------*/

//  01234   -> false
//  01234ab -> true
//
DSTR_BOOL dstr_islower(CDSTR p)
{
    dstr_assert_view(p);

    if (DLEN(p) == 0)
        return DSTR_FALSE;

    size_t lower_count = 0;
    for (size_t n = 0; n < DLEN(p); ++n) {
        char ch = DVAL(p, n);
        if (isalpha(ch)) {
            if (!islower(ch)) {
                return DSTR_FALSE; }
            else {
                ++lower_count; }
        }
    }

    return lower_count > 0;
}
/*-------------------------------------------------------------------------------*/

int dstr_to_int(CDSTR p, size_t* index, int base)
{
    int errsave = errno;
    errno = 0;

    dstr_assert_view(p);
    const char* str = dstr_cstr(p);

    char* endp;
    int result = strtol(str, &endp, base);

    if (endp == str)
        errno = EINVAL;

    if (index) {
        *index = endp - str;
    }

    if (errno == 0)
        errno = errsave;

    return result;
}
/*-------------------------------------------------------------------------------*/

long dstr_to_long(CDSTR p, size_t* index, int base)
{
    int errsave = errno;
    errno = 0;

    dstr_assert_view(p);
    const char* str = dstr_cstr(p);

    char* endp;
    long result = strtol(str, &endp, base);

    if (endp == str)
        errno = EINVAL;

    if (index) {
        *index = endp - str;
    }

    if (errno == 0)
        errno = errsave;

    return result;
}
/*-------------------------------------------------------------------------------*/

unsigned long dstr_to_ulong(CDSTR p, size_t* index, int base)
{
    int errsave = errno;
    errno = 0;

    dstr_assert_view(p);
    const char* str = dstr_cstr(p);

    char* endp;
    unsigned long result = strtoul(str, &endp, base);

    if (endp == str)
        errno = EINVAL;

    if (index) {
        *index = endp - str;
    }

    if (errno == 0)
        errno = errsave;

    return result;
}
/*-------------------------------------------------------------------------------*/

long long dstr_to_llong(CDSTR p, size_t* index, int base)
{
    int errsave = errno;
    errno = 0;

    dstr_assert_view(p);
    const char* str = dstr_cstr(p);

    char* endp;
    long long result = strtoll(str, &endp, base);

    if (endp == str)
        errno = EINVAL;

    if (index) {
        *index = endp - str;
    }

    if (errno == 0)
        errno = errsave;

    return result;
}
/*-------------------------------------------------------------------------------*/

unsigned long long dstr_to_ullong(CDSTR p, size_t* index, int base)
{
    int errsave = errno;
    errno = 0;

    dstr_assert_view(p);
    const char* str = dstr_cstr(p);

    char* endp;
    unsigned long long result = strtoull(str, &endp, base);

    if (endp == str)
        errno = EINVAL;

    if (index) {
        *index = endp - str;
    }

    if (errno == 0)
        errno = errsave;

    return result;
}
/*-------------------------------------------------------------------------------*/

float dstr_to_float(CDSTR p, size_t* index)
{
    int errsave = errno;
    errno = 0;

    dstr_assert_view(p);
    const char* str = dstr_cstr(p);

    char* endp;
    float result = strtof(str, &endp);

    if (endp == str)
        errno = EINVAL;

    if (index) {
        *index = endp - str;
    }

    if (errno == 0)
        errno = errsave;

    return result;
}
/*-------------------------------------------------------------------------------*/

double dstr_to_double(CDSTR p, size_t* index)
{
    int errsave = errno;
    errno = 0;

    dstr_assert_view(p);
    const char* str = dstr_cstr(p);

    char* endp;
    double result = strtod(str, &endp);

    if (endp == str)
        errno = EINVAL;

    if (index) {
        *index = endp - str;
    }

    if (errno == 0)
        errno = errsave;

    return result;
}
/*-------------------------------------------------------------------------------*/

long double dstr_to_ldouble(CDSTR p, size_t* index)
{
    int errsave = errno;
    errno = 0;

    dstr_assert_view(p);
    const char* str = dstr_cstr(p);

    char* endp;
    long double result = strtold(str, &endp);

    if (endp == str)
        errno = EINVAL;

    if (index) {
        *index = endp - str;
    }

    if (errno == 0)
        errno = errsave;

    return result;
}
/*-------------------------------------------------------------------------------*/

int dstr_align_center(DSTR dest, size_t width, char fill)
{
    dstr_assert_valid(dest);

    size_t slen = DLEN(dest);
    if (slen >= width)
        return DSTR_SUCCESS;

    size_t left_side  = (width - slen) / 2;
    size_t right_side = width - slen - left_side;
    assert(right_side > 0);

    if (left_side) {
        if (!dstr_insert_cc_imp(dest, 0, fill, left_side))
            return DSTR_FAIL;
    }

    if (!dstr_insert_cc_imp(dest, DLEN(dest), fill, right_side))
        return DSTR_FAIL;

    // sanity
    //
    assert(dstr_length(dest) == width);
    dstr_assert_valid(dest);

    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

int dstr_align_right(DSTR dest, size_t width, char fill)
{
    dstr_assert_valid(dest);

    size_t slen = DLEN(dest);
    if (slen >= width)
        return DSTR_SUCCESS;

    size_t left_side  = width - slen;

    if (!dstr_insert_cc_imp(dest, 0, fill, left_side))
        return DSTR_FAIL;

    // sanity
    //
    assert(dstr_length(dest) == width);
    dstr_assert_valid(dest);

    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

int dstr_align_left(DSTR dest, size_t width, char fill)
{
    dstr_assert_valid(dest);

    size_t slen = DLEN(dest);
    if (slen >= width)
        return DSTR_SUCCESS;

    size_t right_side = width - slen;

    if (!dstr_insert_cc_imp(dest, DLEN(dest), fill, right_side))
        return DSTR_FAIL;

    // sanity
    //
    assert(dstr_length(dest) == width);
    dstr_assert_valid(dest);

    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

static int dstr_replace_all_imp(DSTR dest,
                                const char* oldstr, size_t oldlen,
                                const char* newstr, size_t newlen,
                                size_t count)
{
    size_t pos = 0;
    size_t num_replaced = 0;

    // We will work on a copy so if a failure occurs in the middle of replace
    // we don't leave the original argument in an undefined status
    //
    INIT_DSTR(tmp);
    if (!dstr_assign_ds(&tmp, dest)) {
        return DSTR_FAIL; }

    for (;;) {
        pos = dstr_find_sz_imp(&tmp, pos, oldstr, 0);
        if (pos == DSTR_NPOS)
            break;
        if (!dstr_replace_imp(&tmp, pos, oldlen, newstr, newlen)) {
            dstr_clean_data(&tmp);
            return DSTR_FAIL; }
        if (++num_replaced == count)
            break;
        pos += newlen;
    };

    if (num_replaced > 0) {
        dstr_swap(&tmp, dest); }

    DONE_DSTR(tmp);
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

int dstr_replace_all_sz(DSTR dest, const char* oldstr, const char* newstr, size_t count)
{
    if (!count)  return DSTR_SUCCESS;
    if (!oldstr) return DSTR_SUCCESS;
    if (!newstr) return DSTR_SUCCESS;
    if (*oldstr == '\0') return DSTR_SUCCESS;
    if (*newstr == '\0') return DSTR_SUCCESS;

    size_t oldlen = strlen(oldstr);
    size_t newlen = strlen(newstr);

    return dstr_replace_all_imp(dest, oldstr, oldlen, newstr, newlen, count);
}
/*-------------------------------------------------------------------------------*/

int dstr_replace_all_ds(DSTR dest, CDSTR oldstr, CDSTR newstr, size_t count)
{
    if (!count)  return DSTR_SUCCESS;
    if (!oldstr) return DSTR_SUCCESS;
    if (!newstr) return DSTR_SUCCESS;

    size_t oldlen = DLEN(oldstr);
    size_t newlen = DLEN(newstr);
    if (oldlen == 0) return DSTR_SUCCESS;
    if (newlen == 0) return DSTR_SUCCESS;

    return dstr_replace_all_imp(dest,
                                DBUF(oldstr), oldlen,
                                DBUF(newstr), newlen,
                                count);
}
/*-------------------------------------------------------------------------------*/

// returns the number of non-overlapping occurrences of a substring within the string
//
static size_t dstr_count_aux(CDSTR p, const char* s, size_t slen, int ignore_case)
{
    size_t pos = 0;
    size_t num_found = 0;

    for (;;) {
        pos = dstr_find_sz_imp(p, pos, s, ignore_case);
        if (pos == DSTR_NPOS)
            break;

        pos += slen;
        ++num_found;
    }

    return num_found;
}
/*-------------------------------------------------------------------------------*/

size_t dstr_count_sz(CDSTR p, const char* s)
{
    if (!p) return 0;
    if (!s) return 0;
    if (*s == '\0') return (DLEN(p) + 1);
    if (DLEN(p) == 0) return 0;

    return dstr_count_aux(p, s, strlen(s), 0);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_count_ds(CDSTR p, CDSTR s)
{
    if (!p) return 0;
    if (!s) return 0;
    if (DLEN(s) == 0) return (DLEN(p) + 1);
    if (DLEN(p) == 0) return 0;

    return dstr_count_aux(p, DBUF(s), DLEN(s), 0);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_icount_sz(CDSTR p, const char* s)
{
    if (!p) return 0;
    if (!s) return 0;
    if (*s == '\0') return (DLEN(p) + 1);
    if (DLEN(p) == 0) return 0;

    return dstr_count_aux(p, s, strlen(s), 1);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_icount_ds(CDSTR p, CDSTR s)
{
    if (!p) return 0;
    if (!s) return 0;
    if (DLEN(s) == 0) return (DLEN(p) + 1);
    if (DLEN(p) == 0) return 0;

    return dstr_count_aux(p, DBUF(s), DLEN(s), 1);
}
/*-------------------------------------------------------------------------------*/

int dstr_expand_tabs(DSTR dest, size_t width)
{
    dstr_assert_valid(dest);
    if (!dest || DLEN(dest) == 0) {
        return DSTR_SUCCESS; }

    INIT_DSTR(tmp);
    for (size_t pos = 0; pos < DLEN(dest); ++pos) {
        char ch = DVAL(dest, pos);
        if (ch == '\t') {
            if (width == 0) {
                continue; }
            size_t to_tabstop = width - (DLEN(&tmp) % width);
            if (!dstr_append_cc(&tmp, ' ', to_tabstop)) {
                dstr_clean_data(&tmp);
                return DSTR_FAIL; } }
        else {
            if (DLEN(&tmp) + 1 == DCAP(&tmp)) {
                if (!dstr_grow_by(&tmp, 1)) {
                    dstr_clean_data(&tmp);
                    return DSTR_FAIL; } }
            DVAL(&tmp, DLEN(&tmp)) = ch;
            DLEN(&tmp) += 1;
            DVAL(&tmp, DLEN(&tmp)) = '\0'; } }

    dstr_swap(&tmp, dest);
    DONE_DSTR(tmp);
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

int dstr_zfill(DSTR dest, size_t width)
{
    dstr_assert_valid(dest);

    if (DLEN(dest) >= width)
        return DSTR_SUCCESS;

    size_t num_zeroes = width - DLEN(dest);
    size_t insert_pos = 0;

    char ch = DVAL(dest, 0);
    if (ch == '+' || ch == '-') {
        ++insert_pos;
    }

    if (!dstr_insert_cc_imp(dest, insert_pos, '0', num_zeroes))
        return DSTR_FAIL;

    dstr_assert_valid(dest);
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

void dstr_title(DSTR p)
{
    bool curr_is_alpha = false;

    for (size_t pos = 0; pos < DLEN(p); ++pos)
    {
        char ch = DVAL(p, pos);
        bool prev_is_alpha = curr_is_alpha;
        curr_is_alpha = isalpha(ch);

        if (curr_is_alpha && !prev_is_alpha)
        {
            DVAL(p, pos) = (char) toupper(ch);
        }
    }
}
/*-------------------------------------------------------------------------------*/

int dstr_join_sz(DSTR dest, const char* sep, const char* argv[], size_t n)
{
    // Nothing to join
    //
    if (!argv || !argv[0] || n == 0)
        return DSTR_SUCCESS;;

    // NULL is like empty separator. i.e. concatenation
    //
    if (!sep)
        sep = "";

    size_t len = strlen(sep);
    size_t index = 0;

    for (;;) {
        if (!dstr_append_sz(dest, argv[index]))
            return DSTR_FAIL;

        if ((++index == n) || (argv[index] == NULL))
            break;

        if (!dstr_append_imp(dest, sep, len))
            return DSTR_FAIL;
    }

    return DSTR_SUCCESS;
}
/*--------------------------------------------------------------------------*/

int dstr_join_ds(DSTR dest, CDSTR sep, const char* argv[], size_t n)
{
    // Nothing to join
    //
    if (!argv || !argv[0] || n == 0)
        return DSTR_SUCCESS;;

    size_t index = 0;

    for (;;) {
        if (!dstr_append_sz(dest, argv[index]))
            return DSTR_FAIL;

        if ((++index == n) || (argv[index] == NULL))
            break;

        if (!dstr_append_imp(dest, DBUF(sep), DLEN(sep)))
            return DSTR_FAIL;
    }

    return DSTR_SUCCESS;
}
/*--------------------------------------------------------------------------*/

int dstr_multiply(DSTR dest, size_t n)
{
    dstr_assert_valid(dest);

    if (n == 0) {
        dstr_clear(dest);
        return DSTR_SUCCESS; }
    else if (n == 1) {
        return DSTR_SUCCESS; }

    INIT_DSTR(tmp);
    if (!dstr_reserve(&tmp, n * DLEN(dest)))
        return DSTR_FAIL;

    for (size_t i = 0; i < n; ++i) {
        if (!dstr_append_no_overlap(&tmp, DBUF(dest), DLEN(dest))) {
            dstr_clean_data(&tmp);
            return DSTR_FAIL; } }

    dstr_swap(&tmp, dest);
    DONE_DSTR(tmp);

    dstr_assert_valid(dest);
    return DSTR_SUCCESS;
}
/*--------------------------------------------------------------------------*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   Traslate, delete and squeeze (like "tr" command)
 *   and Ruby tr()
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static void
make_deletion_set(const char* tr_set, uint8_t tr_table[], size_t tlen)
{
    char negate = 0;

    if (*tr_set == '^') {
        negate = 1;
        ++tr_set; }

    memset(tr_table, negate, tlen);

    for (int i  = 0; tr_set[i] ; ++i) {
        char c = tr_set[i];
        if (c == '\\') {
            if (i > 0 && tr_set[i-1] == '\\')
                tr_table[(uint8_t)(c)] = !negate;
            continue;  }

        if (c != '-') {
            tr_table[(uint8_t)(c)] = !negate;
            continue; }

        if (i == 0 || tr_set[i+1] == '-' || tr_set[i+1] == '\0') {
            tr_table[(uint8_t)(c)] = !negate;
            continue; }

        if (tr_set[i-1] == '\\') {
            tr_table[(uint8_t)(c)] = !negate;
            continue; }

        if (tr_set[i-1] > tr_set[i+1]) {
            for (char ch = tr_set[i-1]; ch >= tr_set[i+1]; --ch) {
                tr_table[(uint8_t)ch] = !negate; }
            continue; }

        for (char ch = tr_set[i-1]; ch <= tr_set[i+1]; ++ch) {
            tr_table[(uint8_t)ch] = !negate; }
    }
}
/*--------------------------------------------------------------------------*/

static void dstr_translate_delete_aux(DSTR dest, const char* arr1)
{
    assert(arr1 != NULL);

    uint8_t to_delete[256];
    make_deletion_set(arr1, to_delete, sizeof(to_delete));

    size_t read_index = 0;
    size_t write_index = 0;

    // Do translation from table
    //
    for ( ; read_index < DLEN(dest); ++read_index) {
        char ch = DVAL(dest, read_index);
        int deleted = to_delete[(uint8_t) ch];

        if (deleted)
            continue;

        DVAL(dest, write_index) = ch;
        ++write_index; }

    DVAL(dest, write_index) = '\0';
    DLEN(dest) = write_index;
    dstr_assert_valid(dest);
}
/*--------------------------------------------------------------------------*/

static bool expand_tr_str(DSTR dest, const char* src1, bool allow_negate)
{
    bool negate = false;
    const char* src = src1;

    if (allow_negate && *src == '^') {
        ++src;
        negate = true;  }

    for (int i  = 0; src[i] ; ++i) {
        char c = src[i];
        if (c == '\\') {
            if (i > 0 && src[i-1] == '\\')
                dstr_append_char(dest, c);;
            continue;  }

        if (c != '-') {
            dstr_append_char(dest, c);;
            continue; }

        if (i == 0 || src[i+1] == '-' || src[i+1] == '\0') {
            dstr_append_char(dest, c);;
            continue; }

        if (src[i-1] == '\\') {
            dstr_append_char(dest, c);;
            continue; }

        if (src[i-1] > src[i+1]) {
            for (char ch = src[i-1] + 1; ch > src[i+1]; --ch) {
                dstr_append_char(dest, ch); }
            continue; }

        for (char ch = src[i-1] + 1; ch < src[i+1]; ++ch) {
            dstr_append_char(dest, ch); }
    }

    return negate;
}
/*--------------------------------------------------------------------------*/

static void
make_tr_table(const char* from, const char* to, uint8_t tbl[], size_t tlen)
{
    INIT_DSTR(dfrom);
    bool negate = expand_tr_str(&dfrom, from, 1 /*enable_negate*/);

    INIT_DSTR(dto);
    expand_tr_str(&dto, to,  0 /*disable_negate*/);

    if (negate) {
        char back = dstrback(&dto);
        memset(tbl, back, tlen);
        for (size_t i = 0; i < DLEN(&dfrom); ++i) {
            uint8_t ch = DVAL(&dfrom, i);
            tbl[ch] = ch; } }
    else {
        memset(tbl, 0, tlen);
        if (dstrlen(&dto) == 0) {
            dstr_append_no_overlap(&dto, dstrdata(&dfrom), dstrlen(&dfrom)); }
        else if (dstrlen(&dfrom) > dstrlen(&dto)) {
            char back = dstrback(&dto);
            dstr_append_cc(&dto, back, (DLEN(&dfrom) - DLEN(&dto)));  }
        else if (dstrlen(&dfrom) == dstrlen(&dto)) {
            ; }  // do nothing
        else {
            dstr_resize(&dto, DLEN(&dfrom));  }

        for (size_t i = 0; i < DLEN(&dfrom); ++i) {
            uint8_t c = DVAL(&dfrom, i);
            tbl[c] = DVAL(&dto, i); } }

    DONE_DSTR(dfrom);
    DONE_DSTR(dto);
}
/*--------------------------------------------------------------------------*/

void dstr_translate(DSTR dest, const char* arr1, const char* arr2)
{
    if (!dest)
        return;
    if (!arr1 || *arr1 == '\0')
        return;
    if (!arr2) {
        dstr_translate_delete_aux(dest, arr1);
        return;  }

    /* No change if replacement string empty */
    if (*arr2 == '\0') {
        return; }

    uint8_t tbl[256];
    make_tr_table(arr1, arr2, tbl, sizeof(tbl));

    for (size_t index = 0; index < dstr_length(dest); ++index) {
        char c_old = dstr_getchar(dest, index);
        char c_new = tbl[(uint8_t) c_old];
        if (c_new) {
            dstr_putchar(dest, index, c_new); } }
 }
/*--------------------------------------------------------------------------*/

void dstr_squeeze(DSTR dest, const char* sqzset)
{
    if (!dest) return;
    uint8_t to_squeeze[256];

    if (!sqzset || *sqzset == '\0') {
        // For empty string or NULL we squeeze every consequtive charachter
        // run.
        //
        memset(to_squeeze, 1, sizeof(to_squeeze));  }
    else {
        make_deletion_set(sqzset, to_squeeze, sizeof(to_squeeze));  }

    size_t read_index = 0;
    size_t write_index = 0;

    // Do translation from table
    //
    char prev_ch = '\0';
    for ( ; read_index < DLEN(dest); ++read_index) {
        char ch = DVAL(dest, read_index);
        if (ch != prev_ch || !to_squeeze[(uint8_t)ch]) {
            DVAL(dest, write_index) = ch;
            ++write_index; }
        prev_ch = ch; }

    DVAL(dest, write_index) = '\0';
    DLEN(dest) = write_index;
    dstr_assert_valid(dest);
}
/*--------------------------------------------------------------------------*/

void dstr_translate_squeeze(DSTR dest, const char* arr1, const char* arr2)
{
    dstr_translate(dest, arr1, arr2);
    dstr_squeeze(dest, arr2);
}
/*--------------------------------------------------------------------------*/

size_t dstr_partition(CDSTR p, const char* s, struct DSTR_PartInfo* pInfo)
{
    dstr_assert_view(p);

    if (!s)
        s = "";

    size_t len = strnlen(s, DLEN(p));

    size_t pos =
        (len == 0) ?
        0 :
        dstr_find_sz(p, 0, s);

    if (pos == DSTR_NPOS) {
        pos = DLEN(p);
        len = 0;
    }

    if (pInfo) {
        pInfo->l_pos = 0;
        pInfo->l_len = pos;
        pInfo->m_pos = pos;
        pInfo->m_len = len;
        pInfo->r_pos = pos + len;
        pInfo->r_len = DLEN(p) - (pos + len);

        assert((pInfo->l_len + pInfo->m_len + pInfo->r_len) == DLEN(p));
        return 0;
    }

    return pos;
}
/*--------------------------------------------------------------------------*/

size_t dstr_rpartition(CDSTR p, const char* s, struct DSTR_PartInfo* pInfo)
{
    dstr_assert_view(p);

    if (!s) s = "";

    size_t len = strnlen(s, DLEN(p));

    size_t pos =
        (len == 0) ?
        DLEN(p) :
        dstr_rfind_sz(p, DLEN(p), s);

    if (pos == DSTR_NPOS) {
        pos = 0;
        len = 0; }

    if (pInfo) {
        pInfo->l_pos = 0;
        pInfo->l_len = pos;
        pInfo->m_pos = pos;
        pInfo->m_len = len;
        pInfo->r_pos = pos + len;
        pInfo->r_len = DLEN(p) - (pos + len);

        assert((pInfo->l_len + pInfo->m_len + pInfo->r_len) == DLEN(p));
        return 0;
    }

    return pos;
}
/*--------------------------------------------------------------------------*/

static DSTR_BOOL same_carry_after_puncts(CDSTR p, long pos, int carry)
{
    char c = '\0';
    for (; pos >= 0; --pos) {
        c = DVAL(p, pos);
        if (isalnum(c)) break; }

    return
        isalnum(c) &&
        ((isdigit(c) && isdigit(carry)) ||
         (isalpha(c) && isalpha(carry)));
}
/*--------------------------------------------------------------------------*/

// To make more readable
//
enum {
    NO_CARRY = 0
};

static int dstr_increment_alnum(DSTR dest)
{
    int carry = NO_CARRY;
    int only_alnum = 1;

    for (long pos = DLEN(dest) - 1; pos >= 0; --pos) {
        char c = DVAL(dest, pos);
        if (!isalnum(c)) {
            if (carry && only_alnum) {
                if (!same_carry_after_puncts(dest, pos, carry)) {
                    dstr_insert_cc_imp(dest, pos + 1, carry, 1);
                    return NO_CARRY; }  }
            only_alnum = 0;  }
        else if (c == '9') {
            DVAL(dest, pos) = '0';
            carry = '1'; }
        else if (c == 'z') {
            DVAL(dest, pos) = 'a';
            carry = 'a'; }
        else if (c == 'Z') {
            DVAL(dest, pos) = 'A';
            carry = 'A'; }
        else {
            DVAL(dest, pos) = ++c;
            carry = NO_CARRY;
            break; }  }

    return carry;
}
/*--------------------------------------------------------------------------*/

static int dstr_increment_printable(DSTR dest)
{
    int carry = NO_CARRY;

    for (long pos = DLEN(dest) - 1; pos >= 0; --pos) {
        char c = DVAL(dest, pos);
        if (isprint(c)) {
            switch (c) {
            case '~':
                DVAL(dest, pos) = '!';
                carry = '!';
                break;
            default:
                DVAL(dest, pos) =  ++c;
                carry = NO_CARRY;
            }
        }
        if (carry == NO_CARRY)
            break;
    }
    return carry;
}
/*--------------------------------------------------------------------------*/

int dstr_increment(DSTR dest)
{
    dstr_assert_valid(dest);
    if (DLEN(dest) == 0) return DSTR_SUCCESS;

    size_t alnum_count = 0;
    size_t print_count = 0;

    for (size_t n = 0; n < DLEN(dest); ++n) {
        char ch = DVAL(dest, n);
        if (isalnum(ch)) {
            ++alnum_count; }
        else if (isprint(ch)) {
            ++print_count; } }

    int carry = NO_CARRY;
    if (alnum_count) {
        // If we have alnum chars, only they are incremented
        carry = dstr_increment_alnum(dest); }
    else if (print_count) {
        // If we have printable chars without alnum only they are incremented
        carry = dstr_increment_printable(dest); }
    else {
        // otherwise we don't know how to inc
        return DSTR_FAIL; }

    if (carry) {
        if (!dstr_insert_cc_imp(dest, 0, (char) carry, 1))
            return DSTR_FAIL; }

    dstr_assert_valid(dest);
    return DSTR_SUCCESS;
}
/*--------------------------------------------------------------------------*/

void dstr_remove_char(DSTR p, char to_delete)
{
    dstr_assert_valid(p);

    size_t write_index = 0;
    size_t read_index = 0;

    for ( ; read_index < DLEN(p); ++read_index) {
        char ch = DVAL(p, read_index);
        if (ch != to_delete) {
            DVAL(p, write_index) = ch;
            ++write_index; } }

    DVAL(p, write_index) = '\0';
    DLEN(p) = write_index;
    dstr_assert_valid(p);
}
/*--------------------------------------------------------------------------*/

void dstr_remove_any(DSTR p, const char* selectors)
{
    if (!selectors || !*selectors) return;

    /*
     *  Optimization: if selectors is a string of one character then call
     *  dstr_remove_char() with that character.
     */
    if (selectors[1] == '\0') {
        dstr_remove_char(p, selectors[0]);
        return; }

    /*  We already have delete by selectors in the more general translate
     *  functions
     */
    dstr_translate_delete_aux(p, selectors);
}
/*--------------------------------------------------------------------------*/

void dstr_remove_prefix(DSTR p, const char* s)
{
    if (!s || !*s) return;
    ptrdiff_t pos = dstr_prefix_sz_imp(p, s, 0);
    if (pos > 0)
        dstr_remove_imp(p, 0, pos);
}
/*--------------------------------------------------------------------------*/

void dstr_iremove_prefix(DSTR p, const char* s)
{
    if (!s || !*s) return;
    ptrdiff_t pos = dstr_prefix_sz_imp(p, s, 1);
    if (pos > 0)
        dstr_remove_imp(p, 0, pos);
}
/*--------------------------------------------------------------------------*/

void dstr_remove_suffix(DSTR p, const char* s)
{
    if (!s || !*s) return;
    ptrdiff_t pos = dstr_suffix_sz_imp(p, s, 0);
    if (pos >= 0) {
        DLEN(p) = pos;
        DVAL(p, pos) = '\0'; }
}
/*--------------------------------------------------------------------------*/

void dstr_iremove_suffix(DSTR p, const char* s)
{
    if (!s || !*s) return;
    ptrdiff_t pos = dstr_suffix_sz_imp(p, s, 1);
    if (pos >= 0) {
        DLEN(p) = pos;
        DVAL(p, pos) = '\0'; }
}
/*--------------------------------------------------------------------------*/
