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
#include <memory>
#include <cassert>
#include <deque>
#include <cstdint>

#include <dstr/dstring.hpp>
#include <dstr/dstring_view.hpp>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

// TODO: USE PCRE2 constants directly, nor need for extra constants
//
// Constants for options (based on POCO)
//
enum {
    REGEX_CASELESS        = 0x00000001, /// case insensitive matching (/i) [ctor]
    REGEX_MULTILINE       = 0x00000002, /// enable multi-line mode; affects ^ and $ (/m) [ctor]
    REGEX_DOTALL          = 0x00000004, /// dot matches all characters, including newline (/s) [ctor]
    REGEX_EXTENDED        = 0x00000008, /// totally ignore whitespace (/x) [ctor]
    REGEX_ANCHORED        = 0x00000010, /// treat pattern as if it starts with a ^ [ctor, match]
    REGEX_DOLLAR_ENDONLY  = 0x00000020, /// dollar matches end-of-string only, not last newline in string [ctor]
    REGEX_EXTRA           = 0x00000040, /// enable optional PCRE functionality [ctor]
    REGEX_NOTBOL          = 0x00000080, /// circumflex does not match beginning of string [match]
    REGEX_NOTEOL          = 0x00000100, /// $ does not match end of string [match]
    REGEX_UNGREEDY        = 0x00000200, /// make quantifiers ungreedy [ctor]
    REGEX_NOTEMPTY        = 0x00000400, /// empty string never matches [match]
    REGEX_UTF8            = 0x00000800, /// assume pattern and subject is UTF-8 encoded [ctor]
    REGEX_NO_AUTO_CAPTURE = 0x00001000, /// disable numbered capturing parentheses [ctor, match]
    REGEX_NO_UTF8_CHECK   = 0x00002000, /// do not check validity of UTF-8 code sequences [match]
    REGEX_FIRSTLINE       = 0x00040000, /// an  unanchored  pattern  is  required  to  match
    REGEX_DUPNAMES        = 0x00080000, /// names used to identify capturing  subpatterns need not be unique [ctor]
    REGEX_NEWLINE_CR      = 0x00100000, /// assume newline is CR ('\r'), the default [ctor]
    REGEX_NEWLINE_LF      = 0x00200000, /// assume newline is LF ('\n') [ctor]
    REGEX_NEWLINE_CRLF    = 0x00300000, /// assume newline is CRLF ("\r\n") [ctor]
    REGEX_NEWLINE_ANY     = 0x00400000, /// assume newline is any valid Unicode newline character [ctor]
    REGEX_NEWLINE_ANYCRLF = 0x00500000, /// assume newline is any of CR, LF, CRLF [ctor]
    REGEX_GLOBAL          = 0x10000000, /// replace all occurences (/g) [subst]
    REGEX_NO_VARS         = 0x20000000  /// treat dollar in replacement string as ordinary character [subst]
};

namespace {

// Encapsulates a PCRE2 regular expression object
//
class DStringRegex
{
public:
    DStringRegex(DStringView pattern, int options = 0);
    ~DStringRegex();

    // Implemented out of line below
    //
    bool match(DStringView subject, size_t offset, int options) const;
    int  match(DStringView s, size_t off, DString::Match& m, int opts) const;
    int  match_groups(DStringView s, size_t off, DString::MatchVector& m,
                      int opts) const;
    int capture(DStringView subject, size_t offset,
                DString& result, int options) const;
    int capture(DStringView subject, size_t offset,
                std::vector<DString>& strings, int options) const;
    int subst(DString& subject, size_t offset, DStringView repl,
              int options) const;

private:
    // Actual type is pcre2_code_8*
    pcre2_code* _pRE;

    struct GroupInfo {
        long        group_number;
        const char* group_name;
    };
    std::vector<GroupInfo> m_groups;

    const char* find_group_name(int n) const {
        for (const auto& grp : m_groups) {
            if (grp.group_number == n)
                return grp.group_name;
        }
        return NULL;
    }

private:
    // Prevent default, copy and assignment
    //
    DStringRegex() = delete;
    DStringRegex(const DStringRegex&) = delete;
    DStringRegex& operator=(const DStringRegex&) = delete;
    DStringRegex(DStringRegex&&) = delete;
    DStringRegex& operator=(DStringRegex&&) = delete;
};
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

// RAII wrapper around pcre2_match_data*
//
struct MatchData {
    MatchData(pcre2_code* code)
        :
        ovec(nullptr),
        _match(pcre2_match_data_create_from_pattern(code, nullptr))
    {
       if (!_match)
           throw DStringError("cannot create match data");
    }
    ~MatchData()  {
        if (_match) pcre2_match_data_free(_match);
    }
    pcre2_match_data* pointer() { return _match; }
    PCRE2_SIZE operator[](size_t index) const {
        if (!ovec) ovec = data();
        return ovec[index];
    }

private:
    const PCRE2_SIZE* data() const { return pcre2_get_ovector_pointer(_match); }
    mutable const PCRE2_SIZE* ovec = nullptr;
    pcre2_match_data* _match;  // actual pointer
};
/*-------------------------------------------------------------------------------*/
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

DStringRegex::DStringRegex(DStringView pattern, int options)
    :
    _pRE(nullptr)
{
    pcre2_compile_context* context = pcre2_compile_context_create(nullptr);
    if (!context)
        throw DStringError("cannot create compile context");

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
    this->_pRE = pcre2_compile((PCRE2_SPTR)pattern.c_str(),
                               pattern.length(),
                               compile_options(options),
                               &error_code,
                               &error_offset,
                               context);

    pcre2_compile_context_free(context);

    if (!_pRE)
    {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(error_code, buffer, sizeof(buffer));
        DString msg = DString::format("PCRE2: %s (at offset %zu)",
                                      (char*)buffer,
                                      error_offset);

        throw DStringError(std::move(msg));
    }

    unsigned int name_count = 0;
    pcre2_pattern_info(_pRE, PCRE2_INFO_NAMECOUNT, &name_count);

    unsigned int name_entry_size = 0;
    pcre2_pattern_info(_pRE, PCRE2_INFO_NAMEENTRYSIZE, &name_entry_size);

    unsigned char* name_table = nullptr;
    pcre2_pattern_info(_pRE, PCRE2_INFO_NAMETABLE, &name_table);

    if (name_count)
        m_groups.reserve(name_count);

    for (uint32_t i = 0; i < name_count; i++)
    {
        unsigned char* group = name_table + 2 + (name_entry_size * i);
        int n = pcre2_substring_number_from_name(_pRE, group);
        m_groups.push_back({n, (const char*)group});
    }
}
/*-------------------------------------------------------------------------------*/

DStringRegex::~DStringRegex()
{
    if (_pRE)
        pcre2_code_free(_pRE);
}
/*-------------------------------------------------------------------------------*/

int DStringRegex::match(DStringView subject, size_t offset,
                        DString::Match& mtch,
                        int options) const
{
    if (offset > subject.length()) {
        mtch.offset = DString::NPOS;
        mtch.length = 0;
        return 0;
    }

    MatchData mdata(_pRE);

    int rc = pcre2_match(_pRE,
                         (PCRE2_SPTR)(subject.c_str()),
                         subject.size(),
                         offset,
                         match_options(options),
                         mdata.pointer(), nullptr);

    // Success
    //
    if (rc > 0) {
        if (mdata[0] == PCRE2_UNSET) {
            mtch.offset = DString::NPOS;
            mtch.length = 0;
        }
        else {
            mtch.offset = mdata[0];
            mtch.length = mdata[1] - mtch.offset;
        }
        return rc;
    }

    // handle error
    //
    switch (rc) {
    case 0:
        throw DStringError("too many captured substrings");
    case PCRE2_ERROR_NOMATCH:
        mtch.offset = DString::NPOS;
        mtch.length = 0;
        return 0;
    case PCRE2_ERROR_BADOPTION:
        throw DStringError("bad option");
    default:
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(rc, buffer, sizeof(buffer));
        throw DStringError((char*)buffer);
    }
}
/*-------------------------------------------------------------------------------*/

bool DStringRegex::match(DStringView subject, size_t offset, int options) const
{
    DString::Match mtch;
    match(subject, offset, mtch, options);
    // printf("Match offset: %zu, len: %zu\n",  mtch.offset,  mtch.length);
    return
        mtch.offset == offset &&
        mtch.length == subject.length() - offset;
}
/*-------------------------------------------------------------------------------*/

int DStringRegex::match_groups(DStringView subject, size_t offset,
                               DString::MatchVector& matches,
                               int options) const
{
    assert (offset <= subject.length());

    DString::MatchVector mvec;
    MatchData mdata(_pRE);

    int rc = pcre2_match(_pRE,
                         (PCRE2_SPTR)(subject.c_str()),
                         subject.size(),
                         offset,
                         match_options(options) & 0xFFFF,
                         mdata.pointer(),
                         nullptr);

    if (rc <= 0)
        goto handle_error;

    // Success
    //
    for (int i = 0; i < rc; ++i) {
        DString::Match m;

        // Set offset and length
        //
        if (mdata[2 * i] == PCRE2_UNSET) {
            m.offset = DString::NPOS;
            m.length = 0;
        }
        else {
            m.offset = mdata[2 * i];
            m.length = mdata[2 * i + 1] - m.offset;
        }

        // Set group name of match
        //
        if (const char* name = find_group_name(i)) {
            m.name = name;
        }
        else {
            m.name = "";
        }

        mvec.push_back(m);
    }

    mvec.swap(matches);
    return rc;

handle_error:
    switch (rc) {
    case 0:
        throw DStringError("too many captured substrings");
    case PCRE2_ERROR_NOMATCH:
        return 0;
    case PCRE2_ERROR_BADOPTION:
        throw DStringError("bad option");
    default:
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(rc, buffer, sizeof(buffer));
        throw DStringError((char*)buffer);
    }
}
/*-------------------------------------------------------------------------------*/

int DStringRegex::capture(DStringView subject, size_t offset,
                          DString& result,
                          int options) const
{
    DString::Match mtch;

    int rc = match(subject, offset, mtch, options);

    if (rc > 0 && mtch.offset != DString::NPOS)
        result = subject.substr(mtch.offset, mtch.length);

    return rc;
}
/*-------------------------------------------------------------------------------*/

int DStringRegex::capture(DStringView subject, size_t offset,
                          std::vector<DString>& strings,
                          int options) const
{
    DString::MatchVector matches;
    std::vector<DString> tmp;

    int rc = match_groups(subject, offset, matches, options);

    for (const auto& m: matches)
    {
        if (m.offset != DString::NPOS)
            tmp.push_back({subject, m.offset, m.length});
        else
            tmp.push_back("");
    }

    strings.swap(tmp);
    return rc;
}
/*-------------------------------------------------------------------------------*/

int DStringRegex::subst(DString& subject, size_t offset,
                        DStringView replacement,
                        int options) const
{
    if (offset > subject.length())
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
    int rc = pcre2_substitute(_pRE,
                              (PCRE2_SPTR)subject.c_str(), subject.length(),
                              offset,
                              pcre_opts | PCRE2_SUBSTITUTE_OVERFLOW_LENGTH,
                              NULL, NULL, (PCRE2_SPTR)replacement.c_str(),
                              replacement.length(), outbuf, &outlen);

    if (rc == 0) {
        // none replaced
        return 0;
    }
    else if (rc > 0) {
        subject.assign((char*) outbuf, outlen);
        return rc;
    }
    else if (rc == PCRE2_ERROR_NOMEMORY) {
        // increase memory to needed and retry
        //
        std::vector<uint8_t> v;
        v.reserve(outlen);
        rc = pcre2_substitute(_pRE,
                              (PCRE2_SPTR)subject.c_str(),  subject.length(),
                              offset, pcre_opts, NULL, NULL,
                              (PCRE2_SPTR)replacement.c_str(),
                              replacement.length(), v.data(), &outlen);

        if (rc > 0) {
            subject.assign((char*)v.data());
            return rc;
        }
    }

    if (rc < 0) {
        PCRE2_UCHAR errbuf[256];
        pcre2_get_error_message(rc, errbuf, sizeof(errbuf));
        DString msg = DString::format("PCRE2 substitute error (%d): %s", rc,
                                      (char*)errbuf);
        throw DStringError(std::move(msg));
    }

    return rc;
}
/*-------------------------------------------------------------------------------*/

// Borrowed std::make_unique from C++14 code
//
#if __cplusplus <= 201103L
template<typename _Tp, typename... _Args>
inline std::unique_ptr<_Tp> make_unique(_Args&&... __args)
{
    return std::unique_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...));
}
#else
using std::make_unique;
#endif
/*-------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

struct DStringRegexCache
{
    static const size_t LIMIT = 30;

    struct RegexCacheEntry {
        RegexCacheEntry(DStringView rhs, int opts)
            :
            pattern(rhs),
            options(opts),
            pRE(make_unique<DStringRegex>(pattern, options))
        {}

        DString pattern;
        int options;
        std::unique_ptr<DStringRegex> pRE;
    };

    const DStringRegex& get_RE(DStringView pattern, int options) {
        for (const auto& entry : fifo) {
            if (pattern == entry.pattern && options == entry.options) {
                return *entry.pRE;
            }
        }

        if (fifo.size() >= LIMIT)
            remove_oldest_if_full();

        fifo.emplace_back(pattern, options);
        return *(fifo.back().pRE);
    }

    void remove_oldest_if_full() {
        while (fifo.size() >= LIMIT) {
            fifo.pop_front();
        }
    }

    std::deque<RegexCacheEntry> fifo;
};

#ifdef __BORLANDC__
#if !defined(__clang__) || (__clang_major__ < 15)
#define thread_local
#endif
#endif
static thread_local DStringRegexCache re_cache;

}  // unnamed ns
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

//
//  DSTRING REGEX API
//
bool DString::match(DStringView pattern, size_t offset) const
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

    const auto& re = re_cache.get_RE(pattern, ctor_opts);
    return re.match(*this, offset, match_opts);
}
/*-------------------------------------------------------------------------------*/

size_t DString::match_within(DStringView pattern, size_t offset) const
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

    const auto& re = re_cache.get_RE(pattern, ctor_opts);

    DString::Match mtch;
    re.match(*this, offset, mtch, match_opts);
    return mtch.offset;
}
/*-------------------------------------------------------------------------------*/

static int parse_regex_options(const char* str)
{
    if (!str || !*str)
        return 0;

    int opts = 0;
    for (; *str; ++str) {
        switch (*str) {
        case ' ':
        case '/': break;
        case 'g': opts |= REGEX_GLOBAL;     break;
        case 'i': opts |= REGEX_CASELESS;   break;
        case 'm': opts |= REGEX_MULTILINE;  break;
        case 's': opts |= REGEX_DOTALL;     break;
        case 'x': opts |= REGEX_EXTENDED;   break;
        case 'X': opts |= REGEX_EXTRA;      break;
        case 'U': opts |= REGEX_UNGREEDY;   break;
        case 'D': opts |= REGEX_DOLLAR_ENDONLY;  break;
        case 'd': opts |= REGEX_NOTEOL;          break;
        case 'E': opts |= REGEX_NOTEMPTY;        break;
        case 'n': opts |= REGEX_NO_AUTO_CAPTURE; break;
        case 'F': opts |= REGEX_FIRSTLINE;       break;
        case 'A': opts |= REGEX_ANCHORED;        break;
        case 't': opts |= REGEX_DUPNAMES;        break;
        case '$': opts |= REGEX_NO_VARS;         break;
        case '\n': opts |= REGEX_NEWLINE_LF;     break;
        case '\r': opts |= REGEX_NEWLINE_CR;     break;
        default:
            throw DStringError(DString::format("invalid regex option: '%c'", *str));
        }
    }
    return opts;
}
/*-------------------------------------------------------------------------------*/

int DStringView::capture(DStringView pattern,
                         size_t offset,
                         DString& result,
                         const char* opts) const
{
    int options = parse_regex_options(opts);
    auto& re = re_cache.get_RE(pattern, options);
    return re.capture(*this, offset, result, options);
}
/*-------------------------------------------------------------------------------*/

int DStringView::capture(DStringView pattern,
                         size_t offset,
                         std::vector<DString>& vec,
                         const char* opts) const
{
    int options = parse_regex_options(opts);
    const auto& re = re_cache.get_RE(pattern, options);
    return re.capture(*this, offset, vec, options);
}
/*-------------------------------------------------------------------------------*/

int DStringView::match(DStringView pattern,
                       size_t offset,
                       DString::Match& m,
                       const char* opts) const
{
    int options = parse_regex_options(opts);
    const auto& re = re_cache.get_RE(pattern, options);
    return re.match(*this, offset, m, options);
}
/*-------------------------------------------------------------------------------*/

int DStringView::match_groups(DStringView pattern, size_t offset,
                              DString::MatchVector& matches,
                              const char* opts) const
{
    int options = parse_regex_options(opts);
    const auto& re = re_cache.get_RE(pattern, options);
    return re.match_groups(*this, offset, matches, options);
}
/*-------------------------------------------------------------------------------*/

DString DString::capture(DStringView pattern,
                         size_t offset,
                         const char* opts) const
{
    DString result;
    view().capture(pattern, offset, result, opts);
    return result;
}
/*-------------------------------------------------------------------------------*/

int DString::capture(DStringView pattern,
                     size_t offset,
                     std::vector<DString>& vec,
                     const char* opts) const
{
    return view().capture(pattern, offset, vec, opts);
}
/*-------------------------------------------------------------------------------*/

int DString::match(DStringView pattern,
                   size_t offset,
                   DString::Match& m,
                   const char* opts) const
{
    return view().match(pattern, offset, m, opts);
}
/*-------------------------------------------------------------------------------*/

int DString::match_groups(DStringView pattern, size_t offset,
                          DString::MatchVector& matches,
                          const char* opts) const
{
    return view().match_groups(pattern, offset, matches, opts);
}
/*-------------------------------------------------------------------------------*/

int DString::subst_inplace(DStringView pattern, size_t offset,
                           DStringView replacement,
                           const char* opts)
{
    int options = parse_regex_options(opts);
    const auto& re = re_cache.get_RE(pattern, options);
    return re.subst(*this, offset, replacement, options);
}
/*-------------------------------------------------------------------------------*/
