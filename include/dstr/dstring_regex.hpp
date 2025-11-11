#ifndef DSTRING_REGEXP_INCLUDED
#define DSTRING_REGEXP_INCLUDED

#include <vector>
#include <unordered_map>
#include <dstr/dstring.hpp>

typedef std::vector<RE_Match> RE_MatchVector;

class DStringRegex
{
public:

    DStringRegex(const DString& pattern, int options = 0);
    ~DStringRegex();

    // match
    //
    int match(const DString& subject, size_t offset,
              RE_Match& m,
              int options = 0) const;

    int match(const DString& subject, RE_Match& mtch, int options = 0) const
    {
        return match(subject, 0, mtch, options);
    }

    bool match(const DString& subject,
               size_t offset = 0,
               int options = DSTR_REGEX_ANCHORED | DSTR_REGEX_NOTEMPTY) const;

    // match groups into a vector of matches
    //
    int match_groups(const DString& subject, size_t offset,
                     RE_MatchVector& matches,
                     int options = 0) const;

    int match_groups(const DString& subject,
                     RE_MatchVector& matches,
                     int options = 0) const
    {
        return match_groups(subject, 0, matches, options);
    }
    //------------------------------------------------------------------------

    // capture - returns matches as strings or vector of strings
    //
    DString capture(const DString& subject, size_t offset,
                    int options = 0) const;
    DString capture(const DString& subject, int options = 0) const
    {
        return capture(subject, 0, options);
    }

    int capture(const DString& subject, size_t offset,
                std::vector<DString>& strings,
                int options = 0) const;

    int capture(const DString& subject,
                std::vector<DString>& strings,
                int options = 0) const
    {
        return capture(subject, 0, strings, options);
    }
    //------------------------------------------------------------------------

    // substitute
    //
    int subst(DString& subject, size_t offset,
              const DString& replacement,
              int options = 0) const;

    int subst(DString& s, const DString& r, int options = 0) const
    {
        return subst(s, 0, r, options);
    }
    //------------------------------------------------------------------------

private:
      // Actual type is pcre2_code_8*
    void* _pcre;

    typedef std::unordered_map<int, DString> GroupDict;
    GroupDict m_groups;

private:
    size_t subst_single(DString& subject, size_t offset,
                        const DString& replacement,
                        int options) const;

    // Prevent default, copy and assignment
    //
    DStringRegex() = delete;
    DStringRegex(const DStringRegex&) = delete;
    DStringRegex& operator=(const DStringRegex&) = delete;
    DStringRegex(DStringRegex&&) = delete;
    DStringRegex& operator=(DStringRegex&&) = delete;
};


#endif
