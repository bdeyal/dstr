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


class MatchData
{
public:
    MatchData(pcre2_code* code)
        :
        _match( pcre2_match_data_create_from_pattern(code, nullptr) )
   {
       if (!_match)
           throw DStringError("cannot create match data");
   }

    ~MatchData()
    {
        if (_match)
            pcre2_match_data_free(_match);
    }

    uint32_t count() const
    {
        return pcre2_get_ovector_count(_match);
    }

    const PCRE2_SIZE* data() const
    {
        return pcre2_get_ovector_pointer(_match);
    }

    operator pcre2_match_data*()
    {
        return _match;
    }

private:
    pcre2_match_data* _match;
};


DStringRegex::DStringRegex(const DString& pattern, int options)
    :
    _pcre(nullptr)
{
    int errorCode;
    PCRE2_SIZE errorOffset;
    unsigned nameCount;
    unsigned nameEntrySize;
    unsigned char* nameTable;

    pcre2_compile_context* context = pcre2_compile_context_create(nullptr);
    if (!context) throw DStringError("cannot create compile context");

    if (options & RE_NEWLINE_LF)
        pcre2_set_newline(context, PCRE2_NEWLINE_LF);
    else if (options & RE_NEWLINE_CRLF)
        pcre2_set_newline(context, PCRE2_NEWLINE_CRLF);
    else if (options & RE_NEWLINE_ANY)
        pcre2_set_newline(context, PCRE2_NEWLINE_ANY);
    else if (options & RE_NEWLINE_ANYCRLF)
        pcre2_set_newline(context, PCRE2_NEWLINE_ANYCRLF);
    else // default RE_NEWLINE_CR
        pcre2_set_newline(context, PCRE2_NEWLINE_CR);

    _pcre = pcre2_compile((PCRE2_SPTR)pattern.c_str(),
                          pattern.length(),
                          compileOptions(options),
                          &errorCode,
                          &errorOffset,
                          context);

    pcre2_compile_context_free(context);

    if (!_pcre)
    {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errorCode, buffer, sizeof(buffer));
        DString msg;
        msg.sprintf("%s (at offset %zu)", (char*)buffer, (size_t)(errorOffset));
        throw DStringError(msg);
    }

    pcre2_pattern_info((pcre2_code*)_pcre, PCRE2_INFO_NAMECOUNT, &nameCount);
    pcre2_pattern_info((pcre2_code*)_pcre, PCRE2_INFO_NAMEENTRYSIZE, &nameEntrySize);
    pcre2_pattern_info((pcre2_code*)_pcre, PCRE2_INFO_NAMETABLE, &nameTable);

    for (uint32_t i = 0; i < nameCount; i++)
    {
        unsigned char* group = nameTable + 2 + (nameEntrySize * i);
        int n = pcre2_substring_number_from_name((pcre2_code*)_pcre, group);
        _groups[n] = DString((char*)group);
    }
}


DStringRegex::~DStringRegex()
{
    if (_pcre)
        pcre2_code_free((pcre2_code*)_pcre);
}


int DStringRegex::match(const DString& subject, size_t offset, Match& mtch, int options) const
{
    assert (offset <= subject.length());

    MatchData matchData((pcre2_code*)_pcre);

    int rc = pcre2_match((pcre2_code*)_pcre,
                         (PCRE2_SPTR)(subject.c_str()),
                         subject.size(),
                         offset,
                         matchOptions(options),
                         matchData, nullptr);

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

    const PCRE2_SIZE* ovec = matchData.data();
    mtch.offset = ovec[0] < 0 ? DString::NPOS : ovec[0];
    mtch.length = ovec[1] - mtch.offset;
    return rc;
}


int DStringRegex::match(const DString& subject, size_t offset, MatchVector& matches, int options) const
{
    assert (offset <= subject.length());

    MatchVector mvec;

    MatchData matchData((pcre2_code*)_pcre);

    int rc = pcre2_match((pcre2_code*)_pcre,
                         (PCRE2_SPTR)(subject.c_str()),
                         subject.size(),
                         offset,
                         options & 0xFFFF,
                         matchData,
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
    matches.reserve(rc);
    const PCRE2_SIZE* ovec = matchData.data();
    for (int i = 0; i < rc; ++i)
    {
        Match m;
        GroupDict::const_iterator it;

        m.offset = ovec[i*2] < 0 ? DString::NPOS : ovec[i*2] ;
        m.length = ovec[i*2 + 1] - m.offset;

        it = _groups.find(i);
        if (it != _groups.end())
        {
            m.name = (*it).second;
        }

        mvec.push_back(m);
    }

    mvec.swap(matches);
    return rc;
}


bool DStringRegex::match(const DString& subject, size_t offset) const
{
    Match mtch;
    match(subject, offset, mtch, RE_ANCHORED | RE_NOTEMPTY);
    return mtch.offset == offset && mtch.length == subject.length() - offset;
}


bool DStringRegex::match(const DString& subject, size_t offset, int options) const
{
    Match mtch;
    match(subject, offset, mtch, options);
    return mtch.offset == offset && mtch.length == subject.length() - offset;
}


int DStringRegex::extract(const DString& subject, DString& str, int options) const
{
    Match mtch;
    int rc = match(subject, 0, mtch, options);
    if (mtch.offset != DString::NPOS)
        str.assign(subject, mtch.offset, mtch.length);
    else
        str.clear();
    return rc;
}


int DStringRegex::extract(const DString& subject, size_t offset, DString& str, int options) const
{
    Match mtch;
    int rc = match(subject, offset, mtch, options);
    if (mtch.offset != DString::NPOS)
        str.assign(subject, mtch.offset, mtch.length);
    else
        str.clear();
    return rc;
}


int DStringRegex::split(const DString& subject, size_t offset, std::vector<DString>& strings, int options) const
{
    MatchVector matches;
    std::vector<DString> tmp;

    int rc = match(subject, offset, matches, options);

    for (const auto& m: matches)
    {
        if (m.offset != DString::NPOS)
            tmp.push_back(subject.substr(m.offset, m.length));
        else
            tmp.push_back(DString());
    }

    strings.swap(tmp);
    return rc;
}


int DStringRegex::subst(DString& subject, size_t offset, const DString& replacement, int options) const
{
    if (options & RE_GLOBAL)
    {
        int rc = 0;
        size_t pos = substOne(subject, offset, replacement, options);
        while (pos != DString::NPOS)
        {
            ++rc;
            pos = substOne(subject, pos, replacement, options);
        }
        return rc;
    }
    else
    {
        return substOne(subject, offset, replacement, options) != DString::NPOS ? 1 : 0;
    }
}


size_t DStringRegex::substOne(DString& subject, size_t offset, const DString& replacement, int options) const
{
    if (offset >= subject.length()) return DString::NPOS;

    MatchData matchData((pcre2_code*)_pcre);

    int rc = pcre2_match((pcre2_code*)_pcre,
                         (PCRE2_SPTR)(subject.c_str()),
                         subject.size(),
                         offset,
                         matchOptions(options),
                         matchData,
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

    const PCRE2_SIZE* ovec = matchData.data();
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
                if (*it == '$' && !(options & RE_NO_VARS))
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


bool DStringRegex::match(const DString& subject, const DString& pattern, int options)
{
    int ctorOptions = options &
        (RE_CASELESS |
         RE_MULTILINE |
         RE_DOTALL |
         RE_EXTENDED |
         RE_ANCHORED |
         RE_DOLLAR_ENDONLY |
         RE_EXTRA |
         RE_UNGREEDY |
         RE_UTF8 |
         RE_NO_AUTO_CAPTURE);

    int mtchOptions = options &
        (RE_ANCHORED |
         RE_NOTBOL |
         RE_NOTEOL |
         RE_NOTEMPTY |
         RE_NO_AUTO_CAPTURE |
         RE_NO_UTF8_CHECK);

    DStringRegex re(pattern, ctorOptions);
    return re.match(subject, 0, mtchOptions);
}


int DStringRegex::compileOptions(int options)
{
    int pcreOptions = 0;

    if (options & RE_CASELESS)
        pcreOptions |= PCRE2_CASELESS;
    if (options & RE_MULTILINE)
        pcreOptions |= PCRE2_MULTILINE;
    if (options & RE_DOTALL)
        pcreOptions |= PCRE2_DOTALL;
    if (options & RE_EXTENDED)
        pcreOptions |= PCRE2_EXTENDED;
    if (options & RE_ANCHORED)
        pcreOptions |= PCRE2_ANCHORED;
    if (options & RE_DOLLAR_ENDONLY)
        pcreOptions |= PCRE2_DOLLAR_ENDONLY;
    if (options & RE_UNGREEDY)
        pcreOptions |= PCRE2_UNGREEDY;
    if (options & RE_UTF8)
        pcreOptions |= PCRE2_UTF | PCRE2_UCP;
    if (options & RE_NO_AUTO_CAPTURE)
        pcreOptions |= PCRE2_NO_AUTO_CAPTURE;
    if (options & RE_FIRSTLINE)
        pcreOptions |= PCRE2_FIRSTLINE;
    if (options & RE_DUPNAMES)
        pcreOptions |= PCRE2_DUPNAMES;

    return pcreOptions;
}


int DStringRegex::matchOptions(int options)
{
    int pcreOptions = 0;

    if (options & RE_ANCHORED)
        pcreOptions |= PCRE2_ANCHORED;
    if (options & RE_NOTBOL)
        pcreOptions |= PCRE2_NOTBOL;
    if (options & RE_NOTEOL)
        pcreOptions |= PCRE2_NOTEOL;
    if (options & RE_NOTEMPTY)
        pcreOptions |= PCRE2_NOTEMPTY;
    if (options & RE_NO_AUTO_CAPTURE)
        pcreOptions |= PCRE2_NO_AUTO_CAPTURE;
    if (options & RE_NO_UTF8_CHECK)
        pcreOptions |= PCRE2_NO_UTF_CHECK;

    return pcreOptions;
}
