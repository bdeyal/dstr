/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#if !defined(DSTRING_VIEW_HPP_INCLUDED)
#define DSTRING_VIEW_HPP_INCLUDED

#if !defined(DSTRING_HPP_INCLUDED)
#error "dstring_view.hpp cannot be included directly. Use dstr/dstring.hpp"
#endif

// Forward declaratons
//
class    DString;
struct   DStringMatch;
typedef  std::vector<DStringMatch> DSMatchVector;
//-----------------------------------------------

class DStringView {
public:
    friend class DString;

    // Types, constants and typedefs
    //
    static const size_t NPOS = DSTR_NPOS;

    // typedefs C++ algorithms support
    //
    typedef char value_type;
    typedef char& reference;
    typedef const char& const_reference;
    typedef char* iterator;
    typedef const char* const_iterator;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;

public:
    DStringView()
    {
        init_data("", 0);
    }

    ~DStringView()
    {
    }

    // DStringView s("A C string");
    //
    DStringView(const char* sz)
    {
        if (!sz)
            init_data("", 0);
        else
            init_data(sz, strlen(sz));
    }

    // DStringView s(other_DString);
    //
    DStringView(const DStringView& rhs)
    {
        init_data(rhs.data(), rhs.length());
    }

    // DStringView s("abcdefg", 3) -> "abc"
    //
    DStringView(const char* buffer, size_t len)
    {
        if (!buffer || !len) {
            init_data("", 0); }
        else {
            len = strnlen(buffer, len);
            init_data(buffer, len); }
    }

    DStringView& operator=(const char* sz)
    {
        if (!sz)
            init_data("", 0);
        else
            init_data(sz, strlen(sz));

        return *this;
    }

    void swap(DStringView& rhs)
    {
        DSTR_VIEW tmp = m_imp;
        m_imp = rhs.m_imp;
        rhs.m_imp = tmp;
    }

    // We (currently) return a copied object since we require DStringView
    // to be null terminated without null bytes in range
    //
    DString substr(size_t pos, size_t count = NPOS) const;

    size_t copy_substr(size_t pos, size_t numbytes, char dest[], size_t destsize)
    {
        return dstr_substr(pImp(), pos, numbytes, dest, destsize);
    }

    // Following function must be defined out of line either in .cpp file or
    // in a header file but only after class DString is fully known.
    //
    DString align_center(size_t width, char fill = ' ') const;
    DString align_right(size_t width, char fill = ' ') const;
    DString align_left(size_t width, char fill = ' ') const;
    DString zfill(size_t width) const;
    DString left(size_t count) const;
    DString mid(size_t pos, size_t count) const;
    DString right(size_t count) const;
    DString expandtabs(size_t width = 8) const;
    DString title() const;
    DString join(const std::vector<DString>& v) const;
    DString join(const char* argv[], size_t argc) const;
    DString times(size_t n) const;
    DString lstrip(char c) const;
    DString lstrip(const char* sz) const;
    DString rstrip(char c) const;
    DString rstrip(const char* sz) const;
    DString strip(char c) const;
    DString strip(const char* sz) const;
    DString upper() const;
    DString lower() const;
    DString swapcase() const;
    DString reverse() const;
    DString trim_right() const;
    DString trim_left() const;
    DString trim() const;

    // Inline queries
    //
    const char* c_str()    const noexcept { return m_imp.data;     }
    const char* data()     const noexcept { return m_imp.data;     }
    size_t      length()   const noexcept { return m_imp.length;   }
    bool        empty()    const noexcept { return length() == 0;  }

    bool index_ok(size_t pos) const
    {
        return dstr_valid_index(pImp(), pos);
    }

    char get(long pos) const
    {
        return dstr_getchar_safe(pImp(), pos);
    }

    char operator[](size_t pos) const
    {
        return m_imp.data[pos];
    }

    unsigned long hash(int seed = 0) const
    {
        return dstr_hash(pImp(), seed);
    }

    long atoi() const
    {
        return dstr_atoi(pImp());
    }

    double atof() const
    {
        return dstr_atof(pImp());
    }

    long atoll() const
    {
        return dstr_atoll(pImp());
    }

    size_t find(char c, size_t pos=0) const
    {
        return dstr_find_c(pImp(), pos, c);
    }

    size_t find(const char* sz, size_t pos=0) const
    {
        return dstr_find_sz(pImp(), pos, sz);
    }

    size_t find(DStringView sv, size_t pos=0) const
    {
        return dstr_find_sz(pImp(), pos, sv.data());
    }

    size_t ifind(char c, size_t pos=0) const
    {
        return dstr_ifind_c(pImp(), pos, c);
    }

    size_t ifind(const char* sz, size_t pos=0) const
    {
        return dstr_ifind_sz(pImp(), pos, sz);
    }

    size_t ifind(DStringView sv, size_t pos=0) const
    {
        return dstr_ifind_sz(pImp(), pos, sv.data());
    }

    size_t rfind(char c, size_t pos = NPOS) const
    {
        return dstr_rfind_c(pImp(), pos, c);
    }

    size_t rfind(const char* sz, size_t pos = NPOS) const
    {
        return dstr_rfind_sz(pImp(), pos, sz);
    }

    size_t rfind(DStringView sv, size_t pos=0) const
    {
        return dstr_rfind_sz(pImp(), pos, sv.data());
    }

    size_t irfind(char c, size_t pos = NPOS) const
    {
        return dstr_irfind_c(pImp(), pos, c);
    }

    size_t irfind(const char* sz, size_t pos = NPOS) const
    {
        return dstr_irfind_sz(pImp(), pos, sz);
    }

    size_t irfind(DStringView sv, size_t pos=0) const
    {
        return dstr_irfind_sz(pImp(), pos, sv.data());
    }

    size_t count(const char* sz) const
    {
        return dstr_count_sz(pImp(), sz);
    }

    size_t count(DStringView ds) const
    {
        return dstr_count_ds(pImp(), ds.pImp());
    }

    size_t icount(const char* sz) const
    {
        return dstr_icount_sz(pImp(), sz);
    }

    size_t icount(DStringView ds) const
    {
        return dstr_icount_ds(pImp(), ds.pImp());
    }

    bool contains(const char* s) const
    {
        return dstr_contains_sz(pImp(), s);
    }

    bool icontains(const char* s) const
    {
        return dstr_icontains_sz(pImp(), s);
    }

    bool isdigits()     const { return dstr_isdigits(pImp()); }
    bool isxdigits()    const { return dstr_isxdigits(pImp());  }
    bool isblank()      const { return dstr_isblank(pImp()); }
    bool isalnum()      const { return dstr_isalnum(pImp()); }
    bool isalpha()      const { return dstr_isalpha(pImp()); }
    bool is_ascii()     const { return dstr_isascii(pImp()); }
#if !defined(_WIN32)
    bool isascii()      const { return is_ascii();           }
#endif
    bool isdecimal()    const { return dstr_isdecimal(pImp()); }
    bool isidentifier() const { return dstr_isidentifier(pImp()); }
    bool islower()      const { return dstr_islower(pImp()); }
    bool isnumeric()    const { return dstr_isnumeric(pImp()); }
    bool isprintable()  const { return dstr_isprintable(pImp()); }
    bool isspace()      const { return dstr_isspace(pImp()); }
    bool istitle()      const { return dstr_istitle(pImp()); }
    bool isupper()      const { return dstr_isupper(pImp()); }

    bool startswith(const char* s) const
    {
        return dstr_prefix_sz(pImp(), s);
    }

    bool istartswith(const char* s) const
    {
        return dstr_iprefix_sz(pImp(), s);
    }

    bool endswith(const char* s) const
    {
        return dstr_suffix_sz(pImp(), s);
    }

    bool iendswith(const char* s) const
    {
        return dstr_isuffix_sz(pImp(), s);
    }

    // ffo = find_first of
    //
    size_t ffo(const char* s, size_t pos = 0) const
    {
        return dstr_ffo_sz(pImp(), pos, s);
    }

    size_t ffo(DStringView rhs, size_t pos = 0) const
    {
        return dstr_ffo_ds(pImp(), pos, rhs.pImp());
    }

    // ffno = find first not of
    //
    size_t ffno(const char* s, size_t pos = 0) const
    {
        return dstr_ffno_sz(pImp(), pos, s);
    }

    size_t ffno(DStringView rhs, size_t pos = 0) const
    {
        return dstr_ffno_ds(pImp(), pos, rhs.pImp());
    }

    // flo = find_last_of
    //
    size_t flo(const char* s, size_t pos = DSTR_NPOS) const
    {
        return dstr_flo_sz(pImp(), pos, s);
    }

    size_t flo(DStringView rhs, size_t pos = DSTR_NPOS) const
    {
        return dstr_flo_ds(pImp(), pos, rhs.pImp());
    }

    // flno = find_last_not_of
    //
    size_t flno(const char* s, size_t pos = DSTR_NPOS) const
    {
        return dstr_flno_sz(pImp(), pos, s);
    }

    size_t flno(DStringView rhs, size_t pos = DSTR_NPOS) const
    {
        return dstr_flno_ds(pImp(), pos, rhs.pImp());
    }

    // Comparisons and operators ==, !=, >, <, >=, <=
    //
    // Note: NULL C string is considered empty string ""
    //
    int compare(const char* sz)   const { return strcmp(data(), sz ? sz : ""); }
    int icompare(const char* sz)  const { return strcasecmp(data(), sz ? sz : ""); }
    int compare(DStringView rhs)  const { return strcmp(data(), rhs.data()); }
    int icompare(DStringView rhs) const { return strcasecmp(data(), rhs.data()); }

    bool iequal(const char* sz)   const { return (icompare(sz) == 0);  }
    bool iequal(DStringView rhs)  const { return (icompare(rhs) == 0); }

    bool operator!=(const char* sz) const { return (compare(sz) != 0);  }
    bool operator<(const char* sz)  const { return (compare(sz) <  0);  }
    bool operator>(const char* sz)  const { return (compare(sz) >  0);  }
    bool operator<=(const char* sz) const { return (compare(sz) <= 0);  }
    bool operator>=(const char* sz) const { return (compare(sz) >= 0);  }

    bool operator<(DStringView rhs)  const { return (compare(rhs) < 0);  }
    bool operator>(DStringView rhs)  const { return (compare(rhs) > 0);  }
    bool operator<=(DStringView rhs) const { return (compare(rhs) <= 0); }
    bool operator>=(DStringView rhs) const { return (compare(rhs) >= 0); }

    bool operator==(const char* sz) const
    {
        return (compare(sz) == 0);
    }

    bool operator==(DStringView rhs) const
    {
        return (size() == rhs.size()) && (compare(rhs) == 0);
    }

    bool operator!=(DStringView rhs) const
    {
        return (size() != rhs.size()) || (compare(rhs) != 0);
    }

    int stoi(size_t* index = nullptr, int base = 10) const
    {
        return dstr_to_int(pImp(), index, base);
    }

    long stol(size_t* index = nullptr, int base = 10) const
    {
        return dstr_to_long(pImp(), index, base);
    }

    unsigned long stoul(size_t* index = nullptr, int base = 10) const
    {
        return dstr_to_ulong(pImp(), index, base);
    }

    long long stoll(size_t* index = nullptr, int base = 10) const
    {
        return dstr_to_llong(pImp(), index, base);
    }

    unsigned long long stoull(size_t* index = nullptr, int base = 10) const
    {
        return dstr_to_ullong(pImp(), index, base);
    }

    float stof(size_t* index = nullptr) const
    {
        return dstr_to_float(pImp(), index);
    }

    double stod(size_t* index = nullptr) const
    {
        return dstr_to_double(pImp(), index);
    }

    long double stold(size_t* index = nullptr) const
    {
        return dstr_to_ldouble(pImp(), index);
    }

    // C++ algorithms support : functions
    //
    size_type      size()   const { return length(); }
    const_iterator begin()  const { return m_imp.data; }
    const_iterator cbegin() const { return begin(); }
    const_iterator end()    const { return &m_imp.data[m_imp.length]; }
    const_iterator cend()   const { return end(); }

    void remove_prefix(size_t n)
    {
        if (!n) return;
        if (n > size()) n = size();
        m_imp.data += n;
        m_imp.length -= n;
    }

    /////////////////////////////////////////////////////////////////
    //
    //  Split, tokenize, partition
    //
    /////////////////////////////////////////////////////////////////

    // split: empty string between separator character or string
    //
    void split(char c, std::vector<DString>& dest) const;
    void split(const char* sep, std::vector<DString>& dest) const;

    void split(DStringView sep, std::vector<DString>& dest)  const
    {
        split(sep.data(), dest);
    }

    void splitlines(std::vector<DString>& dest) const
    {
        split('\n', dest);
    }

    void tokenize(const char* separators, std::vector<DString>& dest) const;
    void tokenize(DStringView separators, std::vector<DString>& dest) const
    {
        tokenize(separators.data(), dest);
    }

    // split() without any separator will split on any whitespace
    // character (including \n \r \t \f and spaces) and will discard
    // empty strings from the result
    //
    void split(std::vector<DString>& dest) const
    {
        tokenize("\n\r\t\f ", dest);
    }

    void partition(const char* s,
                   DString& left, DString& middle, DString& right) const;

    void rpartition(const char* s,
                    DString& left, DString& middle, DString& right) const;

    void partition(DStringView s, DString& l, DString& m, DString& r) const
    {
        partition(s.c_str(), l, m, r);
    }

    void rpartition(DStringView s, DString& l, DString& m, DString& r) const
    {
        rpartition(s.c_str(), l, m, r);
    }

#if !defined(NO_DSTRING_REGEX)
    ///////////////////////////////////////////////////
    //
    //   Regexp Support
    //
    ///////////////////////////////////////////////////

    // *this is an exact match
    //
    bool match(DStringView pattern, size_t offset = 0) const;

    // pattern appears somewhere in *this. Returns position or NPOS
    //
    size_t match_within(DStringView pattern, size_t offset = 0) const;

    // store match info in Match parameter
    //
    int match(DStringView ptrn,
              size_t offset, DStringMatch& m,
              const char* opts = nullptr) const;

    int match(DStringView pattern, DStringMatch& mtch, const char* options = nullptr) const
    {
        return match(pattern, 0, mtch, options);
    }

    // match groups into a vector of matches
    //
    int match_groups(DStringView pattern, size_t offset,
                     DSMatchVector& matches,
                     const char* options = nullptr) const;

    int match_groups(DStringView pattern,
                     DSMatchVector& matches,
                     const char* options = nullptr) const
    {
        return match_groups(pattern, 0, matches, options);
    }

    // capture - returns matches as strings or vector of strings
    //
    int capture(DStringView pattern,
                size_t offset,
                DString& result,
                const char* options = nullptr) const;

    int capture(DStringView pattern,
                DString& result,
                const char* options = nullptr) const
    {
        return capture(pattern, 0, result, options);
    }

    int capture(DStringView pattern,
                size_t offset,
                std::vector<DString>& strings,
                const char* options = nullptr) const;

    int capture(DStringView pattern,
                std::vector<DString>& strings,
                const char* options = nullptr) const
    {
        return capture(pattern, 0, strings, options);
    }
#endif // NO_DSTRING_REGEX


private:
    // Single data member.
    //
    DSTR_VIEW m_imp;

private:
    CDSTR pImp() const { return (CDSTR)(&m_imp); }
    void init_data(const char* p, size_t count)
    {
        m_imp.data   = p;
        m_imp.length = count;
    }
};
//----------------------------------------------------------------

// Include guard
//
#endif
