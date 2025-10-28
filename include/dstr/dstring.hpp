#ifndef DSTRING_H_INCLUDED
#define DSTRING_H_INCLUDED

#include <iosfwd>
#include <dstr/dstr.h>

// A thin wrapper around C DSTR_TYPE
//
class DString {
public:
    static const size_t NPOS = DSTR_NPOS;

    DString()
    {
        dstr_init_data(pImp());
    }

    ~DString()
    {
        if (capacity() > DSTR_INITIAL_CAPACITY)
            dstr_clean_data(pImp());
    }

    // DString s('A', 100);
    //
    DString(char c, size_t count)
    {
        if (c == '\0' || count == 0) {
            dstr_init_data(pImp());
            return;
        }
        init_capacity(count);
        init_data(c, count);
        init_length(count);
    }

    // DString s("A C string");
    //
    DString(const char* sz)
    {
        size_t len = sz ? strlen(sz) : 0;

        if (!len) {
            dstr_init_data(pImp());
            return;
        }
        init_capacity(len);
        init_data(sz, len);
        init_length(len);
    }

    // DString s(other_DString);
    //
    DString(const DString& rhs)
    {
        if (rhs.length() == 0) {
            dstr_init_data(pImp());
            return;
        }
        init_capacity(rhs.length());
        init_data(rhs.data(), rhs.length());
        init_length(rhs.length());
    }

#if __cplusplus >= 201103L
    DString(DString&& rhs)
    {
        dstr_init_data(pImp());
        swap(rhs);
    }
#endif

    // DString sub_string(other_dstr, 5, 5);
    //
    DString(const DString& rhs, size_t pos, size_t count)
    {
        if (pos >= rhs.size() || count == 0) {
            dstr_init_data(pImp());
            return;
        }

        if (count > rhs.size() - pos)
            count = rhs.size() - pos;

        init_capacity(count);
        init_data(rhs.data() + pos, count);
        init_length(count);
    }

    // DString s("abcdefg", 3) -> "abc"
    //
    DString(const char* buffer, size_t len)
    {
        if (!buffer || (len = strnlen(buffer, len)) == 0) {
            dstr_init_data(pImp());
            return;
        }
        init_capacity(len);
        init_data(buffer, len);
        init_length(len);
    }

    // DString s(&str[5], &str[15]);
    //
    DString(const char* first, const char* last)
    {
        size_t len = last - first;

        if ((len = strnlen(first, len)) == 0) {
            dstr_init_data(pImp());
            return;
        }

        init_capacity(len);
        init_data(first, len);
        init_length(len);
    }

    // substr returns a new string constructed by the 'substr constructor'
    //
    DString substr(size_t pos, size_t count = NPOS) const
    {
        return DString(*this, pos, count);
    }

    // Like in basic
    //
    DString left(size_t count) const
    {
        return substr(0, count);
    }

    DString mid(size_t pos, size_t count) const
    {
        return substr(pos, count);
    }

    DString right(size_t count) const
    {
        return (size() <= count) ?
            DString(*this) :
            substr(size() - count, count);
    }

    // slurp a text file into a DString
    //
    int from_file(const char* fname)
    {
        if (dstr_assign_fromfile(pImp(), fname))
            return DSTR_SUCCESS;

        return DSTR_FAIL;
    }

    int from_cfile(FILE* fp)
    {
        if (dstr_slurp_stream(pImp(), fp))
            return DSTR_SUCCESS;

        return DSTR_FAIL;
    }

#if __cplusplus >= 201103L
    // Assignments operator and functions
    //
    DString& operator=(DString&& rhs) noexcept
    {
        if (&rhs != this)
            swap(rhs);
        return *this;
    }
#endif

    DString& operator=(const DString& rhs)
    {
        if (&rhs != this)
            assign(rhs);
        return *this;
    }

    DString& operator=(const char* sz)
    {
        return assign(sz);
    }

    DString& operator=(char c)
    {
        return assign(c);
    }

    DString& assign(char c)
    {
        clear();
        return append(c);
    }

    DString& assign(char c, size_t count)
    {
        dstr_assign_cc(pImp(), c, count);
        return *this;
    }

    DString& assign(const char* sz)
    {
        dstr_assign_sz(pImp(), sz);
        return *this;
    }

    DString& assign(const char* buff, size_t len)
    {
        dstr_assign_bl(pImp(), buff, len);
        return *this;
    }

    DString& assign(const char* first, const char* last)
    {
        dstr_assign_range(pImp(), first, last);
        return *this;
    }

    DString& assign(const DString& rhs)
    {
        dstr_assign_ds(pImp(), rhs.pImp());
        return *this;
    }

#if __cplusplus >= 201103L
    DString& assign(DString&& rhs) noexcept
    {
        if (&rhs != this)
            swap(rhs);
        return *this;
    }
#endif

    DString& assign(const DString& rhs, size_t pos, size_t count)
    {
        dstr_assign_substr(pImp(), rhs.pImp(), pos, count);
        return *this;
    }

    DString& sprintf(const char* fmt, ...);

    DString& vsprintf(const char* fmt, va_list args) {
        dstr_assign_vsprintf(pImp(), fmt, args);
        return *this;
    }

    // Insertion functions
    //
    DString& insert(size_t pos, char c, size_t count)
    {
        dstr_insert_cc(pImp(), pos, c, count);
        return *this;
    }

    DString& insert(size_t pos, const char* value)
    {
        dstr_insert_sz(pImp(), pos, value);
        return *this;
    }

    DString& insert(size_t pos, const DString& rhs)
    {
        dstr_insert_ds(pImp(), pos, rhs.pImp());
        return *this;
    }

    DString& insert(size_t pos, const char* buff, size_t len)
    {
        dstr_insert_bl(pImp(), pos, buff, len);
        return *this;
    }

    DString& insert(size_t pos, const char* first, const char* last)
    {
        dstr_insert_range(pImp(), pos, first, last);
        return *this;
    }

    DString& align_center(size_t width, char fill = ' ')
    {
        if (length() < width)
            dstr_align_center(pImp(), width, fill);
        return *this;
    }

    DString& align_right(size_t width, char fill = ' ')
    {
        if (length() < width)
            dstr_align_right(pImp(), width, fill);
        return *this;
    }

    DString& align_left(size_t width, char fill = ' ')
    {
        if (length() < width)
            dstr_align_left(pImp(), width, fill);
        return *this;
    }

    DString make_center(size_t width, char fill = ' ') const
    {
        DString result(*this);
        result.align_center(width, fill);
        return result;
    }

    DString make_right(size_t width, char fill = ' ') const
    {
        DString result(*this);
        result.align_right(width, fill);
        return result;
    }

    DString make_left(size_t width, char fill = ' ') const
    {
        DString result(*this);
        result.align_left(width, fill);
        return result;
    }

    DString& expandtabs(size_t width = 8)
    {
        dstr_expand_tabs(pImp(), width);
        return *this;
    }

    DString make_expandtabs(size_t width = 8) const
    {
        DString result(*this);
        result.expandtabs(width);
        return result;
    }

    DString& title()
    {
        dstr_title(pImp());
        return *this;
    }

    DString make_title() const
    {
        DString result(*this);
        result.title();
        return result;
    }

    void push_back(char c)
    {
        dstr_append_inline(pImp(), c);
    }

    void push_front(char c)
    {
        insert(0, c, 1);
    }

    void pop_front()
    {
        erase(0);
    }

    void chop()
    {
        dstr_chop(pImp());
    }

    void pop_back()
    {
        chop();
    }

    DString& append(char c)
    {
        push_back(c);
        return *this;
    }

    DString& append(char c, size_t count)
    {
        dstr_append_cc(pImp(), c, count);
        return *this;
    }

    DString& append(const char* value)
    {
        dstr_append_sz(pImp(), value);
        return *this;
    }

    DString& append(const DString& rhs)
    {
        dstr_append_ds(pImp(), rhs.pImp());
        return *this;
    }

    DString& append(const char* buff, size_t len)
    {
        dstr_append_bl(pImp(), buff, len);
        return *this;
    }

    DString& append(const char* first, const char* last)
    {
        dstr_append_range(pImp(), first, last);
        return *this;
    }

    DString& append_sprintf(const char* fmt, ...);

    DString& append_vsprintf(const char* fmt, va_list args) {
        dstr_append_vsprintf(pImp(), fmt, args);
        return *this;
    }

    DString& operator+=(char c) {
        return append(c);
    }

    DString& operator+=(const char* sz) {
        return append(sz);
    }

    DString& operator+=(const DString& ds) {
        return append(ds);
    }

    // Join inplace
    //
    template <class Container>
    DString& join_inplace(const char* sep, const Container& v);

    template <class Container>
    DString& join_inplace(const DString& sep, const Container& v) {
        return join_inplace(sep.c_str(), v);
    }

    DString& join_inplace(const char* sep, const char* argv[], size_t argc) {
        dstr_join_sz(pImp(), sep, argv, argc);
        return *this;
    }

    DString& join_inplace(const char* sep, char* argv[], size_t argc) {
        dstr_join_sz(pImp(), sep, (const char**)argv, argc);
        return *this;
    }

    DString& join_inplace(const DString& sep, const char* argv[], size_t argc) {
        return join_inplace(sep.c_str(), argv, argc);
    }

    // join
    //
    template <class Container>
    DString join(const Container& v) const {
        DString result;
        result.join_inplace(this->c_str(), v);
        return result;
    }

    DString join(const char* argv[], size_t argc) const {
        DString result;
        result.join_inplace(this->c_str(), argv, argc);
        return result;
    }

    // Replace functions
    //
    DString& replace(size_t pos, size_t len, char c, size_t count)
    {
        dstr_replace_cc(pImp(), pos, len, c, count);
        return *this;
    }

    DString& replace(size_t pos, size_t len, const char* value)
    {
        dstr_replace_sz(pImp(), pos, len, value);
        return *this;
    }

    DString& replace(size_t pos, size_t len, const DString& rhs)
    {
        dstr_replace_ds(pImp(), pos, len, rhs.pImp());
        return *this;
    }

    DString& replace(size_t pos, size_t len, const char* buff, size_t bufflen)
    {
        dstr_replace_bl(pImp(), pos, len, buff, bufflen);
        return *this;
    }

    DString& replace(size_t pos, size_t len, const char* first, const char* last)
    {
        dstr_replace_range(pImp(), pos, len, first, last);
        return *this;
    }

    DString& replace_all(const char* oldstr,
                         const char* newstr,
                         size_t count = DSTR_REPLACE_ALL)
    {
        dstr_replace_all_sz(pImp(), oldstr, newstr, count);
        return *this;
    }

    DString& replace_all(const DString& oldstr,
                         const DString& newstr,
                         size_t count = DSTR_REPLACE_ALL)
    {
        dstr_replace_all_ds(pImp(), oldstr.pImp(), newstr.pImp(), count);
        return *this;
    }

    DString& translate(const char* from, const char* to)
    {
        dstr_translate(pImp(), from, to);
        return *this;
    }

    DString& tr(const char* from, const char* to)
    {
        return translate(from, to);
    }

    DString& squeeze(const char* sqz_set)
    {
        dstr_squeeze(pImp(), sqz_set);
        return *this;
    }

    DString& translate_squeeze(const char* from, const char* to)
    {
        dstr_translate_squeeze(pImp(), from, to);
        return *this;
    }

    DString& tr_s(const char* from, const char* to)
    {
        return translate_squeeze(from, to);
    }

    template <class Container>
    void split(char c, Container& dest) const;

    template <class Container>
    void split(const char* separators, Container& dest) const;

    template <class Container>
    void split(const DString& separators, Container& dest)  const {
        split(separators.c_str(), dest);
    }

    template <class Container>
    void splitlines(Container& dest) const {
        split("\r\n", dest);
    }

    template <class Container>
    void partition(const char* s, Container& dest) const;

    template <class Container>
    void rpartition(const char* s, Container& dest) const;

    template <class Container>
    void partition(const DString& s, Container& dest) const {
        partition(s.c_str(), dest);
    }

    template <class Container>
    void rpartition(const DString& s, Container& dest) const {
        rpartition(s.c_str(), dest);
    }

    void reserve(size_t len)
    {
        dstr_reserve(pImp(), len);
    }

    // shrink
    //
    void resize(size_t len)
    {
        dstr_resize(pImp(), len);
    }

    // Text manipulations (upper, lower etc)
    //
    void upper()
    {
        dstr_ascii_upper(pImp());
    }

    void lower()
    {
        dstr_ascii_lower(pImp());
    }

    void swapcase()
    {
        dstr_ascii_swapcase(pImp());
    }

    void reverse()
    {
        dstr_reverse(pImp());
    }

    void trim_right()
    {
        dstr_trim_right(pImp());
    }

    void trim_left()
    {
        dstr_trim_left(pImp());
    }

    void trim()
    {
        dstr_trim_both(pImp());
    }

    void clear()
    {
        dstr_clear(pImp());
    }

    void erase(size_t pos, size_t count = 1)
    {
        dstr_remove(pImp(), pos, count);
    }

    void swap(DString& rhs)
    {
        dstr_swap(pImp(), rhs.pImp());
    }

    size_t copy_substr(size_t pos, size_t numbytes, char dest[], size_t destsize)
    {
        return dstr_substr(pImp(), pos, numbytes, dest, destsize);
    }

    // Inline queries
    //
    const char* c_str()    const { return m_imp.data;     }
    const char* data()     const { return m_imp.data;     }
    size_t      length()   const { return m_imp.length;   }
    size_t      capacity() const { return m_imp.capacity; }
    bool        empty()    const { return length() == 0;  }

    bool index_ok(size_t pos) const {
        return dstr_valid_index(pImp(), pos);
    }

    char get(long pos) const {
        return dstr_getchar_safe(pImp(), pos);
    }

    void put(long pos, char c) {
        dstr_putchar_safe(pImp(), pos, c);
    }

    char operator[](size_t pos) const {
        return m_imp.data[pos];
    }

    char& operator[](size_t pos) {
        return m_imp.data[pos];
    }

    unsigned int hash() const {
        return dstr_hash(pImp());
    }

    long atoi() const {
        return dstr_atoi(pImp());
    }

    double atof() const {
        return dstr_atof(pImp());
    }

    long atoll() const {
        return dstr_atoll(pImp());
    }

    void itos(long long n) {
        dstr_itos(pImp(), n);
    }

    void itos_ul(unsigned long long n, unsigned int base = 10) {
        dstr_itos_ul(pImp(), n, base);
    }

    size_t find(char c, size_t pos=0) const {
        return dstr_find_c(pImp(), pos, c);
    }

    size_t find(const char* sz, size_t pos=0) const {
        return dstr_find_sz(pImp(), pos, sz);
    }

    size_t ifind(char c, size_t pos=0) const {
        return dstr_ifind_c(pImp(), pos, c);
    }

    size_t ifind(const char* sz, size_t pos=0) const {
        return dstr_ifind_sz(pImp(), pos, sz);
    }

    size_t rfind(char c, size_t pos = NPOS) const {
        return dstr_rfind_c(pImp(), pos, c);
    }

    size_t rfind(const char* sz, size_t pos = NPOS) const {
        return dstr_rfind_sz(pImp(), pos, sz);
    }

    size_t irfind(char c, size_t pos = NPOS) const {
        return dstr_irfind_c(pImp(), pos, c);
    }

    size_t irfind(const char* sz, size_t pos = NPOS) const {
        return dstr_irfind_sz(pImp(), pos, sz);
    }

    size_t count(const char* sz) const {
        return dstr_count_sz(pImp(), sz);
    }

    size_t count(const DString& ds) const {
        return dstr_count_ds(pImp(), ds.pImp());
    }

    size_t icount(const char* sz) const {
        return dstr_icount_sz(pImp(), sz);
    }

    size_t icount(const DString& ds) const {
        return dstr_icount_ds(pImp(), ds.pImp());
    }

    bool contains(const char* s) const {
        return dstr_contains_sz(pImp(), s);
    }

    bool icontains(const char* s) const {
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

    bool startswith(const char* s) const {
        return dstr_prefix_sz(pImp(), s);
    }

    bool istartswith(const char* s) const {
        return dstr_iprefix_sz(pImp(), s);
    }

    bool endswith(const char* s) const {
        return dstr_suffix_sz(pImp(), s);
    }

    bool iendswith(const char* s) const {
        return dstr_isuffix_sz(pImp(), s);
    }

    // ffo = find_first of
    //
    size_t ffo(const char* s, size_t pos = 0) const {
        return dstr_ffo_sz(pImp(), pos, s);
    }

    size_t ffo(const DString& rhs, size_t pos = 0) const {
        return dstr_ffo_ds(pImp(), pos, rhs.pImp());
    }

    // ffno = find first not of
    //
    size_t ffno(const char* s, size_t pos = 0) const {
        return dstr_ffno_sz(pImp(), pos, s);
    }

    size_t ffno(const DString& rhs, size_t pos = 0) const {
        return dstr_ffno_ds(pImp(), pos, rhs.pImp());
    }

    // flo = find_last_of
    //
    size_t flo(const char* s, size_t pos = DSTR_NPOS) const {
        return dstr_flo_sz(pImp(), pos, s);
    }

    size_t flo(const DString& rhs, size_t pos = DSTR_NPOS) const {
        return dstr_flo_ds(pImp(), pos, rhs.pImp());
    }

    // flno = find_last_not_of
    //
    size_t flno(const char* s, size_t pos = DSTR_NPOS) const {
        return dstr_flno_sz(pImp(), pos, s);
    }

    size_t flno(const DString& rhs, size_t pos = DSTR_NPOS) const {
        return dstr_flno_ds(pImp(), pos, rhs.pImp());
    }

    // Comparisons and operators ==, !=, >, <, >=, <=
    //
    // Note: NULL C string is considered empty string ""
    //
    int compare(const char* sz)      const { return strcmp(data(), sz ? sz : ""); }
    int icompare(const char* sz)     const { return strcasecmp(data(), sz ? sz : ""); }
    int compare(const DString& rhs)  const { return strcmp(data(), rhs.data()); }
    int icompare(const DString& rhs) const { return strcasecmp(data(), rhs.data()); }

    bool iequal(const char* sz)     const { return (icompare(sz) == 0);  }
    bool iequal(const DString& rhs) const { return (icompare(rhs) == 0); }

    bool operator!=(const char* sz) const { return (compare(sz) != 0);  }
    bool operator<(const char* sz) const  { return (compare(sz) < 0);   }
    bool operator>(const char* sz) const  { return (compare(sz) > 0);   }
    bool operator<=(const char* sz) const { return (compare(sz) <= 0);  }
    bool operator>=(const char* sz) const { return (compare(sz) >= 0);  }

    bool operator<(const DString& rhs) const  { return (compare(rhs) < 0);  }
    bool operator>(const DString& rhs) const  { return (compare(rhs) > 0);  }
    bool operator<=(const DString& rhs) const { return (compare(rhs) <= 0); }
    bool operator>=(const DString& rhs) const { return (compare(rhs) >= 0); }

    // micro optimization - check length before buffer
    //
    bool operator==(const char* sz) const {
        return (compare(sz) == 0);
    }
    bool operator==(const DString& rhs) const {
        return (size() == rhs.size()) && (compare(rhs) == 0);
    }
    bool operator!=(const DString& rhs) const {
        return (size() != rhs.size()) || (compare(rhs) != 0);
    }

    int fgets(FILE* fp) {
        return dstr_fgets(pImp(), fp);
    }

    int fgetline(FILE* fp) {
        return dstr_fgetline(pImp(), fp);
    }

    int stoi(size_t* index = nullptr, int base = 10) {
        return dstr_to_int(pImp(), index, base);
    }

    long stol(size_t* index = nullptr, int base = 10) {
        return dstr_to_long(pImp(), index, base);
    }

    unsigned long stoul(size_t* index = nullptr, int base = 10) {
        return dstr_to_ulong(pImp(), index, base);
    }

    long long stoll(size_t* index = nullptr, int base = 10) {
        return dstr_to_llong(pImp(), index, base);
    }

    unsigned long long stoull(size_t* index = nullptr, int base = 10) {
        return dstr_to_ullong(pImp(), index, base);
    }

    float stof(size_t* index = nullptr) {
        return dstr_to_float(pImp(), index);
    }

    double stod(size_t* index = nullptr) {
        return dstr_to_double(pImp(), index);
    }

    long double stold(size_t* index = nullptr) {
        return dstr_to_ldouble(pImp(), index);
    }

    // C++ algorithms support : typedefs
    //
    typedef char value_type;
    typedef char& reference;
    typedef const char& const_reference;
    typedef char* iterator;
    typedef const char* const_iterator;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;

    // C++ algorithms support : functions
    //
    size_type      size()  const { return length(); }
    iterator       begin()       { return m_imp.data; }
    const_iterator begin() const { return m_imp.data; }
    iterator       end()         { return &m_imp.data[m_imp.length]; }
    const_iterator end()   const { return &m_imp.data[m_imp.length]; }

private:
    // Single data member.
    //
    DSTR_TYPE m_imp;

private:
    CDSTR pImp() const { return &m_imp; }
    DSTR  pImp()       { return &m_imp; }

    void init_capacity(size_t len)
    {
        if (len < DSTR_INITIAL_CAPACITY) {
            m_imp.capacity = DSTR_INITIAL_CAPACITY;
            m_imp.data = m_imp.sso_buffer;
        }
        else {
            dstr_grow_ctor(&m_imp, len);
        }
    }

    void init_data(char c, size_t count) {
        memset(m_imp.data, c, count);
    }

    void init_data(const char* p, size_t count) {
        memcpy(m_imp.data, p, count);
    }

    void init_length(size_t count) {
        m_imp.length = count;
        m_imp.data[count] = '\0';
    }

    // C++98-like compile time assert for container value type
    //
    template <typename T>
    const DString* chk_vtype(const T* t) const { return t; }
};
//----------------------------------------------------------------

inline DString operator+(const DString& lhs, const DString& rhs)
{
    DString result(lhs);
    result.append(rhs);
    return result;
}

inline DString operator+(const DString& lhs, char ch)
{
    DString result(lhs);
    result.append(ch);
    return result;
}

inline DString operator+(char ch, const DString& rhs)
{
    DString result(ch, 1);
    result.append(rhs);
    return result;
}

inline DString operator+(const DString& lhs, const char* sz)
{
    DString result(lhs);
    result.append(sz);
    return result;
}

inline DString operator+(const char* sz, const DString& rhs)
{
    DString result(sz);
    result.append(rhs);
    return result;
}
//----------------------------------------------------------------

// DString and iostream
//
std::ostream& operator<<(std::ostream& out, const DString& s);
std::istream& operator>>(std::istream& in, DString& s);
std::istream& io_getline(std::istream& in, DString& s);
//----------------------------------------------------------------

// to_dstring() functions
//
inline DString to_dstring(int val) {
    DString r;
    r.itos(val);
    return r;
}

inline DString to_dstring(unsigned int val) {
    DString r;
    r.itos_ul(val);
    return r;
}

inline DString to_dstring(long val) {
    DString r;
    r.itos(val);
    return r;
}

inline DString to_dstring(unsigned long val) {
    DString r;
    r.itos_ul(val);
    return r;
}

inline DString to_dstring(long long val) {
    DString r;
    r.itos(val);
    return r;
}

inline DString to_dstring(unsigned long long val) {
    DString r;
    r.itos_ul(val);
    return r;
}

#define NUMBER_TO_DSTRING(v, fmt) do {           \
    char buf[40];                                \
    int n = sprintf(buf, fmt, v);                \
    DString res; res.assign(buf, buf + n);       \
    return res; } while(0)

inline DString to_dstring(float val) {
    NUMBER_TO_DSTRING(val, "%f");
}

inline DString to_dstring(double val) {
    NUMBER_TO_DSTRING(val, "%f");
}

inline DString to_dstring(long double val) {
    NUMBER_TO_DSTRING(val, "%Lf");
}

#undef NUMBER_TO_DSTRING
//----------------------------------------------------------------

// C++98-like compile time assert for container value type
//
#define CHECK_VALUE_TYPE(Cnt) ((void)chk_vtype((typename Cnt::value_type*) 0))

template <class Container>
void DString::split(char sep, Container& dest) const
{
    CHECK_VALUE_TYPE(Container);

    Container v;
    DString str;

    for (size_t i = 0; i < size(); ++i) {
        char c = get(i);
        if (c == sep) {
            v.push_back(str);
            str.clear();
        }
        else {
            str.append(c);
        }
    }

    if (!str.empty()) {
        v.push_back(str);
    }

    dest.swap(v);
}
//-----------------------------------------------------------

template <class Container>
void DString::split(const char* pattern, Container& dest) const
{
    CHECK_VALUE_TYPE(Container);

    Container tmp;

    // Find the first location which does not belong to
    // the separator characters
    //
    size_t first = this->ffno(pattern, 0);

    // Check if we are at the end
    //
    while (first != DString::NPOS)
    {
        // Find the first location (> first) with a character
        // that belongs to the separator group
        //
        size_t last = this->ffo(pattern, first);

        // Create a substring to print
        //
        DString token = this->substr(first, last - first);
        tmp.push_back(token);

        // Prepare for next iteration.
        // Again find the first char not in separator but now
        // not from the start
        //
        first = this->ffno(pattern, last);
    }

    dest.swap(tmp);
}
//-----------------------------------------------------------

template <class Container>
DString& DString::join_inplace(const char* sep, const Container& v)
{
    CHECK_VALUE_TYPE(Container);

    if (v.empty())
        return *this;

    if (!sep)
        sep = "";

    typename Container::const_iterator p = v.begin();

    for (;;) {
        this->append(*p);
        if (++p == v.end()) break;
        this->append(sep);
    }

    return *this;
}
//-----------------------------------------------------------

template <class Container>
void DString::partition(const char* s, Container& dest) const
{
    CHECK_VALUE_TYPE(Container);


    struct DSTR_PartInfo pinfo;
    dstr_partition(pImp(), s, &pinfo);

    Container tmp;
    tmp.push_back(substr(pinfo.l_pos, pinfo.l_len));
    tmp.push_back(substr(pinfo.m_pos, pinfo.m_len));
    tmp.push_back(substr(pinfo.r_pos, pinfo.r_len));
    tmp.swap(dest);
}
//-----------------------------------------------------------

template <class Container>
void DString::rpartition(const char* s, Container& dest) const
{
    CHECK_VALUE_TYPE(Container);

    struct DSTR_PartInfo pinfo;
    dstr_rpartition(pImp(), s, &pinfo);

    Container tmp;
    tmp.push_back(substr(pinfo.l_pos, pinfo.l_len));
    tmp.push_back(substr(pinfo.m_pos, pinfo.m_len));
    tmp.push_back(substr(pinfo.r_pos, pinfo.r_len));
    tmp.swap(dest);
}
//-----------------------------------------------------------



#endif
