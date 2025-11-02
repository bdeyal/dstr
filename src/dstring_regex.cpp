#include <stdexcept>
#include <cstdint>
#include <cassert>

#include <dstr/dstring_regex.hpp>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

class DStringError : public std::exception
{
public:
    DStringError(const DString& s) : m_s(s) {}
    DStringError(const char* s) : m_s(s) {}
    const char* what() const noexcept { return m_s.c_str(); }
private:
    DString m_s;
};
/*-------------------------------------------------------------------------------*/

// RAII wrapper around pcre2_match_data*
//
namespace {
struct MatchData {
    MatchData(pcre2_code* code)
        : _match(pcre2_match_data_create_from_pattern(code, nullptr))
    {
       if (!_match) throw DStringError("cannot create match data");
    }
    ~MatchData()  {
        if (_match) pcre2_match_data_free(_match);
    }
    uint32_t count()         const { return pcre2_get_ovector_count(_match);   }
    const PCRE2_SIZE* data() const { return pcre2_get_ovector_pointer(_match); }
    operator pcre2_match_data*()   { return _match; }
    pcre2_match_data* _match;  // actual pointer
};
} // unnamed NS
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

// Syntactic sugar to make code more readable
//
#define _pRE ((pcre2_code*)(this->_pcre))

DStringRegex::DStringRegex(const DString& pattern, int options)
    :
    _pcre(nullptr)
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
    this->_pcre = pcre2_compile((PCRE2_SPTR)pattern.c_str(),
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
        msg.sprintf("%s (at offset %zu)", (char*)buffer, error_offset);
        throw DStringError(msg);
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
        this->_groups[n] = DString((char*)group);
    }
}
/*-------------------------------------------------------------------------------*/

DStringRegex::~DStringRegex()
{
    if (_pcre)
        pcre2_code_free(_pRE);
}
/*-------------------------------------------------------------------------------*/

int DStringRegex::match(const DString& subject, size_t offset,
                        RE_Match& mtch,
                        int options) const
{
    assert (offset <= subject.length());

    MatchData mdata(_pRE);

    int rc = pcre2_match(_pRE,
                         (PCRE2_SPTR)(subject.c_str()),
                         subject.size(),
                         offset,
                         match_options(options),
                         mdata, nullptr);

    if (rc == PCRE2_ERROR_NOMATCH)
    {
        mtch.offset = DString::NPOS;
        mtch.length = 0;
        return 0;
    }
    else if (rc == PCRE2_ERROR_BADOPTION)
    {
        throw DStringError("bad option");
    }
    else if (rc == 0)
    {
        throw DStringError("too many captured substrings");
    }
    else if (rc < 0)
    {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(rc, buffer, sizeof(buffer));
        throw DStringError((char*)buffer);
    }

    const PCRE2_SIZE* ovec = mdata.data();
    if (ovec[0] == PCRE2_UNSET) {
        mtch.offset = DString::NPOS;
        mtch.length = 0;
    }
    else {
        mtch.offset = ovec[0];
        mtch.length = ovec[1] - mtch.offset;
    }

    return rc;
}
/*-------------------------------------------------------------------------------*/

int DStringRegex::match_all(const DString& subject, size_t offset,
                            RE_MatchVector& matches,
                            int options) const
{
    assert (offset <= subject.length());

    RE_MatchVector mvec;

    MatchData mdata(_pRE);

    int rc = pcre2_match(_pRE,
                         (PCRE2_SPTR)(subject.c_str()),
                         subject.size(),
                         offset,
                         match_options(options) & 0xFFFF,
                         //options & 0xFFFF,
                         mdata,
                         nullptr);

    if (rc == PCRE2_ERROR_NOMATCH)
    {
        return 0;
    }
    else if (rc == PCRE2_ERROR_BADOPTION)
    {
        throw DStringError("bad option");
    }
    else if (rc == 0)
    {
        throw DStringError("too many captured substrings");
    }
    else if (rc < 0)
    {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(rc, buffer, sizeof(buffer));
        throw DStringError((char*)buffer);
    }

    mvec.reserve(rc);
    const PCRE2_SIZE* ovec = mdata.data();

    for (int i = 0; i < rc; ++i)
    {
        RE_Match m;

        if (ovec[2 * i] == PCRE2_UNSET) {
            m.offset = DString::NPOS;
            m.length = 0;
        }
        else {
            m.offset = ovec[2 * i];
            m.length = ovec[2 * i + 1] - m.offset;
        }

        GroupDict::const_iterator it = this->_groups.find(i);
        if (it != this->_groups.end())
        {
            m.name = it->second;
        }

        mvec.push_back(m);
    }

    mvec.swap(matches);
    return rc;
}
/*-------------------------------------------------------------------------------*/

bool DStringRegex::match(const DString& subject, size_t offset) const
{
    RE_Match mtch;
    match(subject, offset, mtch, DSTR_REGEX_ANCHORED | DSTR_REGEX_NOTEMPTY);
    return mtch.offset == offset && mtch.length == subject.length() - offset;
}
/*-------------------------------------------------------------------------------*/

bool DStringRegex::match(const DString& subject, size_t offset,
                         int options) const
{
    RE_Match mtch;
    match(subject, offset, mtch, options);
    return mtch.offset == offset && mtch.length == subject.length() - offset;
}
/*-------------------------------------------------------------------------------*/

int DStringRegex::extract(const DString& subject, size_t offset,
                          DString& str,
                          int options) const
{
    RE_Match mtch;
    int rc = match(subject, offset, mtch, options);
    if (mtch.offset != DString::NPOS)
        str.assign(subject, mtch.offset, mtch.length);
    else
        str.clear();
    return rc;
}
/*-------------------------------------------------------------------------------*/

int DStringRegex::split(const DString& subject, size_t offset,
                        std::vector<DString>& strings,
                        int options) const
{
    RE_MatchVector matches;
    std::vector<DString> tmp;

    int rc = match_all(subject, offset, matches, options);

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

    if (rc == PCRE2_ERROR_NOMATCH)
    {
        return DString::NPOS;
    }
    else if (rc == PCRE2_ERROR_BADOPTION)
    {
        throw DStringError("bad option");
    }
    else if (rc == 0)
    {
        throw DStringError("too many captured substrings");
    }
    else if (rc < 0)
    {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(rc, buffer, sizeof(buffer));
        throw DStringError((char*)buffer);
    }

    const PCRE2_SIZE* ovec = mdata.data();
    DString result;
    size_t len = subject.length();
    size_t pos = 0;
    size_t rp = DString::NPOS;
    while (pos < len)
    {
        if (ovec[0] == pos)
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
                                size_t o = ovec[c*2];
                                size_t l = ovec[c*2 + 1] - o;
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
            pos = ovec[1];
            rp = result.length();
        }
        else result += subject[pos++];
    }
    subject = result;
    return rp;
}
/*-------------------------------------------------------------------------------*/

bool dstring_match(const DString& subject, const DString& pattern, int options)
{
    int ctor_opts = options &
        (DSTR_REGEX_CASELESS |
         DSTR_REGEX_MULTILINE |
         DSTR_REGEX_DOTALL |
         DSTR_REGEX_EXTENDED |
         DSTR_REGEX_ANCHORED |
         DSTR_REGEX_DOLLAR_ENDONLY |
         DSTR_REGEX_EXTRA |
         DSTR_REGEX_UNGREEDY |
         DSTR_REGEX_UTF8 |
         DSTR_REGEX_NO_AUTO_CAPTURE);

    int match_opts = options &
        (DSTR_REGEX_ANCHORED |
         DSTR_REGEX_NOTBOL |
         DSTR_REGEX_NOTEOL |
         DSTR_REGEX_NOTEMPTY |
         DSTR_REGEX_NO_AUTO_CAPTURE |
         DSTR_REGEX_NO_UTF8_CHECK);

    DStringRegex re(pattern, ctor_opts);
    return re.match(subject, 0, match_opts);
}
/*-------------------------------------------------------------------------------*/
