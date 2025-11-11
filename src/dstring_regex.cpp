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
#include <stdexcept>
#include <cstdint>
#include <cassert>
#include <unordered_map>
#include <vector>
#include <list>

#include <dstr/dstring.hpp>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

namespace {

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

// Encapsulates a PCRE2 regular expression object
//
class DStringRegex
{
public:
    DStringRegex(const DString& pattern, int options = 0);
    ~DStringRegex();

    // Implemented below
    //
    bool match(const DString& subject, size_t offset = 0,
               int options = DSTR_REGEX_ANCHORED | DSTR_REGEX_NOTEMPTY) const;
    int match(const DString& s, size_t off, DString::Match& m, int opts = 0) const;
    int match_groups(const DString& s, size_t off, DString::MatchVector& m,  int opts = 0) const;
    DString capture(const DString& subject, size_t offset, int options = 0) const;
    int capture(const DString& subject, size_t offset, std::vector<DString>& strings, int options = 0) const;
    int subst(DString& subject, size_t offset, const DString& replacement, int options = 0) const;

    // Inlines using the above
    //
    int match(const DString& s, DString::Match& m, int opts = 0) const {
        return match(s, 0, m, opts);
    }
    int match_groups(const DString& s, DString::MatchVector& m, int opts = 0) const {
        return match_groups(s, 0, m, opts);
    }
    DString capture(const DString& subject, int options = 0) const {
        return capture(subject, 0, options);
    }
    int capture(const DString& subject, std::vector<DString>& strings, int options = 0) const {
        return capture(subject, 0, strings, options);
    }
    int subst(DString& s, const DString& r, int options = 0) const {
        return subst(s, 0, r, options);
    }

private:
    // Actual type is pcre2_code_8*
    pcre2_code* _pRE;

    typedef std::unordered_map<int, DString> GroupDict;
    GroupDict m_groups;

private:
    size_t subst_single(DString& subject, size_t offset, const DString& repl, int options) const;

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

class DStringError : public std::exception
{
public:
    DStringError(DString&& s) : m_s(s) {}
    DStringError(const char* s) : m_s(s) {}
    const char* what() const noexcept { return m_s.c_str(); }
private:
    DString m_s;
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
    // uint32_t count()         const { return pcre2_get_ovector_count(_match);   }
    const PCRE2_SIZE* data() const { return pcre2_get_ovector_pointer(_match); }
    operator pcre2_match_data*()   { return _match; }

    PCRE2_SIZE operator[](size_t index) const {
        if (!ovec) ovec = data();
        return ovec[index];
    }

private:
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

DStringRegex::DStringRegex(const DString& pattern, int options)
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

    unsigned int name_count;
    pcre2_pattern_info(_pRE, PCRE2_INFO_NAMECOUNT, &name_count);

    unsigned int name_entry_size;
    pcre2_pattern_info(_pRE, PCRE2_INFO_NAMEENTRYSIZE, &name_entry_size);

    unsigned char* name_table;
    pcre2_pattern_info(_pRE, PCRE2_INFO_NAMETABLE, &name_table);

    for (uint32_t i = 0; i < name_count; i++)
    {
        unsigned char* group = name_table + 2 + (name_entry_size * i);
        int n = pcre2_substring_number_from_name(_pRE, group);
        m_groups[n] = DString((char*)group);
    }
}
/*-------------------------------------------------------------------------------*/

DStringRegex::~DStringRegex()
{
    if (_pRE)
        pcre2_code_free(_pRE);
}
/*-------------------------------------------------------------------------------*/

int DStringRegex::match(const DString& subject, size_t offset,
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
                         mdata, nullptr);

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

bool DStringRegex::match(const DString& subject, size_t offset, int options) const
{
    DString::Match mtch;
    match(subject, offset, mtch, options);
    // printf("Match offset: %zu, len: %zu\n",  mtch.offset,  mtch.length);
    return
        mtch.offset == offset &&
        mtch.length == subject.length() - offset;
}
/*-------------------------------------------------------------------------------*/

int DStringRegex::match_groups(const DString& subject, size_t offset,
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
                         // match_options(options) & 0xFFFF,
                         options & 0xFFFF,
                         mdata,
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
        GroupDict::const_iterator it = m_groups.find(i);
        if (it != m_groups.end()) {
            strncpy(m.name, it->second.c_str(), sizeof(m.name) - 1);
        }
        else {
            m.name[0] = '\0';
        }

        mvec.push_back(m);
    }

    mvec.swap(matches);
    return rc;

    // Error
    //
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

DString DStringRegex::capture(const DString& subject, size_t offset,
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

int DStringRegex::capture(const DString& subject, size_t offset,
                          std::vector<DString>& strings,
                          int options) const
{
    DString::MatchVector matches;
    std::vector<DString> tmp;

    int rc = match_groups(subject, offset, matches, options);

    for (const auto& m: matches)
    {
        if (m.offset != DString::NPOS)
            tmp.push_back(subject.substr(m.offset, m.length));
        else
            tmp.push_back("");
    }

    strings.swap(tmp);
    return rc;
}
/*-------------------------------------------------------------------------------*/

int DStringRegex::subst(DString& subject, size_t offset,
                        const DString& replacement,
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
    else
    {
        size_t n = subst_single(subject, offset, replacement, options);
        if (n == DString::NPOS)
            return 0;

        return 1;
    }
}
/*-------------------------------------------------------------------------------*/

size_t DStringRegex::subst_single(DString& subject, size_t offset,
                                  const DString& replacement,
                                  int options) const
{
    if (offset >= subject.length()) return DString::NPOS;

    MatchData mdata(_pRE);

    int rc = pcre2_match(_pRE,
                         (PCRE2_SPTR)(subject.c_str()),
                         subject.size(),
                         offset,
                         match_options(options),
                         mdata,
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
                                DString tmp = subject.substr(o, l);
                                result.append(tmp);
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

struct RegexKey {
    RegexKey(const DString& rhs, int opts)
        :
        pattern(rhs),
        hash_value(rhs.hash(opts))
        {
        }

    RegexKey(const RegexKey& rhs)
        :
        pattern(rhs.pattern),
        hash_value(rhs.hash_value)
        {
        }

    bool operator==(const RegexKey& rhs) const {
        return pattern == rhs.pattern && hash_value == rhs.hash_value;
    }

    DString pattern;
    size_t hash_value;
};
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

struct RegexHash {
    size_t operator()(const RegexKey& key) const {
        return key.hash_value;
    }
};
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

template <size_t LIMIT>
struct DStringRegexCache
{
    // Data member
    //
    std::unordered_map<RegexKey, std::unique_ptr<DStringRegex>, RegexHash> the_map;
    std::list<RegexKey> fifo;

    const DStringRegex& get_RE(const DString& pattern, int options)
    {
        RegexKey key(pattern, options);
        auto iter = the_map.find(key);
        if (iter == the_map.end()) {
            fifo.push_back(key);
            auto upRE = make_unique<DStringRegex>(pattern, options);
            iter = the_map.emplace(key, std::move(upRE)).first;
            remove_oldest_if_limit();
        }
        return *iter->second;
    }

    void remove_oldest_if_limit()
    {
        assert(fifo.size() == the_map.size());
        while (fifo.size() > LIMIT) {
            const auto& oldest = fifo.front();
            the_map.erase(oldest);
            fifo.pop_front();
        }
    }
};
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

static thread_local DStringRegexCache<30> re_cache;

}  // unnamed ns
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

//
//  DSTRING REGEX API
//
bool DString::match(const DString& pattern, size_t offset) const
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

size_t DString::match_within(const DString& pattern, size_t offset) const
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

DString DString::capture(const DString& pattern,
                         size_t offset,
                         int options) const
{
    auto& re = re_cache.get_RE(pattern, options);
    return re.capture(*this, offset, options);
}
/*-------------------------------------------------------------------------------*/


int DString::capture(const DString& pattern,
                     size_t offset,
                     std::vector<DString>& vec,
                     int options) const
{
    const auto& re = re_cache.get_RE(pattern, options);
    return re.capture(*this, offset, vec, options);
}
/*-------------------------------------------------------------------------------*/

int DString::match(const DString& pattern, size_t offset, DString::Match& m, int opts) const
{
    const auto& re = re_cache.get_RE(pattern, opts);
    return re.match(*this, offset, m, opts);
}
/*-------------------------------------------------------------------------------*/

int DString::match_groups(const DString& pattern, size_t offset,
                          DString::MatchVector& matches,
                          int options) const
{
    const auto& re = re_cache.get_RE(pattern, options);
    return re.match_groups(*this, offset, matches, options);
}
/*-------------------------------------------------------------------------------*/

int DString::subst_inplace(const DString& pattern, size_t offset,
                           const DString& replacement,
                           int options)
{
    const auto& re = re_cache.get_RE(pattern, options);
    return re.subst(*this, offset, replacement, options);
}
/*-------------------------------------------------------------------------------*/
