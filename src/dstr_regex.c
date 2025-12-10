/*
 * Part of this file is based on POCO C++ Libraries
 * Original work Copyright (c) 2006-2023, Applied Informatics Software
 * Engineering GmbH. Licensed under the Boost Software License, Version 1.0.
 *
 * Modifications and additional code:
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license
 * text.
 */
#include <assert.h>
#include <stdint.h>

// Regular expression cache uses thread local storage using ISO-C11 tss* functions
// or pthread equivalent if not available
//
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
   #if defined(__GNUC__) && (defined(__MINGW32__) || defined(__MINGW64__))
      #include <pthread.h>
      #define RE_CACHE_USE_PTHREAD 1
   #else
      #include <threads.h>
   #endif
#elif defined(__cplusplus) && (__cplusplus >= 201703L) && __has_include(<threads.h>)
   #include <threads.h>
   #if !defined(_Thread_local)
      #define _Thread_local thread_local
   #endif
#else
   #error "Needs C11 threads.h or C++17 with threads.h support"
#endif

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

// Our headers
//
#include <dstr/dstr.h>
#include "dstr_internal.h"
/*---------------------------------------------------------------------*/

// Constants for options (based on POCO)
//
enum {
    // case insensitive matching (/i) [ctor]
    //
    REGEX_CASELESS = 0x00000001,

    // enable multi-line mode; affects ^ and $ (/m) [ctor]
    //
    REGEX_MULTILINE = 0x00000002,

    // dot matches all characters, including newline (/s) [ctor]
    //
    REGEX_DOTALL = 0x00000004,

    // totally ignore whitespace (/x) [ctor]
    //
    REGEX_EXTENDED = 0x00000008,

    // treat pattern as if it starts with a ^ [ctor, match]
    //
    REGEX_ANCHORED = 0x00000010,

    // dollar matches end-of-string only, not last newline in string [ctor]
    //
    REGEX_DOLLAR_ENDONLY = 0x00000020,

    // enable optional PCRE functionality [ctor]
    //
    REGEX_EXTRA = 0x00000040,

    // circumflex does not match beginning of string [match]
    //
    REGEX_NOTBOL = 0x00000080,

    // $ does not match end of string [match]
    //
    REGEX_NOTEOL = 0x00000100,

    // make quantifiers ungreedy [ctor]
    //
    REGEX_UNGREEDY = 0x00000200,

    // empty string never matches [match]
    //
    REGEX_NOTEMPTY = 0x00000400,

    // assume pattern and subject is UTF-8 encoded [ctor]
    //
    REGEX_UTF8 = 0x00000800,

    // disable numbered capturing parentheses [ctor, match]
    //
    REGEX_NO_AUTO_CAPTURE = 0x00001000,

    // do not check validity of UTF-8 code sequences [match]
    //
    REGEX_NO_UTF8_CHECK = 0x00002000,

    // an  unanchored  pattern  is  required  to  match
    //
    REGEX_FIRSTLINE = 0x00040000,

    // names used to identify capturing subpatterns need not be unique [ctor]
    //
    REGEX_DUPNAMES = 0x00080000,

    // assume newline is CR ('\r'), the default [ctor]
    //
    REGEX_NEWLINE_CR = 0x00100000,

    // assume newline is LF ('\n') [ctor]
    //
    REGEX_NEWLINE_LF = 0x00200000,

    // assume newline is CRLF ("\r\n") [ctor]
    //
    REGEX_NEWLINE_CRLF = 0x00300000,

    // assume newline is any valid Unicode newline character [ctor]
    //
    REGEX_NEWLINE_ANY = 0x00400000,

    // assume newline is any of CR, LF, CRLF [ctor]
    //
    REGEX_NEWLINE_ANYCRLF = 0x00500000,

    // replace all occurences (/g) [subst]
    //
    REGEX_GLOBAL = 0x10000000,

    // treat dollar in replacement string as ordinary character [subst]
    //
    REGEX_NO_VARS = 0x20000000
};
/*-------------------------------------------------------------------------------*/

static int compile_options(int options)
{
    int pcre_opts = 0;

    if (options & REGEX_CASELESS)
        pcre_opts |= PCRE2_CASELESS;
    if (options & REGEX_MULTILINE)
        pcre_opts |= PCRE2_MULTILINE;
    if (options & REGEX_DOTALL)
        pcre_opts |= PCRE2_DOTALL;
    if (options & REGEX_EXTENDED)
        pcre_opts |= PCRE2_EXTENDED;
    if (options & REGEX_ANCHORED)
        pcre_opts |= PCRE2_ANCHORED;
    if (options & REGEX_DOLLAR_ENDONLY)
        pcre_opts |= PCRE2_DOLLAR_ENDONLY;
    if (options & REGEX_UNGREEDY)
        pcre_opts |= PCRE2_UNGREEDY;
    if (options & REGEX_UTF8)
        pcre_opts |= PCRE2_UTF | PCRE2_UCP;
    if (options & REGEX_NO_AUTO_CAPTURE)
        pcre_opts |= PCRE2_NO_AUTO_CAPTURE;
    if (options & REGEX_FIRSTLINE)
        pcre_opts |= PCRE2_FIRSTLINE;
    if (options & REGEX_DUPNAMES)
        pcre_opts |= PCRE2_DUPNAMES;

    return pcre_opts;
}
/*-------------------------------------------------------------------------------*/

static int match_options(int options)
{
    int pcre_opts = 0;

    if (options & REGEX_ANCHORED)
        pcre_opts |= PCRE2_ANCHORED;
    if (options & REGEX_NOTBOL)
        pcre_opts |= PCRE2_NOTBOL;
    if (options & REGEX_NOTEOL)
        pcre_opts |= PCRE2_NOTEOL;
    if (options & REGEX_NOTEMPTY)
        pcre_opts |= PCRE2_NOTEMPTY;
    if (options & REGEX_NO_AUTO_CAPTURE)
        pcre_opts |= PCRE2_NO_AUTO_CAPTURE;
    if (options & REGEX_NO_UTF8_CHECK)
        pcre_opts |= PCRE2_NO_UTF_CHECK;

    return pcre_opts;
}
/*-------------------------------------------------------------------------------*/

static int parse_regex_options(const char* str)
{
    if (!str || !*str)
        return 0;

    int opts = 0;
    for (; *str; ++str) {
        switch (*str) {
        case ' ' :
        case '/' : break;
        case 'g' : opts |= REGEX_GLOBAL;          break;
        case 'i' : opts |= REGEX_CASELESS;        break;
        case 'm' : opts |= REGEX_MULTILINE;       break;
        case 's' : opts |= REGEX_DOTALL;          break;
        case 'x' : opts |= REGEX_EXTENDED;        break;
        case 'X' : opts |= REGEX_EXTRA;           break;
        case 'U' : opts |= REGEX_UNGREEDY;        break;
        case 'D' : opts |= REGEX_DOLLAR_ENDONLY;  break;
        case 'd' : opts |= REGEX_NOTEOL;          break;
        case 'E' : opts |= REGEX_NOTEMPTY;        break;
        case 'n' : opts |= REGEX_NO_AUTO_CAPTURE; break;
        case 'F' : opts |= REGEX_FIRSTLINE;       break;
        case 'A' : opts |= REGEX_ANCHORED;        break;
        case 't' : opts |= REGEX_DUPNAMES;        break;
        case '$' : opts |= REGEX_NO_VARS;         break;
        case '\n': opts |= REGEX_NEWLINE_LF;      break;
        case '\r': opts |= REGEX_NEWLINE_CR;      break;
        default: break;
        }
    }
    return opts;
}
/*-------------------------------------------------------------------------------*/

typedef struct GroupInfo {
    long        group_number;
    const char* group_name;
} GroupInfo;
/*-------------------------------------------------------------------------------*/

typedef struct Compiled_Regex {
    // compiled pcre regex
    pcre2_code* _pRE;

    // array of name groups
    GroupInfo*  p_groups;
    size_t      n_groups;

    // for caching and LRU
    size_t      tick_count;
    DSTR        pattern;
    int         options;
} Compiled_Regex;
/*-------------------------------------------------------------------------------*/

static const char* find_group_name(Compiled_Regex* cr, int n)
{
    for (size_t i = 0; i < cr->n_groups; ++i) {
        GroupInfo* pGI = &(cr->p_groups[i]);
        if (pGI != NULL && pGI->group_number == n)
            return pGI->group_name;
    }
    return NULL;
}
/*-------------------------------------------------------------------------------*/

static void on_malloc_error(void)
{
    fprintf(stderr, "DSTR library: malloc/realloc failed. Out of memory!\n");
    abort();
}
/*-------------------------------------------------------------------------------*/

static void* _re_malloc(size_t len)
{
    void* result = malloc(len);
    if (!result)
        on_malloc_error();
    return result;
}
#define RE_MALLOC(TYPE, Nelem) ((TYPE*) _re_malloc((Nelem) * sizeof(TYPE)))
/*-------------------------------------------------------------------------------*/

static
Compiled_Regex* dstr_compile_regex(const char* pattern, int options, int* err)
{
    pcre2_compile_context* context = pcre2_compile_context_create(NULL);
    if (!context) {
        on_malloc_error();
        return NULL;
    }

    if (options & REGEX_NEWLINE_LF)
        pcre2_set_newline(context, PCRE2_NEWLINE_LF);
    else if (options & REGEX_NEWLINE_CRLF)
        pcre2_set_newline(context, PCRE2_NEWLINE_CRLF);
    else if (options & REGEX_NEWLINE_ANY)
        pcre2_set_newline(context, PCRE2_NEWLINE_ANY);
    else if (options & REGEX_NEWLINE_ANYCRLF)
        pcre2_set_newline(context, PCRE2_NEWLINE_ANYCRLF);
    else // default REGEX_NEWLINE_CR
        pcre2_set_newline(context, PCRE2_NEWLINE_CR);

    int error_code;
    PCRE2_SIZE error_offset;
    pcre2_code* _pRE = pcre2_compile((PCRE2_SPTR)pattern,
                                     strlen(pattern),
                                     compile_options(options),
                                     &error_code,
                                     &error_offset,
                                     context);

    pcre2_compile_context_free(context);

    if (!_pRE) {
        if (err) *err = (REGEX_COMPILE_ERROR_BASE + error_code);
        dstr_regex_perror(error_code);
        return NULL;
    }

    unsigned int name_count = 0;
    pcre2_pattern_info(_pRE, PCRE2_INFO_NAMECOUNT, &name_count);

    unsigned int name_entry_size = 0;
    pcre2_pattern_info(_pRE, PCRE2_INFO_NAMEENTRYSIZE, &name_entry_size);

    unsigned char* name_table = NULL;
    pcre2_pattern_info(_pRE, PCRE2_INFO_NAMETABLE, &name_table);

    GroupInfo* gInfo = NULL;
    if (name_count) {
        gInfo = RE_MALLOC(GroupInfo, name_count);
        if (!gInfo) {
            pcre2_code_free(_pRE);
            return NULL;
        }

        for (uint32_t i = 0; i < name_count; i++) {
            unsigned char* group = name_table + 2 + (name_entry_size * i);
            int n = pcre2_substring_number_from_name(_pRE, group);
            gInfo[i].group_number = n;
            gInfo[i].group_name = (const char*)group;
        }
    }

    Compiled_Regex* result = RE_MALLOC(Compiled_Regex, 1);
    if (!result) {
        if (gInfo) free(gInfo);
        pcre2_code_free(_pRE);
        return NULL;
    }

    result->_pRE = _pRE;
    result->p_groups = gInfo;
    result->n_groups = name_count;
    result->pattern = dstr_create_sz(pattern);
    result->options = options;
    result->tick_count = 0;
    return result;
}
/*-------------------------------------------------------------------------------*/

static void destroy_compiled_regex(Compiled_Regex* cr)
{
    if (!cr)
        return;

    if (cr->pattern)
        dstr_destroy(cr->pattern);

    if (cr->p_groups)
        free(cr->p_groups);

    if (cr->_pRE)
        pcre2_code_free(cr->_pRE);

    free(cr);
}
/*-------------------------------------------------------------------------------*/

static int dstr_regex_match_aux(Compiled_Regex* cr,
                                CDSTR subject, size_t offset,
                                DSTR_Regex_Match* mtch, int options)
{
    if (offset > dstr_length(subject)) {
        mtch->offset = DSTR_NPOS;
        mtch->length = 0;
        return 0;
    }

    pcre2_match_data* mdata =
        pcre2_match_data_create_from_pattern(cr->_pRE, NULL);

    if (!mdata) {
        mtch->offset = DSTR_NPOS;
        mtch->length = 0;
        on_malloc_error();
        return PCRE2_ERROR_NOMEMORY;
    }

    int rc = pcre2_match(cr->_pRE,
                         (PCRE2_SPTR)(dstr_cstr(subject)),
                         dstr_length(subject),
                         offset,
                         match_options(options),
                         mdata,
                         NULL);

    if (rc == PCRE2_ERROR_NOMATCH) {
        mtch->offset = DSTR_NPOS;
        mtch->length = 0;
        pcre2_match_data_free(mdata);
        return 0;
    }
    else if (rc <= 0) {
        pcre2_match_data_free(mdata);
        return rc;
    }

    // Success
    //
    PCRE2_SIZE* ovec = pcre2_get_ovector_pointer(mdata);

    if (ovec[0] == PCRE2_UNSET) {
        mtch->offset = DSTR_NPOS;
        mtch->length = 0;
    }
    else {
        mtch->offset = ovec[0];
        mtch->length = ovec[1] - mtch->offset;
    }
    pcre2_match_data_free(mdata);
    return rc;
}
/*-------------------------------------------------------------------------------*/

static bool dstr_regex_exact_aux(Compiled_Regex* cr,
                                 CDSTR subject, size_t offset,
                                 int options)
{
    DSTR_Regex_Match mtch = {
        .offset = DSTR_NPOS,
        .length = 0,
        .name = { 0 },
    };

    dstr_regex_match_aux(cr, subject, offset, &mtch, options);
    return
        mtch.offset == offset &&
        mtch.length == dstr_length(subject) - offset;
}
/*-------------------------------------------------------------------------------*/

static int
dstr_regex_match_groups_aux(Compiled_Regex* cr,
                            CDSTR subject, size_t offset,
                            DSTR_Match_Vector* vec,
                            int options)
{
    assert (offset <= dstrlen(subject));

    pcre2_match_data* mdata =
        pcre2_match_data_create_from_pattern(cr->_pRE, NULL);

    if (!mdata) {
        on_malloc_error();
        return PCRE2_ERROR_NOMEMORY;
    }

    int rc = pcre2_match(cr->_pRE,
                         (PCRE2_SPTR)(dstr_cstr(subject)),
                         dstr_length(subject),
                         offset,
                         match_options(options) & 0xFFFF,
                         mdata,
                         NULL);

    if (rc <= 0 || vec == NULL) {
        pcre2_match_data_free(mdata);
        if (rc == PCRE2_ERROR_NOMATCH) {
            return 0;
        }
        else {
            return rc;
        }
    }

    PCRE2_SIZE* ovec = pcre2_get_ovector_pointer(mdata);

    vec->matches_len = (size_t) rc;
    vec->matches = RE_MALLOC(DSTR_Regex_Match, vec->matches_len);
    if (!vec->matches) {
        pcre2_match_data_free(mdata);
        return rc;
    }

    for (int i = 0; i < rc; ++i) {
        DSTR_Regex_Match* pM = &vec->matches[i];

        // Set offset and length
        //
        if (ovec[2 * i] == PCRE2_UNSET) {
            pM->offset = DSTR_NPOS;
            pM->length = 0;
        }
        else {
            pM->offset = ovec[2 * i];
            pM->length = ovec[2 * i + 1] - pM->offset;
        }

        // Set group name of match
        //
        const char* name = find_group_name(cr, i);
        if (name) {
            strncpy(pM->name, name, sizeof(pM->name) - 1);
        }
        else {
            pM->name[0] = '\0';
        }
    }

    pcre2_match_data_free(mdata);
    return rc;
}
/*-------------------------------------------------------------------------------*/

static int dstr_regex_subst_aux(Compiled_Regex* cr,
                                DSTR subject, size_t offset,
                                const char* replacement, int options)
{
    if (offset > dstr_length(subject))
        return 0;

    // we try substitution on a stack buffer first
    //
    uint8_t stbuff[512];
    uint8_t* outbuf = stbuff;
    PCRE2_SIZE outlen = sizeof(stbuff);
    uint32_t pcre_opts = PCRE2_SUBSTITUTE_EXTENDED;

    if (options & REGEX_GLOBAL)
        pcre_opts |= PCRE2_SUBSTITUTE_GLOBAL;
    if (options & REGEX_NO_VARS)
        pcre_opts |= PCRE2_SUBSTITUTE_LITERAL;

    // First call - try with local stack buffer
    //
    int rc = pcre2_substitute(cr->_pRE,
                              (PCRE2_SPTR)dstr_cstr(subject),
                              dstr_length(subject),
                              offset,
                              pcre_opts | PCRE2_SUBSTITUTE_OVERFLOW_LENGTH,
                              NULL, NULL, (PCRE2_SPTR)(replacement),
                              strlen(replacement), outbuf, &outlen);

    if (rc == 0) {
        // none replaced
        return rc;
    }
    else if (rc > 0) {
        dstr_assign_bl(subject, (char*) outbuf, outlen);
        return rc;
    }
    else if (rc == PCRE2_ERROR_NOMEMORY) {
        // increase memory to needed and retry
        //
        uint8_t* buffer = RE_MALLOC(uint8_t, outlen);
        if (!buffer) return rc;
        rc = pcre2_substitute(cr->_pRE,
                              (PCRE2_SPTR)dstr_cstr(subject),
                              dstr_length(subject),
                              offset, pcre_opts, NULL, NULL,
                              (PCRE2_SPTR)(replacement),
                              strlen(replacement),
                              buffer,
                              &outlen);

        if (rc > 0)
            dstr_assign_sz(subject, (char*)buffer);

        free(buffer);
    }

    return rc;
}
/*-------------------------------------------------------------------------------*/

// We store at most DSTR_CACHE_SIZE compiled RE patterns for reuse.
// Once the cache is full, the oldest used pattern is discarded
//
#define DSTR_CACHE_SIZE 40

#if defined(RE_CACHE_USE_PTHREAD)
   typedef pthread_key_t    tss_t;
   typedef pthread_once_t   once_flag;
   #define ONCE_FLAG_INIT   PTHREAD_ONCE_INIT
   #define tss_create       pthread_key_create
   #define tss_get          pthread_getspecific
   #define tss_set          pthread_setspecific
   #define call_once        pthread_once
   #define thrd_success     0
#endif

// Since this is unsigned 64 bit type no danger of overflow
// e.g. for a million calls per sec, overflow would occur in
// half a million years...
//
static _Thread_local uint64_t global_tick_count = 0;
static _Thread_local Compiled_Regex* re_cache[DSTR_CACHE_SIZE] = { 0 };

// Code below for cleanup the cache at exit of thread
//
static tss_t     tss_cache_key;
static once_flag tss_cache_once = ONCE_FLAG_INIT;
/*-------------------------------------------------------------------------------*/

static void cache_cleanup(void* unused)
{
    ((void)unused);

    for (size_t i = 0; i < DSTR_CACHE_SIZE; ++i) {
        if (re_cache[i]) {
            destroy_compiled_regex(re_cache[i]);
            re_cache[i] = NULL;
        }
    }
}
/*-------------------------------------------------------------------------------*/

static void cache_cleanup_at_exit(void)
{
    // Argument to cache cleanup is unused (see above)
    // so NULL is OK
    //
    cache_cleanup(NULL);
}
/*-------------------------------------------------------------------------------*/

static void create_cache_key(void)
{
    if (tss_create(&tss_cache_key, cache_cleanup) != thrd_success) {
        fprintf(stderr, "Error: Failed to create TSS key.\n");
        abort();
    }
    atexit(cache_cleanup_at_exit);
}
/*-------------------------------------------------------------------------------*/

static void registr_automatic_exit_cleanup(void)
{
    call_once(&tss_cache_once, create_cache_key);
    if (tss_get(tss_cache_key) == NULL)
        tss_set(tss_cache_key, (void*)1);
}
/*-------------------------------------------------------------------------------*/

#ifdef DEBUG_DISPLAY_CACHE
static void debug_display_cache(void)
{
    static int print = -1;
    if (print < 0) {
        print = (getenv("DSTR_PRINT_CACHE") == NULL) ? 0 : 1;
    }

    if (!print)
        return;

    for (size_t i = 0; i < DSTR_CACHE_SIZE; ++i) {
        if (re_cache[i]) {
            printf("Index:%02zu, T:%3zu, \"%s\", %d\n",
                   i, re_cache[i]->tick_count,
                   dstrdata(re_cache[i]->pattern),
                   re_cache[i]->options);
        }
    }
    putchar('\n');
}
/*-------------------------------------------------------------------------------*/
#endif

static Compiled_Regex* get_RE(const char* pattern, int options, int* errcode)
{
    registr_automatic_exit_cleanup();
    global_tick_count++;

#ifdef DEBUG_DISPLAY_CACHE
    debug_display_cache();
#endif

    Compiled_Regex* pRes = NULL;
    size_t first_empty_slot = DSTR_NPOS;
    size_t oldest_index = DSTR_NPOS;
    size_t oldest_so_far = DSTR_NPOS;

    // Lookup in cache
    //
    for (size_t i = 0; i < DSTR_CACHE_SIZE; ++i) {
        if ((pRes = re_cache[i]) == NULL) {
            first_empty_slot = i;
            break;
        }
        if (pRes->options == options && dstreq(pRes->pattern, pattern)) {
            pRes->tick_count = global_tick_count;
            return pRes;
        }
        else {
            if (pRes->tick_count < oldest_so_far) {
                oldest_index = i;
                oldest_so_far = pRes->tick_count;
            }
        }
    }

    // Not found. Compile and create a new one and store in cache
    //
    if ((pRes = dstr_compile_regex(pattern, options, errcode)) == NULL)
        return NULL;

    // That's the newest member for now
    //
    pRes->tick_count = global_tick_count;

    // If we have an empty slot, store in it
    //
    if (first_empty_slot < DSTR_CACHE_SIZE) {
        re_cache[first_empty_slot] = pRes;
        return pRes;
    }

    // Otherwise cache is full - replace with oldest (LRU)
    //
    assert(oldest_index < DSTR_CACHE_SIZE);
    destroy_compiled_regex(re_cache[oldest_index]);
    re_cache[oldest_index] = pRes;
    return pRes;
}
/*-------------------------------------------------------------------------------*/

bool dstr_regex_exact(CDSTR subject, const char* pattern, size_t offset)
{
    int ctor_opts = (REGEX_CASELESS |
                     REGEX_MULTILINE |
                     REGEX_DOTALL |
                     REGEX_EXTENDED |
                     REGEX_ANCHORED |
                     REGEX_DOLLAR_ENDONLY |
                     REGEX_EXTRA |
                     REGEX_UTF8 |
                     REGEX_NO_AUTO_CAPTURE);

    int match_opts = (REGEX_ANCHORED |
                      REGEX_NOTBOL |
                      REGEX_NOTEOL |
                      REGEX_NOTEMPTY |
                      REGEX_NO_AUTO_CAPTURE |
                      REGEX_NO_UTF8_CHECK);

    Compiled_Regex* cr = get_RE(pattern, ctor_opts, NULL);
    if (!cr)
        return false;

    return dstr_regex_exact_aux(cr, subject, offset, match_opts);
}
/*-------------------------------------------------------------------------------*/

size_t dstr_regex_contains(CDSTR subject, const char* pattern, size_t offset)
{
    int ctor_opts = (REGEX_CASELESS |
                     REGEX_MULTILINE |
                     REGEX_DOTALL |
                     REGEX_EXTENDED |
                     REGEX_DOLLAR_ENDONLY |
                     REGEX_EXTRA |
                     REGEX_UTF8);

    int match_opts = (REGEX_NOTBOL |
                      REGEX_NOTEOL |
                      REGEX_NOTEMPTY |
                      REGEX_NO_UTF8_CHECK);

    Compiled_Regex* cr = get_RE(pattern, ctor_opts, NULL);
    if (!cr) {
        return DSTR_NPOS; }

    DSTR_Regex_Match mtch = { DSTR_NPOS, 0, { 0 } };
    dstr_regex_match_aux(cr, subject, offset, &mtch, match_opts);
    return mtch.offset;
}
/*-------------------------------------------------------------------------------*/

int dstr_regex_match(CDSTR subject, const char* pattern, size_t offset,
                     DSTR_Regex_Match* c_match, const char* opts)
{
    int errcode;
    int options = parse_regex_options(opts);
    Compiled_Regex* cr = get_RE(pattern, options, &errcode);
    if (!cr)
        return errcode;

    int result;
    if (!c_match) {
        DSTR_Regex_Match m;
        result = dstr_regex_match_aux(cr, subject, offset, &m, options); }
    else {
        result = dstr_regex_match_aux(cr, subject, offset, c_match, options); }

    return result;
}
/*-------------------------------------------------------------------------------*/

int dstr_regex_match_groups(CDSTR subject, const char* pattern, size_t offset,
                            DSTR_Match_Vector* matches, const char* opts)
{
    int errcode = 0;
    int options = parse_regex_options(opts);
    Compiled_Regex* cr = get_RE(pattern, options, &errcode);
    if (!cr)
        return errcode;

    return dstr_regex_match_groups_aux(cr, subject, offset, matches, options);
}
/*-------------------------------------------------------------------------------*/

void dstr_regex_match_vector_free(DSTR_Match_Vector* vec)
{
    if (vec && vec->matches) {
        free(vec->matches);
        vec->matches = NULL;
        vec->matches_len = 0;
    }
}
/*-------------------------------------------------------------------------------*/

int dstr_regex_substitute(DSTR subject, const char* pattern, size_t offset,
                          const char* replacement, const char* opts)
{
    int errcode = 0;
    int options = parse_regex_options(opts);
    Compiled_Regex* cr = get_RE(pattern, options, &errcode);
    if (!cr)
        return errcode;

    return dstr_regex_subst_aux(cr, subject, offset, replacement, options);
}
/*-------------------------------------------------------------------------------*/

static void dstr_regex_perror_aux(int rc, DSTR p)
{
    char buffer[512];

    if (rc > REGEX_COMPILE_ERROR_BASE)
        rc -= REGEX_COMPILE_ERROR_BASE;

    pcre2_get_error_message(rc, (PCRE2_UCHAR*) buffer, sizeof(buffer));

    if (p == NULL) {
        fprintf(stderr, "DSTR Regex (pcre2 error): %s (%d)\n", buffer, rc); }
    else {
        dsprintf(p, "DSTR Regex (pcre2 error): %s (%d)\n", buffer, rc); }
}
/*-------------------------------------------------------------------------------*/

void dstr_regex_perror(int rc)
{
    dstr_regex_perror_aux(rc, NULL);
}
/*-------------------------------------------------------------------------------*/

void dstr_regex_strerror(DSTR dest, int rc)
{
    dstr_regex_perror_aux(rc, dest);
}
/*-------------------------------------------------------------------------------*/
