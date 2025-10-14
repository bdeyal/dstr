#ifndef DSTRING_H_INCLUDED
#define DSTRING_H_INCLUDED

#include <iosfwd>
#include <vector>
#include <dstr/dstr.h>

class DString {
public:
    static const size_t NPOS = DSTR_NPOS;

    // Constructors
    //
    DString()
    {
    }

    DString(char c, size_t count)
    {
        if (c == '\0' || count == 0) return;
        if (count >= DSTR_INITIAL_CAPACITY) {
            dstr_grow_ctor(pImp(), count);
        }
        m_imp.length = count;
        memset(m_imp.data, c, count);
        m_imp.data[count] = '\0';
    }

    DString(const char* sz)
    {
        if (!sz) return;
        size_t len = strlen(sz);
        if (len >= DSTR_INITIAL_CAPACITY) {
            dstr_grow_ctor(pImp(), len);
        }
        m_imp.length = len;
        memcpy(m_imp.data, sz, len + 1);
    }

    DString(const DString& rhs)
    {
        if (rhs.empty()) return;
        if (rhs.length() >= DSTR_INITIAL_CAPACITY) {
            dstr_grow_ctor(pImp(), rhs.length());
        }
        m_imp.length = rhs.length();
        memcpy(m_imp.data, rhs.m_imp.data, rhs.length() + 1);
    }

#if __cplusplus >= 201103L
    DString(DString&& rhs)
    {
        swap(rhs);
    }
#endif

    DString(const DString& rhs, size_t pos, size_t count)
    {
        assign(rhs, pos, count);
    }

    DString(const char* buffer, size_t len)
    {
        if (!buffer) return;
        if ((len = strnlen(buffer, len)) == 0) return;
        if (len >= DSTR_INITIAL_CAPACITY) {
            dstr_grow_ctor(pImp(), len);
        }
        m_imp.length = len;
        memcpy(m_imp.data, buffer, len);
        m_imp.data[len] = '\0';
    }

    DString(const char* first, const char* last)
    {
        size_t distance = last - first;
        if (distance == 0) return;
        if (distance >= DSTR_INITIAL_CAPACITY) {
            dstr_grow_ctor(pImp(), distance);
        }
        m_imp.length = distance;
        memcpy(m_imp.data, first, distance);
        m_imp.data[distance] = '\0';
    }

    // Destructor
    //
    ~DString()
    {
        if (capacity() > DSTR_INITIAL_CAPACITY)
            dstr_clean_data(pImp());
    }

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

    // slurp a testfile into DString
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
        assign(sz);
        return *this;
    }

    DString& operator=(char c)
    {
        assign(c);
        return *this;
    }

    DString& assign(char c)
    {
        clear();
        append(c);
        return *this;
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
    DString& vsprintf(const char* fmt, va_list args);

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
    DString& append_vsprintf(const char* fmt, va_list args);

    DString& operator+=(char c) {
        return append(c);
    }

    DString& operator+=(const char* sz) {
        return append(sz);
    }

    DString& operator+=(const DString& ds) {
        return append(ds);
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

    void split(char c, std::vector<DString>& dest) const;
    void split(const char* separators, std::vector<DString>& dest) const;
    void split(const DString& separators, std::vector<DString>& dest)  const {
        split(separators.c_str(), dest);
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

    void truncate()
    {
        dstr_truncate(pImp());
    }

    void clear()
    {
        truncate();
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

    // Inline fast queries
    //
    const char* c_str() const {
        return m_imp.data;
    }

    size_t length() const {
        return m_imp.length;
    }

    size_t capacity() const {
        return m_imp.capacity;
    }

    bool empty() const {
        return length() == 0;
    }

    bool index_ok(size_t pos) const {
        return dstr_valid_index(pImp(), pos);
    }

    char get_char(long pos) const {
        return dstr_getchar_safe(pImp(), pos);
    }

    void put_char(long pos, char c) {
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

    void itoa(long n) {
        dstr_itoa(pImp(), n);
    }

    bool isdigits() const {
        return dstr_isdigits(pImp());
    }

    bool isxdigits() const {
        return dstr_isxdigits(pImp());
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

    bool isblank() const {
        return dstr_isblank(pImp());
    }

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

    int compare(const char* sz) const {
        return dstr_compare_sz(pImp(), sz);
    }

    int compare(const DString& rhs) const {
        return dstr_compare_ds(pImp(), rhs.pImp());
    }

    bool operator==(const char* sz) const {
        return dstr_equal_sz(pImp(), sz);
    }
    bool operator!=(const char* sz) const {
        return !dstr_equal_sz(pImp(), sz);
    }
    bool operator<(const char* sz) const  { return (compare(sz) < 0);   }
    bool operator>(const char* sz) const  { return (compare(sz) > 0);   }
    bool operator<=(const char* sz) const { return (compare(sz) <= 0);  }
    bool operator>=(const char* sz) const { return (compare(sz) >= 0);  }

    bool operator==(const DString& rhs) const {
        return dstr_equal_ds(pImp(), rhs.pImp());
    }
    bool operator!=(const DString& rhs) const {
        return !dstr_equal_ds(pImp(), rhs.pImp());
    }
    bool operator<(const DString& rhs) const  { return (compare(rhs) < 0);  }
    bool operator>(const DString& rhs) const  { return (compare(rhs) > 0);  }
    bool operator<=(const DString& rhs) const { return (compare(rhs) <= 0); }
    bool operator>=(const DString& rhs) const { return (compare(rhs) >= 0); }

    bool iequal(const char* sz) const {
        return dstr_iequal_sz(pImp(), sz);
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

    CDSTR pImp() const { return &m_imp; }
    DSTR  pImp()       { return &m_imp; }

    struct DSTR_TYPE_Wrapper : DSTR_TYPE {
        DSTR_TYPE_Wrapper()
        {
            dstr_init_data(this);
        }
    };

    DSTR_TYPE_Wrapper m_imp;
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

// DString and iostream
//
std::ostream& operator<<(std::ostream& out, const DString& s);
std::istream& operator>>(std::istream& in, DString& s);
std::istream& io_getline(std::istream& in, DString& s);

// to_dstring() functions
//
inline DString to_dstring(int val) {
    DString result;
    result.sprintf("%d", val);
    return result;
}

inline DString to_dstring(unsigned int val) {
    DString result;
    result.sprintf("%u", val);
    return result;
}

inline DString to_dstring(long val) {
    DString result;
    result.sprintf("%ld", val);
    return result;
}

inline DString to_dstring(unsigned long val) {
    DString result;
    result.sprintf("%lu", val);
    return result;
}

inline DString to_dstring(long long val) {
    DString result;
    result.sprintf("%lld", val);
    return result;
}

inline DString to_dstring(unsigned long long val) {
    DString result;
    result.sprintf("%llu", val);
    return result;
}

inline DString to_dstring(float val) {
    DString result;
    result.sprintf("%f", val);
    return result;
}

inline DString to_dstring(double val) {
    DString result;
    result.sprintf("%f", val);
    return result;
}

inline DString to_dstring(long double val) {
    DString result;
    result.sprintf("%Lf", val);
    return result;
}



#endif
