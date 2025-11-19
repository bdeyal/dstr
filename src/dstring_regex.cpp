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
#include <vector>
#include <cstdint>

#include <dstr/dstring.hpp>
#include <dstr/dstring_view.hpp>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

namespace {

// Encapsulates a PCRE2 regular expression object
//
class DStringRegex
{
public:
    DStringRegex(DStringView pattern, int options = 0);
    ~DStringRegex();

    // Implemented below
    //
    bool match(DStringView subject, size_t offset, int options) const;
    int match(DStringView s, size_t off, DString::Match& m, int opts) const;
    int match_groups(DStringView s, size_t off, DString::MatchVector& m,  int opts) const;
    DString capture(DStringView subject, size_t offset, int options) const;
    int capture(DStringView subject, size_t offset, std::vector<DString>& strings, int options) const;
    int subst(DString& subject, size_t offset, DStringView repl, int options) const;

    // Inlines using the above
    //
    int match(DStringView s, DString::Match& m, int opts) const {
        return match(s, 0, m, opts);
    }
    int match_groups(DStringView s, DString::MatchVector& m, int opts) const {
        return match_groups(s, 0, m, opts);
    }
    DString capture(DStringView subject, int options) const {
        return capture(subject, 0, options);
    }
    int capture(DStringView subject, std::vector<DString>& strings, int options) const {
        return capture(subject, 0, strings, options);
    }
    int subst(DString& s, DStringView r, int options) const {
        return subst(s, 0, r, options);
    }
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
    size_t subst_single(DString& subject, size_t offset, DStringView repl, int options) const;

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

    if (options & DSTR_REGEX_CASELESS)
        pcre_opts |= PCRE2_CASELESS;
    if (options & DSTR_REGEX_MULTILINE)
        pcre_opts |= PCRE2_MULTILINE;
    if (options & DSTR_REGEX_DOTALL)
        pcre_opts |= PCRE2_DOTALL;
    if (options & DSTR_REGEX_EXTENDED)
        pcre_opts |= PCRE2_EXTENDED;
    if (options & DSTR_REGEX_ANCHORED)
        pcre_opts |= PCRE2_ANCHORED;
    if (options & DSTR_REGEX_DOLLAR_ENDONLY)
        pcre_opts |= PCRE2_DOLLAR_ENDONLY;
    if (options & DSTR_REGEX_UNGREEDY)
        pcre_opts |= PCRE2_UNGREEDY;
    if (options & DSTR_REGEX_UTF8)
        pcre_opts |= PCRE2_UTF | PCRE2_UCP;
    if (options & DSTR_REGEX_NO_AUTO_CAPTURE)
        pcre_opts |= PCRE2_NO_AUTO_CAPTURE;
    if (options & DSTR_REGEX_FIRSTLINE)
        pcre_opts |= PCRE2_FIRSTLINE;
    if (options & DSTR_REGEX_DUPNAMES)
        pcre_opts |= PCRE2_DUPNAMES;

    return pcre_opts;
}
/*-------------------------------------------------------------------------------*/

static int match_options(int options)
{
    int pcre_opts = 0;

    if (options & DSTR_REGEX_ANCHORED)
        pcre_opts |= PCRE2_ANCHORED;
    if (options & DSTR_REGEX_NOTBOL)
        pcre_opts |= PCRE2_NOTBOL;
    if (options & DSTR_REGEX_NOTEOL)
        pcre_opts |= PCRE2_NOTEOL;
    if (options & DSTR_REGEX_NOTEMPTY)
        pcre_opts |= PCRE2_NOTEMPTY;
    if (options & DSTR_REGEX_NO_AUTO_CAPTURE)
        pcre_opts |= PCRE2_NO_AUTO_CAPTURE;
    if (options & DSTR_REGEX_NO_UTF8_CHECK)
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

    if (options & DSTR_REGEX_NEWLINE_LF)
        pcre2_set_newline(context, PCRE2_NEWLINE_LF);
    else if (options & DSTR_REGEX_NEWLINE_CRLF)
        pcre2_set_newline(context, PCRE2_NEWLINE_CRLF);
    else if (options & DSTR_REGEX_NEWLINE_ANY)
        pcre2_set_newline(context, PCRE2_NEWLINE_ANY);
    else if (options & DSTR_REGEX_NEWLINE_ANYCRLF)
        pcre2_set_newline(context, PCRE2_NEWLINE_ANYCRLF);
    else // default DSTR_REGEX_NEWLINE_CR
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
        DString msg;
        msg.sprintf("PCRE2: %s (at offset %zu)", (char*)buffer, error_offset);
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
            strncpy(m.name, name, sizeof(m.name) - 1);
        }
        else {
            m.name[0] = '\0';
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

DString DStringRegex::capture(DStringView subject, size_t offset,
                              int options) const
{
    DString::Match mtch;
    int rc = match(subject, offset, mtch, options);
    if (rc > 0 && mtch.offset != DString::NPOS)
        return DString(subject, mtch.offset, mtch.length);
    else
        return DString();
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
    if (options & DSTR_REGEX_GLOBAL)
    {
        int rc = 0;
        size_t pos = subst_single(subject, offset, replacement, options);
        while (pos != DString::NPOS)
        {
            ++rc;
            pos = subst_single(subject, pos, replacement, options);
        }
        return rc;
    }

    size_t n = subst_single(subject, offset, replacement, options);
    return (n == DString::NPOS) ? 0 : 1;
}
/*-------------------------------------------------------------------------------*/

size_t DStringRegex::subst_single(DString& subject, size_t offset,
                                  DStringView replacement,
                                  int options) const
{
    if (offset >= subject.length()) return DString::NPOS;

    MatchData mdata(_pRE);

    int rc = pcre2_match(_pRE,
                         (PCRE2_SPTR)(subject.c_str()),
                         subject.size(),
                         offset,
                         match_options(options),
                         mdata.pointer(),
                         nullptr);

    // Handle error
    //
    if (rc <= 0) {
        switch (rc) {
        case 0:
            throw DStringError("too many captured substrings");
        case PCRE2_ERROR_NOMATCH:
            return DString::NPOS;
        case PCRE2_ERROR_BADOPTION:
            throw DStringError("bad option");
        default:
            PCRE2_UCHAR buffer[256];
            pcre2_get_error_message(rc, buffer, sizeof(buffer));
            throw DStringError((char*)buffer);
        }
    }

    // Success (rc > 0)
    //
    DString result;
    size_t len = subject.length();
    size_t pos = 0;
    size_t rp = DString::NPOS;
    while (pos < len)
    {
        if (mdata[0] == pos)
        {
            DString::const_iterator it  = replacement.begin();
            DString::const_iterator end = replacement.end();
            while (it != end)
            {
                if (*it == '$' && !(options & DSTR_REGEX_NO_VARS))
                {
                    ++it;
                    if (it != end)
                    {
                        char d = *it;
                        if (d >= '0' && d <= '9')
                        {
                            int c = d - '0';
                            if (c < rc)
                            {
                                size_t o = mdata[c*2];
                                size_t l = mdata[c*2 + 1] - o;
                                result.append(&subject[o], l);
                            }
                        }
                        else
                        {
                            result += '$';
                            result += d;
                        }
                        ++it;
                    }
                    else result += '$';
                }
                else result += *it++;
            }
            pos = mdata[1];
            rp = result.length();
        }
        else result += subject[pos++];
    }
    subject = result;
    return rp;
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

template <size_t LIMIT>
struct DStringRegexCache
{
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

#if defined(__BORLANDC__)
#if !defined(__clang__) || (__clang_major__ < 15)
#define thread_local /**/
#endif
#endif
static thread_local DStringRegexCache<30> re_cache;

}  // unnamed ns
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

//
//  DSTRING REGEX API
//
bool DString::match(DStringView pattern, size_t offset) const
{
    int ctor_opts = (DSTR_REGEX_CASELESS |
                     DSTR_REGEX_MULTILINE |
                     DSTR_REGEX_DOTALL |
                     DSTR_REGEX_EXTENDED |
                     DSTR_REGEX_ANCHORED |
                     DSTR_REGEX_DOLLAR_ENDONLY |
                     DSTR_REGEX_EXTRA |
                     DSTR_REGEX_UTF8 |
                     DSTR_REGEX_NO_AUTO_CAPTURE);

    int match_opts = (DSTR_REGEX_ANCHORED |
                      DSTR_REGEX_NOTBOL |
                      DSTR_REGEX_NOTEOL |
                      DSTR_REGEX_NOTEMPTY |
                      DSTR_REGEX_NO_AUTO_CAPTURE |
                      DSTR_REGEX_NO_UTF8_CHECK);

    const auto& re = re_cache.get_RE(pattern, ctor_opts);
    return re.match(*this, offset, match_opts);
}
/*-------------------------------------------------------------------------------*/

size_t DString::match_within(DStringView pattern, size_t offset) const
{
    int ctor_opts = (DSTR_REGEX_CASELESS |
                     DSTR_REGEX_MULTILINE |
                     DSTR_REGEX_DOTALL |
                     DSTR_REGEX_EXTENDED |
                     DSTR_REGEX_DOLLAR_ENDONLY |
                     DSTR_REGEX_EXTRA |
                     DSTR_REGEX_UTF8);

    int match_opts = (DSTR_REGEX_NOTBOL |
                      DSTR_REGEX_NOTEOL |
                      DSTR_REGEX_NOTEMPTY |
                      DSTR_REGEX_NO_UTF8_CHECK);

    const auto& re = re_cache.get_RE(pattern, ctor_opts);

    DString::Match mtch;
    re.match(*this, offset, mtch, match_opts);
    return mtch.offset;
}
/*-------------------------------------------------------------------------------*/

DString DString::capture(DStringView pattern,
                         size_t offset,
                         int options) const
{
    auto& re = re_cache.get_RE(pattern, options);
    return re.capture(*this, offset, options);
}
/*-------------------------------------------------------------------------------*/


int DString::capture(DStringView pattern,
                     size_t offset,
                     std::vector<DString>& vec,
                     int options) const
{
    const auto& re = re_cache.get_RE(pattern, options);
    return re.capture(*this, offset, vec, options);
}
/*-------------------------------------------------------------------------------*/

int DString::match(DStringView pattern, size_t offset, DString::Match& m, int opts) const
{
    const auto& re = re_cache.get_RE(pattern, opts);
    return re.match(*this, offset, m, opts);
}
/*-------------------------------------------------------------------------------*/

int DString::match_groups(DStringView pattern, size_t offset,
                          DString::MatchVector& matches,
                          int options) const
{
    const auto& re = re_cache.get_RE(pattern, options);
    return re.match_groups(*this, offset, matches, options);
}
/*-------------------------------------------------------------------------------*/

int DString::subst_inplace(DStringView pattern, size_t offset,
                           DStringView replacement,
                           int options)
{
    const auto& re = re_cache.get_RE(pattern, options);
    return re.subst(*this, offset, replacement, options);
}
/*-------------------------------------------------------------------------------*/
