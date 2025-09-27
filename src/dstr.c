#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <stddef.h>

#include <dstr/dstr.h>

/*
 *  For convenience, make code shorter
 */
#define BASE(p)       (p)
#define DBUF(p)       (BASE(p)->data)
#define DLEN(p)       (BASE(p)->length)
#define DCAP(p)       (BASE(p)->capacity)
#define DVAL(p, i)    DBUF(p)[(i)]
#define D_SSO_BUF(p)  (&(p)->sso_buffer[0])
#define D_IS_SSO(p)   (DBUF(p) == D_SSO_BUF(p))
/*--------------------------------------------------------------------------*/

#define dstr_assert_valid(p) do {                               \
    assert((p) != NULL);                                        \
    assert(DBUF(p) != NULL);                                    \
    assert(DCAP(p) > 0);                                        \
    assert(DLEN(p) < DCAP(p));                                  \
    assert(DVAL(p, DLEN(p)) == '\0');                           \
    assert(DLEN(p) == strlen(DBUF(p)));                         \
    assert((DCAP(p) % DSTR_INITIAL_CAPACITY) == 0);             \
} while(0)
/*--------------------------------------------------------------------------*/

#if !defined(__cplusplus)
#if __STDC_VERSION__ < 199901L
#error dstr requires at least a C99 compiler
#endif
#endif

/*
 *   Get the number of characters vsnprintf would write
 *   vsnprintf is from C99 onwards
 */
#define get_vsprintf_len(f, a) vsnprintf(NULL, 0, f, a)

/*
 *  WIN32 missing functions or functions with different names
 */
#ifdef _WIN32
static const char* my_strcasestr(const char* haystack, const char* needle)
{
    const char* cp = haystack;

    if (*needle == '\0')
        return haystack;

    while (*cp != '\0') {
        const char* s1 = cp;
        const char* s2 = needle;

        while ( *s1 && *s2 && (*s1 == *s2 || toupper(*s1) == toupper(*s2)) ) {
            ++s1;
            ++s2;
        }

        if (*s2 == '\0')
            return cp;

        ++cp;
    }

    return NULL;
}
#define strcasecmp _stricmp
#define strcasestr my_strcasestr
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

static inline char* dstr_address(DSTR p, size_t pos) {
    return (DBUF(p) + pos);
}
static inline const char* dstr_address_c(CDSTR p, size_t pos) {
    return (DBUF(p) + pos);
}
/*-------------------------------------------------------------------------------*/

static inline char* dstr_tail(DSTR p)
{
    return dstr_address(p, DLEN(p));
}
/*-------------------------------------------------------------------------------*/

static const char* my_strcasechr(const char* s, int c)
{
    if (!s)
        return NULL;

    unsigned char uc = (unsigned char) toupper(c);
    unsigned char lc = (unsigned char) tolower(c);

    while (*s != '\0') {
        unsigned char curr = *s;
        if (curr == lc || curr == uc)
            return s;
        ++s;
    }

    return NULL;
}
/*-------------------------------------------------------------------------------*/

static DSTR dstr_alloc_empty(void)
{
    DSTR p = (DSTR) malloc(sizeof(struct DSTR_IMP));
    if (p) {
        // inline at dstr.h
        dstr_init_data(p);
    }
    dstr_assert_valid(p);
    return p;
}
/*-------------------------------------------------------------------------------*/

static DSTR dstr_grow(DSTR p, size_t len)
{
    size_t new_capacity;
    char* newbuff;

    assert(p != NULL);

    if (p->capacity > len) {
        return p;
    }

    new_capacity = p->capacity;
    while (new_capacity <= len)
        new_capacity *= 2;

    if (new_capacity > UINT32_MAX) {
        errno = ERANGE;
        return NULL;
    }

    if (p->capacity == DSTR_INITIAL_CAPACITY) {
        assert(D_IS_SSO(p));
        if ((newbuff = (char*) malloc(new_capacity)) == NULL) {
            return NULL;
        }

        memcpy(newbuff, p->data, p->length + 1);
        p->data[0] = '\0';
    }
    else {
        assert(!D_IS_SSO(p));
        if ((newbuff = (char*) realloc(p->data, new_capacity)) == NULL) {
            return NULL;
        }
    }

    p->capacity = new_capacity;
    p->data = newbuff;

    dstr_assert_valid(p);
    assert(len < DCAP(p));
    return p;
}
/*-------------------------------------------------------------------------------*/

static inline DSTR dstr_grow_by(DSTR p, size_t n)
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

static int dstr_insert_imp(DSTR p, size_t index, const char* buff, size_t len)
{
    size_t bytes_to_move;
    char* first = DBUF(p);
    char* last = first + DLEN(p);
    ptrdiff_t overlap = -1;

    // different handling if source data within the DSTR allocated
    // buffer
    //
    if (first <= buff && buff <= last) {
        overlap = buff - first;
    }

    if (len == 0)
        return DSTR_SUCCESS;

    if (!dstr_grow_by(p, len))
        return DSTR_FAIL;

    // in case of overlap, check if realloc() moved the buffer
    //
    if (overlap >= 0 && (DBUF(p) != first)) {
        buff = DBUF(p) + overlap;
    }

    index = min_2(index, DLEN(p));
    bytes_to_move = DLEN(p) - index;

    if (bytes_to_move > 0) {
        assert(index + len + bytes_to_move < DCAP(p));
        memmove( dstr_address(p, index + len),
                 dstr_address(p, index),
                 bytes_to_move );
    }

    memcpy(dstr_address(p, index), buff, len);

    DLEN(p) += len;
    DVAL(p, DLEN(p)) = '\0';

    dstr_assert_valid(p);
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

static int dstr_insert_cc_imp(DSTR p, size_t index, char c, size_t count)
{
    size_t bytes_to_move;

    if (count == 0)
        return DSTR_SUCCESS;

    index = min_2(index, DLEN(p));
    bytes_to_move = DLEN(p) - index;

    if (c == '\0') {
        dstr_truncate_imp(p, index);
        return DSTR_SUCCESS;
    }

    if (!dstr_grow_by(p, count))
        return DSTR_FAIL;

    if (bytes_to_move > 0) {
        memmove(dstr_address(p, index + count),
                  dstr_address(p, index),
                  bytes_to_move);
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
    size_t bytes_to_move;

    if (count == 0)
        return;

    if (pos >= DLEN(p))
        return;

    count = min_2(count, DLEN(p) - pos);
    bytes_to_move = DLEN(p) - pos - count;

    if (bytes_to_move > 0)
        memmove(dstr_address(p, pos), dstr_address(p, pos + count), bytes_to_move);

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

    // Check for overlap, must copy src if so
    //
    char* first = DBUF(p);
    char* last = first + DLEN(p);
    int result = DSTR_FAIL;

    // in case of overlap we copy to a tmp DSTR
    //
    if (first <= buff && buff <= last) {
        DSTR_IMP tmp;
        dstr_init_data(&tmp);

        if (dstr_assign_bl(&tmp, buff, buflen))
        {
            dstr_remove_imp(p, pos, count);
            buff = DBUF(&tmp);
            result = dstr_insert_imp(p, pos, buff, buflen);
            dstr_clean_data(&tmp);
        }
    }
    else
    {
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

static inline int dstr_append_imp(DSTR p, const char* value, size_t len)
{
    return dstr_insert_imp(p, DLEN(p), value, len);
}
/*-------------------------------------------------------------------------------*/

static inline int dstr_assign_imp(DSTR p, const char* value, size_t len)
{
    dstr_truncate(p);
    return dstr_append_imp(p, value, len);
}
/*-------------------------------------------------------------------------------*/

static inline DSTR dstr_create_len_imp(size_t len)
{
    DSTR p = dstr_alloc_empty();
    if (!p)
        return NULL;

    return dstr_grow(p, len);
}
/*-------------------------------------------------------------------------------*/

static DSTR dstr_create_buff_imp(const char* buff, size_t len)
{
    DSTR p;

    if ((p = dstr_create_len_imp(len)) == NULL)
        return NULL;

    if (len > 0) {
        assert(buff != NULL);
        memcpy(DBUF(p), buff, len);
    }

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

    dstr_assert_valid(p);

    if (pos >= DLEN(p))
        return DSTR_NPOS;

    search_loc = DBUF(p) + pos;

    if (ignore_case)
        found_loc = strcasestr(search_loc, s);
    else
        found_loc = strstr(search_loc, s);

    if (found_loc == NULL)
        return DSTR_NPOS;

    return (size_t)(found_loc - DBUF(p));
}
/*-------------------------------------------------------------------------------*/

static DSTR_BOOL dstr_suffix_sz_imp(CDSTR p,
                                    const char* s,
                                    int ignore_case)
{
    DSTR_BOOL result;
    size_t compare_len;
    const char* compare_addr;

    dstr_assert_valid(p);
    assert(s != NULL);

    if (DSTR_IS_NULL(p) || s == NULL)
        return DSTR_FALSE;

    if ((compare_len = strlen(s)) > DLEN(p))
        return DSTR_FALSE;

    compare_addr = DBUF(p) + (DLEN(p) - compare_len);

    if (ignore_case)
        result = (strcasecmp(compare_addr, s) == 0);
    else
        result = (strcmp(compare_addr, s) == 0);

    return result;
}
/*-------------------------------------------------------------------------------*/

static DSTR_BOOL dstr_prefix_sz_imp(CDSTR p,
                                    const char* s,
                                    int ignore_case)
{
    const char* pbuf;

    dstr_assert_valid(p);
    assert(s != NULL);

    pbuf = DBUF(p);
    if (ignore_case) {
        while (*s && (toupper(*pbuf) == toupper(*s))) {
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
    return (*s == '\0');
}
/*-------------------------------------------------------------------------------*/

static size_t dstr_find_c_imp(CDSTR p,
                              size_t pos,
                              char c,
                              int ignore_case)
{
    const char* search_loc;
    const char* found_loc;

    dstr_assert_valid(p);

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

static size_t dstr_ffo_imp(CDSTR p,
                           size_t pos,
                           const char* pattern,
                           int not_off)
{
    size_t result = pos;

    dstr_assert_valid(p);
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

static DSTR_BOOL dstr_isdigits_imp(CDSTR src, DSTR_BOOL is_hex)
{
    size_t n;

    dstr_assert_valid(src);

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
DSTR dstr_create(void)
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
    if (buff == NULL)
        len = 0;
    else
        len = strnlen(buff, len);

    return dstr_create_buff_imp(buff, len);
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_sz(const char* sz)
{
    if (sz == NULL)
        return dstr_alloc_empty();

    return dstr_create_buff_imp(sz, strlen(sz));
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_ds(CDSTR rhs)
{
    dstr_assert_valid(rhs);
    return dstr_create_buff_imp(DBUF(rhs), DLEN(rhs));
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_substr(CDSTR p, size_t pos, size_t count)
{
    dstr_assert_valid(p);

    if (DLEN(p) > pos) {
        count = min_2(count, DLEN(p) - pos);
        if (count > 0)
            return dstr_create_buff_imp(DBUF(p)+pos, count);
    }
    return NULL;
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_cc(char ch, size_t count)
{
    DSTR p;

    if (count == 0)
        return dstr_create();

    if ((p = dstr_create_len_imp(count)) == NULL)
        return NULL;

    if (ch == '\0') {
        DVAL(p, 0) = '\0';
        DLEN(p) = 0;
    }
    else {
        memset(DBUF(p), ch, count);
        DVAL(p, count) = '\0';
        DLEN(p) = count;
    }

    dstr_assert_valid(p);
    return p;
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_assign_fromfile(DSTR p, const char* fname)
{
    FILE* fp;
    long fsize, sread;
    int original_errno = 0;
    bool create_new = (p == NULL);

    if ((fp = fopen(fname, "r")) == NULL)
        return NULL;

    if (fseek(fp, 0L, SEEK_END) != 0)
        goto err_close_fp;

    if ((fsize = ftell(fp)) == -1)
        goto err_close_fp;

    rewind(fp);

    if (create_new) {
        if ((p = dstr_create_len_imp(fsize)) == NULL)
            goto err_close_fp;
    }
    else {
        if ((p = dstr_grow(p, fsize)) == NULL)
            goto err_close_fp;
    }

    if ((sread = fread(DBUF(p), sizeof(char), fsize, fp)) != fsize)
        goto err_clean_result;

    fclose(fp);

    DVAL(p, sread) = '\0';
    DLEN(p) = sread;

    dstr_assert_valid(p);
    return p;

err_clean_result:
    original_errno = errno;
    if (create_new) {
        dstr_destroy(p);
    }

err_close_fp:
    if (!original_errno)
        original_errno = errno;
    fclose(fp);
    errno = original_errno;
    return NULL;
}
/*-------------------------------------------------------------------------------*/

DSTR dstr_create_fromfile(const char* fname)
{
    return dstr_assign_fromfile(NULL, fname);
}
/*-------------------------------------------------------------------------------*/

void dstr_clean_data(DSTR p)
{
    if (!D_IS_SSO(p)) {
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
    return dstr_grow(p, len) ? DSTR_SUCCESS : DSTR_FAIL;
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
        if (dstr_grow(p, len) == NULL)
            result = DSTR_FAIL;
    }

    return result;
}
/*-------------------------------------------------------------------------------*/

int dstr_shrink_to_fit(DSTR p)
{
    dstr_assert_valid(p);

    // Nothing to shrink
    //
    if (DCAP(p)/2 < DLEN(p))
        return DSTR_SUCCESS;

    DSTR_IMP tmp;
    dstr_init_data(&tmp);
    if (!dstr_assign_ds(&tmp, p))
        return DSTR_FAIL;

    dstr_swap(p, &tmp);
    dstr_clean_data(&tmp);

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
    dstr_assert_valid(src);

    return dstr_assign_imp(dest, DBUF(src), DLEN(src));
}
/*-------------------------------------------------------------------------------*/

int dstr_assign_sz(DSTR p, const char* value)
{
    dstr_assert_valid(p);

    if (value == NULL) {
        dstr_truncate(p);
        return DSTR_SUCCESS;
    }

    return dstr_assign_imp(p, value, strlen(value));
}
/*-------------------------------------------------------------------------------*/

int dstr_assign_bl(DSTR p, const char* buff, size_t len)
{
    dstr_assert_valid(p);

    if (buff == NULL || len == 0) {
        dstr_truncate(p);
        return DSTR_SUCCESS;
    }

    len = strnlen(buff, len);
    return dstr_assign_imp(p, buff, len);
}
/*-------------------------------------------------------------------------------*/

int dstr_assign_substr(DSTR dest, CDSTR p, size_t pos, size_t count)
{
    dstr_assert_valid(p);

    if (pos >= DLEN(p))
        return DSTR_SUCCESS;

    if (count == 0)
        return DSTR_SUCCESS;

    return dstr_assign_imp( dest,
                            DBUF(p) + pos,
                            min_2(count, DLEN(p)-pos) );
}
/*-------------------------------------------------------------------------------*/

int dstr_assign_cc(DSTR p, char c, size_t count)
{
    dstr_assert_valid(p);
    dstr_truncate(p);
    return dstr_insert_cc_imp(p, DLEN(p), c, count);
}
/*-------------------------------------------------------------------------------*/

int dstr_append_cc(DSTR p, char c, size_t count)
{
    dstr_assert_valid(p);
    return dstr_insert_cc_imp(p, DLEN(p), c, count);
}
/*-------------------------------------------------------------------------------*/

int dstr_append_c(DSTR p, char c)
{
    dstr_assert_valid(p);

    if (c == '\0')
        return DSTR_SUCCESS;

    if ((DLEN(p) + 1) >= DCAP(p)) {
        if (!dstr_grow_by(p, 1)) {
            return DSTR_FAIL;
        }
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
    dstr_assert_valid(src);

    return dstr_append_imp(dest, DBUF(src), DLEN(src));
}
/*-------------------------------------------------------------------------------*/

int dstr_append_sz(DSTR p, const char* value)
{
    dstr_assert_valid(p);

    if (value == NULL)
        return DSTR_SUCCESS;

    return dstr_append_imp(p, value, strlen(value));
}
/*-------------------------------------------------------------------------------*/

int dstr_append_bl(DSTR p, const char* buff, size_t len)
{
    dstr_assert_valid(p);

    if (buff == NULL)
        return DSTR_SUCCESS;

    if ((len = strnlen(buff, len)) == 0)
        return DSTR_SUCCESS;

    return dstr_append_imp(p, buff, len);
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
    return dstr_suffix_sz_imp(p, s, DSTR_FALSE);
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_isuffix_sz(CDSTR p, const char* s)
{
    /* ignore case */
    return dstr_suffix_sz_imp(p, s, DSTR_TRUE);
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_prefix_sz(CDSTR p, const char* s)
{
    return dstr_prefix_sz_imp(p, s, DSTR_FALSE);
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_iprefix_sz(CDSTR p, const char* s)
{
    return dstr_prefix_sz_imp(p, s, DSTR_TRUE);
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

DSTR_BOOL dstr_isblank(CDSTR p)
{
    return dstr_ffno_sz(p, 0, " \t") == DSTR_NPOS;
}
/*-------------------------------------------------------------------------------*/

size_t dstr_substr(CDSTR p,
                   size_t pos,
                   size_t count,
                   char dest[],
                   size_t destsize)
{
    dstr_assert_valid(p);

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

    va_copy(argptr2, argptr);
    len = get_vsprintf_len(fmt, argptr2);
    va_end(argptr2);

    if (len < 0)
        return DSTR_FAIL;

    if (!dstr_grow_by(p, len))
        return DSTR_FAIL;

    if ((len = vsprintf(dstr_tail(p), fmt, argptr)) < 0)
        return DSTR_FAIL;

    DLEN(p) += len;

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
    dstr_truncate(p);
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

    dstr_append_vsprintf(result, fmt, argptr);
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
    dstr_assert_valid(src);
    return dstr_insert_imp(dest, index, DBUF(src), DLEN(src));
}
/*-------------------------------------------------------------------------------*/

int dstr_insert_bl(DSTR p, size_t index, const char* buff, size_t len)
{
    dstr_assert_valid(p);

    /* do nothing*/
    if (buff == NULL || len == 0)
        return DSTR_SUCCESS;

    len = strnlen(buff, len);

    return dstr_insert_imp(p, index, buff, len);
}
/*-------------------------------------------------------------------------------*/

int dstr_compare_sz(CDSTR lhs, const char* sz)
{
    int result;

    dstr_assert_valid(lhs);

    if (sz == NULL) {
        if (DLEN(lhs) > 0)
            result = 1;
        else
            result = 0;
    }
    else {
        result = strcmp(DBUF(lhs), sz);
    }

    return result;
}
/*-------------------------------------------------------------------------------*/

int dstr_compare_ds(CDSTR lhs, CDSTR rhs)
{
    dstr_assert_valid(lhs);
    dstr_assert_valid(rhs);

    return strcmp(DBUF(lhs), DBUF(rhs));
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_equal_sz(CDSTR lhs, const char* sz)
{
    dstr_assert_valid(lhs);

    if (sz == NULL)
        return (DLEN(lhs) == 0);

    return (strcmp(DBUF(lhs), sz) == 0);
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_iequal_sz(CDSTR lhs, const char* sz)
{
    dstr_assert_valid(lhs);

    if (sz == NULL)
        return (DLEN(lhs) == 0);

    return (strcasecmp(DBUF(lhs), sz) == 0);
}
/*-------------------------------------------------------------------------------*/

DSTR_BOOL dstr_equal_ds(CDSTR lhs, CDSTR rhs)
{
    dstr_assert_valid(lhs);
    dstr_assert_valid(rhs);

    if (DLEN(lhs) != DLEN(rhs))
        return 0;

    return (strncmp(DBUF(lhs), DBUF(rhs), DLEN(lhs)) == 0);
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
    dstr_assert_valid(src);
    return dstr_replace_imp(dest, pos, len, DBUF(src), DLEN(src));
}
/*-------------------------------------------------------------------------------*/

int dstr_replace_bl(DSTR p, size_t pos, size_t count, const char* buff, size_t buflen)
{
    dstr_assert_valid(p);
    return dstr_replace_imp(p, pos, count, buff, buflen);
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
            dstr_truncate(p);
        else
            dstr_remove_imp(p, 0, pos);
    }

    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

void dstr_trim_both(DSTR p)
{
    dstr_trim_left(p);
    dstr_trim_right(p);
}
/*-------------------------------------------------------------------------------*/

void dstr_ascii_upper(DSTR p)
{
    char* s;

    dstr_assert_valid(p);

    for (s = DBUF(p); *s; ++s)
        if (islower(*s))
            *s = (char)toupper(*s);

    dstr_assert_valid(p);
}
/*-------------------------------------------------------------------------------*/

void dstr_ascii_lower(DSTR p)
{
    char* s;

    dstr_assert_valid(p);

    for (s = DBUF(p); *s; ++s)
        if (isupper(*s))
            *s = (char)tolower(*s);

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

void dstr_swap(DSTR d1, DSTR d2)
{
    dstr_assert_valid(d1);
    dstr_assert_valid(d2);

    struct DSTR_IMP tmp = *d1;
    *d1 = *d2;
    *d2 = tmp;

    if (d1->capacity == DSTR_INITIAL_CAPACITY) {
        d1->data = &d1->sso_buffer[0];
    }

    if (d2->capacity == DSTR_INITIAL_CAPACITY) {
        d2->data = &d2->sso_buffer[0];
    }

    dstr_assert_valid(d1);
    dstr_assert_valid(d2);
}
/*-------------------------------------------------------------------------------*/

int dstr_fgets(DSTR p, FILE* fp)
{
    int c;

    dstr_assert_valid(p);
    assert(fp != NULL);

    dstr_truncate(p);

    /* skip blanks */
    do {
        if ((c = fgetc(fp)) == EOF)
            return EOF;

    } while (isspace(c));

    /* read until next blank characters */
    do {
        if (!dstr_append_c(p, (char)c))
            return EOF;

        if ((c = fgetc(fp)) == EOF)
            return DSTR_SUCCESS;

    } while (!isspace(c));

    ungetc(c, fp);
    return DSTR_SUCCESS;
}
/*-------------------------------------------------------------------------------*/

int dstr_fgetline(DSTR p, FILE* fp)
{
    int c;

    dstr_assert_valid(p);
    dstr_truncate(p);

    /* make room for at least 120 chars */
    if (DCAP(p) < 120)
        dstr_grow(p, 120);

    while ((c = fgetc(fp)) != EOF && c != '\n') {
        if (!dstr_append_c(p, c)) {
            dstr_truncate(p);
            return EOF;
        }
    }

    if (DLEN(p) > 0 || (c == '\n'))
        return DSTR_SUCCESS;

    return EOF;
}
/*-------------------------------------------------------------------------------*/

unsigned int dstr_hash(CDSTR src)
{
    const char* p;
    size_t len;
    unsigned int result = 0;

    dstr_assert_valid(src);

    len = DLEN(src);
    p    = DBUF(src);

    while (len) {
        result = (result << 5) - result + (unsigned int)(*p);
        ++p;
        --len;
    }

    return result;
}
/*-------------------------------------------------------------------------------*/

int dstr_itoa(DSTR dest, long n)
{
    /*
     *   Not the fastest but definitely the simplest
     */
    return dstr_assign_sprintf(dest, "%ld", n);
}
/*-------------------------------------------------------------------------------*/

long dstr_atoi(CDSTR src)
{
    const char* p;
    int base = 10;

    dstr_assert_valid(src);

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
