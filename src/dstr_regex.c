/*
 * Part of this file is based on POCO C++ Libraries
 * Original work Copyright (c) 2006-2023, Applied Informatics Software Engineering GmbH.
 * Licensed under the Boost Software License, Version 1.0.
 *
 * Modifications and additional code:
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#include <assert.h>
#include <stdint.h>
#include <dstr/dstr.h>

#include "dstr_regex_common.h"

typedef struct GroupInfo {
    long        group_number;
    const char* group_name;
} GroupInfo;

typedef struct Compiled_Regex
{
    pcre2_code* _pRE;
    GroupInfo*  p_groups;
    size_t      n_groups;
    DSTR        pattern;
    int         options;
} Compiled_Regex;
/*-------------------------------------------------------------------------------*/

static const char* find_group_name(Compiled_Regex* cr, int n)
{
    for (size_t i = 0; i < cr->n_groups; ++i) {
        if (cr->p_groups[i].group_number == n)
            return cr->p_groups[i].group_name;
    }
    return NULL;
}
/*-------------------------------------------------------------------------------*/

static Compiled_Regex* dstr_compile_regex(const char* pattern, int options, int* errcode)
{
    pcre2_compile_context* context = pcre2_compile_context_create(NULL);
    if (!context)
        return NULL;

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

    if (!_pRE)
    {
        if (errcode) *errcode = error_code;
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
        gInfo = (GroupInfo*) malloc(name_count * sizeof(GroupInfo));
        if (!gInfo)
            goto error_clean_pcre;
        for (uint32_t i = 0; i < name_count; i++)
        {
            unsigned char* group = name_table + 2 + (name_entry_size * i);
            int n = pcre2_substring_number_from_name(_pRE, group);
            gInfo[i].group_number = n;
            gInfo[i].group_name = (const char*)group;
        }
    }

    Compiled_Regex* result = (Compiled_Regex*) malloc(sizeof(Compiled_Regex));
    if (!result)
        goto error_clean_groups;
    result->_pRE = _pRE;
    result->p_groups = gInfo;
    result->n_groups = name_count;
    result->pattern = dstrnew(pattern);
    result->options = options;
    return result;

error_clean_groups:
    if (gInfo) free(gInfo);
error_clean_pcre:
    pcre2_code_free(_pRE);
    return NULL;
}
/*-------------------------------------------------------------------------------*/

static void destroy_compiled_regex(Compiled_Regex* cr)
{
    if (!cr)
        return;

    if (cr->pattern)
        dstrfree(cr->pattern);

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
    if (offset > dstrlen(subject)) {
        mtch->offset = DSTR_NPOS;
        mtch->length = 0;
        return 0;
    }

    pcre2_match_data* mdata = pcre2_match_data_create_from_pattern(cr->_pRE, NULL);

    int rc = pcre2_match(cr->_pRE,
                         (PCRE2_SPTR)(dstrdata(subject)),
                         dstrlen(subject),
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
        mtch.length == dstrlen(subject) - offset;
}
/*-------------------------------------------------------------------------------*/

static int
dstr_regex_match_groups_aux(Compiled_Regex* cr,
                            CDSTR subject, size_t offset,
                            DSTR_Regex_Match matches[], size_t mlen,
                            int options)
{
    assert (offset <= dstrlen(subject));

    pcre2_match_data* mdata =
        pcre2_match_data_create_from_pattern(cr->_pRE, NULL);

    int rc = pcre2_match(cr->_pRE,
                         (PCRE2_SPTR)(dstrdata(subject)),
                         dstrlen(subject),
                         offset,
                         match_options(options) & 0xFFFF,
                         mdata,
                         NULL);

    if (rc <= 0) {
        pcre2_match_data_free(mdata);
        return rc;
    }

    PCRE2_SIZE* ovec = pcre2_get_ovector_pointer(mdata);

    for (int i = 0; i < rc; ++i) {
        // Don't write more than supplied
        //
        if (i == (int)mlen)
            break;

        // Set offset and length
        //
        if (ovec[2 * i] == PCRE2_UNSET) {
            matches[i].offset = DSTR_NPOS;
            matches[i].length = 0;
        }
        else {
            matches[i].offset = ovec[2 * i];
            matches[i].length = ovec[2 * i + 1] - matches[i].offset;
        }

        // Set group name of match
        //
        const char* name = find_group_name(cr, i);
        if (name) {
            strncpy(matches[i].name, name, sizeof(matches[i].name) - 1);
        }
        else {
            matches[i].name[0] = '\0';
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
                              (PCRE2_SPTR)dstrdata(subject),
                              dstrlen(subject),
                              offset,
                              pcre_opts | PCRE2_SUBSTITUTE_OVERFLOW_LENGTH,
                              NULL, NULL, (PCRE2_SPTR)(replacement),
                              strlen(replacement), outbuf, &outlen);

    if (rc == 0) {
        // none replaced
        return rc;
    }
    else if (rc > 0) {
        dstrcpy_bl(subject, (char*) outbuf, outlen);
        return rc;
    }
    else if (rc == PCRE2_ERROR_NOMEMORY) {
        // increase memory to needed and retry
        //
        uint8_t* buffer = (uint8_t*) malloc(outlen);
        if (!buffer) return rc;
        rc = pcre2_substitute(cr->_pRE,
                              (PCRE2_SPTR)dstrdata(subject),
                              dstrlen(subject),
                              offset, pcre_opts, NULL, NULL,
                              (PCRE2_SPTR)(replacement),
                              strlen(replacement),
                              buffer,
                              &outlen);

        if (rc > 0) {
            dstrcpy(subject, (char*)buffer);
            free(buffer);
            return rc;
        }
    }

    return rc;
}
/*-------------------------------------------------------------------------------*/

// Not thread safe for now
//
// TODO:
// 1. Thread safe
// 2. LRU
//
#define DSTR_CACHE_SIZE 20
#define HALF_CACHE_SIZE (DSTR_CACHE_SIZE / 2)
static Compiled_Regex* re_cache[DSTR_CACHE_SIZE] = { 0 };
static size_t cache_index = 0;
static bool set_cleanup = false;

static void cache_cleanup(void)
{
    for (size_t i = 0; i < DSTR_CACHE_SIZE; ++i) {
        if (re_cache[i]) {
            destroy_compiled_regex(re_cache[i]);
            re_cache[i] = NULL;
        }
    }
}
/*-------------------------------------------------------------------------------*/

static Compiled_Regex* get_RE(const char* pattern, int options, int* errcode)
{
    if (!set_cleanup) {
        atexit(cache_cleanup);
        set_cleanup = true;
    }

    for (size_t i = 0; i < cache_index; ++i) {
        if (dstreq(re_cache[i]->pattern, pattern) &&
            re_cache[i]->options == options)
        {
            return re_cache[i];
        }
    }

    Compiled_Regex* newreg = dstr_compile_regex(pattern, options, errcode);
    if (!newreg) return NULL;

    // If full, destroy first half, then move second half to first
    //
    if (cache_index == DSTR_CACHE_SIZE)
    {
        for (size_t i = 0; i < HALF_CACHE_SIZE; ++i)
        {
            destroy_compiled_regex(re_cache[i]);
            re_cache[i] = re_cache[i + HALF_CACHE_SIZE];
            re_cache[i + HALF_CACHE_SIZE] = NULL;
        }
        cache_index = HALF_CACHE_SIZE;
    }

    re_cache[cache_index] = newreg;
    ++cache_index;
    return newreg;
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

    DSTR_Regex_Match mtch;
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

int dstr_regex_substitute(DSTR subject, const char* pattern, size_t offset,
                          const char* replacement, const char* opts)
{
    int errcode;
    int options = parse_regex_options(opts);
    Compiled_Regex* cr = get_RE(pattern, options, &errcode);
    if (!cr)
        return errcode;

    return dstr_regex_subst_aux(cr, subject, offset, replacement, options);
}
/*-------------------------------------------------------------------------------*/

int dstr_regex_match_groups(CDSTR subject, const char* pattern, size_t offset,
                            DSTR_Regex_Match matches[], size_t mlen,
                            const char* opts)
{
    int errcode;
    int options = parse_regex_options(opts);
    Compiled_Regex* cr = get_RE(pattern, options, &errcode);
    if (!cr)
        return errcode;

    return dstr_regex_match_groups_aux(cr, subject, offset, matches, mlen, options);
}
/*-------------------------------------------------------------------------------*/
