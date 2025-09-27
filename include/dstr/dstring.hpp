#ifndef DSTRING_H_INCLUDED
#define DSTRING_H_INCLUDED

#include <dstr/dstr.h>

class DString {
public:
    static const size_t NPOS = DSTR_NPOS;
    static const bool IgnoreCase = true;

    // Constructors
    //
    DString()
    {
    }

    DString(char c, size_t count)
    {
        assign(c, count);
    }

    DString(const char* sz)
    {
        assign(sz);
    }

    DString(const DString& rhs)
    {
        assign(rhs);
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
        assign(buffer, len);
    }

    // Destructor
    //
    ~DString()
    {
        if (capacity() > DSTR_INITIAL_CAPACITY)
            dstr_clean_data(pImp());
    }

    DString substr(size_t pos, size_t count) const
    {
        return DString(*this, pos, count);
    }

    // slurp a testfile into DString
    //
    int from_file(const char* fname)
    {
        if (dstr_assign_fromfile(pImp(), fname))
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

    DString& sprintf(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        dstr_assign_vsprintf(pImp(), fmt, args);
        va_end(args);
        return *this;
    }

    DString& vsprintf(const char* fmt, va_list args)
    {
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

    DString& append_sprintf(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        dstr_append_vsprintf(pImp(), fmt, args);
        va_end(args);
        return *this;
    }

    DString& append_vsprintf(const char* fmt, va_list args)
    {
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
        return dstr_cstring(pImp());
    }

    size_t length() const {
        return dstr_length(pImp());
    }

    size_t capacity() const {
        return dstr_capacity(pImp());
    }

    bool empty() const {
        return dstr_isempty(pImp());
    }

    bool index_ok(size_t pos) const {
        return dstr_valid_index(pImp(), pos);
    }

    char getchar(long pos) const {
        return dstr_getchar_safe(pImp(), pos);
    }

    void putchar(long pos, char c) {
        dstr_putchar_safe(pImp(), pos, c);
    }

    char operator[](size_t pos) const {
        return dstr_getchar(pImp(), pos);
    }

    char& operator[](size_t pos) {
        return pImp()->data[pos];
    }

    unsigned int hash() const {
        return dstr_hash(pImp());
    }

    long atoi() const {
        return dstr_atoi(pImp());
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

    size_t ifind(const char*sz, size_t pos=0) const {
        return dstr_ifind_sz(pImp(), pos, sz);
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

    bool startswith(const char* s, bool ignore_case=false) const {
        return ignore_case ?
            dstr_iprefix_sz(pImp(), s) :
            dstr_prefix_sz(pImp(), s);
    }

    bool endswith(const char* s, bool ignore_case=false) const {
        return ignore_case ?
            dstr_isuffix_sz(pImp(), s) :
            dstr_suffix_sz(pImp(), s);
    }

    size_t ffo(const char* s, size_t pos = 0) const {
        return dstr_ffo_sz(pImp(), pos, s);
    }

    size_t ffo(const DString& rhs, size_t pos = 0) const {
        return dstr_ffo_ds(pImp(), pos, rhs.pImp());
    }

    size_t ffno(const char* s, size_t pos = 0) const {
        return dstr_ffno_sz(pImp(), pos, s);
    }

    size_t ffno(const DString& rhs, size_t pos = 0) const {
        return dstr_ffno_ds(pImp(), pos, rhs.pImp());
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

    struct DSTR_IMP_Aux : DSTR_IMP {
        DSTR_IMP_Aux()
        {
            length = 0;
            capacity = DSTR_INITIAL_CAPACITY;
            sso_buffer[0] = '\0';
            data = &sso_buffer[0];
        }
    };

    DSTR_IMP_Aux m_imp;
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

#endif
