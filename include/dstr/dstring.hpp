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
        init();
    }

    explicit DString(size_t len)
    {
        init();
        resize(len);
    }

    DString(char c, size_t count)
    {
        init();
        assign(c, count);
    }

    DString(const char* sz)
    {
        init();
        assign(sz);
    }

    DString(const DString& rhs)
    {
        init();
        assign(rhs);
    }

#if __cplusplus >= 201103L
    DString(DString&& rhs)
    {
        init();
        dstr_swap(&m_imp, &rhs.m_imp);
    }
#endif

    DString(const DString& rhs, size_t pos, size_t count)
    {
        init();
        assign(rhs, pos, count);
    }

    DString(const char* buffer, size_t len)
    {
        init();
        assign(buffer, len);
    }

    // Destructor
    //
    ~DString()
    {
        dstr_clean_data(ptr());
    }

    // slurp a testfile into DString
    //
    int from_file(const char* fname)
    {
        if (dstr_assign_fromfile(ptr(), fname))
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
            dstr_assign_ds(ptr(), rhs.cptr());
        return *this;
    }

    DString& operator=(const char* sz)
    {
        dstr_assign_sz(ptr(), sz);
        return *this;
    }

    DString& operator=(char c)
    {
        assign(c);
        return *this;
    }

    void assign(char c)
    {
        clear();
        append(c);
    }

    void assign(char c, size_t count)
    {
        dstr_assign_cc(ptr(), c, count);
    }

    void assign(const char* sz)
    {
        dstr_assign_sz(ptr(), sz);
    }

    void assign(const char* buff, size_t len)
    {
        dstr_assign_bl(ptr(), buff, len);
    }

    void assign(const DString& rhs)
    {
        dstr_assign_ds(ptr(), rhs.cptr());
    }

    void assign(const DString& rhs, size_t pos, size_t count)
    {
        dstr_assign_substr(ptr(), rhs.cptr(), pos, count);
    }

    void sprintf(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        dstr_assign_vsprintf(ptr(), fmt, args);
        va_end(args);
    }

    void vsprintf(const char* fmt, va_list args)
    {
        dstr_assign_vsprintf(ptr(), fmt, args);
    }

    // Insertion functions
    //
    void insert(size_t pos, char c, size_t count)
    {
        dstr_insert_cc(ptr(), pos, c, count);
    }

    void insert(size_t pos, const char* value)
    {
        dstr_insert_sz(ptr(), pos, value);
    }

    void insert(size_t pos, const DString& rhs)
    {
        dstr_insert_ds(ptr(), pos, rhs.cptr());
    }

    void insert(size_t pos, const char* buff, size_t len)
    {
        dstr_insert_bl(ptr(), pos, buff, len);
    }

    // Append at the end
    //
    void append(char c)
    {
        dstr_append_c(ptr(), c);
    }

    void push_back(char c)
    {
        append(c);
    }

    void push_front(char c)
    {
        insert(0, c, 1);
    }

    void pop_front()
    {
        erase(0);
    }

    void append(char c, size_t count)
    {
        dstr_append_cc(ptr(), c, count);
    }

    void append(const char* value)
    {
        dstr_append_sz(ptr(), value);
    }

    void append(const DString& rhs)
    {
        dstr_append_ds(ptr(), rhs.cptr());
    }

    void append(const char* buff, size_t len)
    {
        dstr_append_bl(ptr(), buff, len);
    }

    void append_sprintf(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        dstr_append_vsprintf(ptr(), fmt, args);
        va_end(args);
    }

    void append_vsprintf(const char* fmt, va_list args)
    {
        dstr_append_vsprintf(ptr(), fmt, args);
    }

    DString& operator+=(char c) {
        append(c);
        return *this;
    }

    DString& operator+=(const char* sz) {
        append(sz);
        return *this;
    }

    DString& operator+=(const DString& ds) {
        append(ds);
        return *this;
    }

#if 0
    // TODO complete operator+
    DString  operator+(char c);
    DString  operator+(const char* sz);
    DString  operator+(const DString& ds);
#endif

    // Replace functions
    //
    void replace(size_t pos, size_t len, char c, size_t count)
    {
        dstr_replace_cc(ptr(), pos, len, c, count);
    }

    void replace(size_t pos, size_t len, const char* value)
    {
        dstr_replace_sz(ptr(), pos, len, value);
    }

    void replace(size_t pos, size_t len, const DString& rhs)
    {
        dstr_replace_ds(ptr(), pos, len, rhs.cptr());
    }

    void replace(size_t pos, size_t len, const char* buff, size_t bufflen)
    {
        dstr_replace_bl(ptr(), pos, len, buff, bufflen);
    }

    // shrink
    //
    void resize(size_t len)
    {
        dstr_resize(ptr(), len);
    }

    // Text manipulations (upper, lower etc)
    //
    void upper()
    {
        dstr_ascii_upper(ptr());
    }

    void lower()
    {
        dstr_ascii_lower(ptr());
    }

    void reverse()
    {
        dstr_reverse(ptr());
    }

    void trim_right()
    {
        dstr_trim_right(ptr());
    }

    void trim_left()
    {
        dstr_trim_left(ptr());
    }

    void trim()
    {
        dstr_trim_both(ptr());
    }

    void truncate()
    {
        dstr_truncate(ptr());
    }

    void clear()
    {
        truncate();
    }

    void erase(size_t pos, size_t count = 1)
    {
        dstr_remove(ptr(), pos, count);
    }

    void chop()
    {
        dstr_chop(ptr());
    }

    void pop_back()
    {
        chop();
    }

    void swap(DString& rhs)
    {
        dstr_swap(ptr(), &rhs.m_imp);
    }

    size_t copy_substr(size_t pos, size_t numbytes, char dest[], size_t destsize)
    {
        return dstr_substr(ptr(), pos, numbytes, dest, destsize);
    }

    // Inline fast queries
    //
    const char* c_str() const {
        return dstr_cstring(cptr());
    }

    size_t length() const {
        return dstr_length(cptr());
    }

    size_t size() const {
        return length();
    }

    size_t capacity() const {
        return dstr_capacity(cptr());
    }

    bool empty() const {
        return dstr_isempty(cptr());
    }

    bool index_ok(size_t pos) const {
        return dstr_valid_index(cptr(), pos);
    }

    char getchar(long pos) const {
        return dstr_getchar_safe(cptr(), pos);
    }

    void putchar(long pos, char c) {
        dstr_putchar_safe(ptr(), pos, c);
    }

    char operator[](size_t pos) const {
        return dstr_getchar(cptr(), pos);
    }

    char& operator[](size_t pos) {
        return DBUF(ptr())[pos];
    }

    unsigned int hash() const {
        return dstr_hash(cptr());
    }

    long atoi() const {
        return dstr_atoi(cptr());
    }

    long atoll() const {
        return dstr_atoll(cptr());
    }

    void itoa(long n) {
        dstr_itoa(ptr(), n);
    }

    bool isdigits() const {
        return dstr_isdigits(cptr());
    }

    bool isxdigits() const {
        return dstr_isxdigits(cptr());
    }

    size_t find(char c, size_t pos=0) const {
        return dstr_find_c(cptr(), pos, c);
    }

    size_t find(const char* sz, size_t pos=0) const {
        return dstr_find_sz(cptr(), pos, sz);
    }

    size_t ifind(char c, size_t pos=0) const {
        return dstr_ifind_c(cptr(), pos, c);
    }

    size_t ifind(const char*sz, size_t pos=0) const {
        return dstr_ifind_sz(cptr(), pos, sz);
    }

    bool contains(const char* s) const {
        return dstr_contains_sz(cptr(), s);
    }

    bool icontains(const char* s) const {
        return dstr_icontains_sz(cptr(), s);
    }

    bool isblank() const {
        return dstr_isblank(cptr());
    }

    bool startswith(const char* s, bool ignore_case=false) const {
        return ignore_case ?
            dstr_iprefix_sz(cptr(), s) :
            dstr_prefix_sz(cptr(), s);
    }

    bool endswith(const char* s, bool ignore_case=false) const {
        return ignore_case ?
            dstr_isuffix_sz(cptr(), s) :
            dstr_suffix_sz(cptr(), s);
    }

    size_t ffo(const char* s, size_t pos = 0) const {
        return dstr_ffo_sz(cptr(), pos, s);
    }

    size_t ffo(const DString& rhs, size_t pos = 0) const {
        return dstr_ffo_ds(cptr(), pos, rhs.cptr());
    }

    size_t ffno(const char* s, size_t pos = 0) const {
        return dstr_ffno_sz(cptr(), pos, s);
    }

    size_t ffno(const DString& rhs, size_t pos = 0) const {
        return dstr_ffno_ds(cptr(), pos, rhs.cptr());
    }

    int compare(const char* sz) const {
        return dstr_compare_sz(cptr(), sz);
    }

    int compare(const DString& rhs) const {
        return dstr_compare_ds(cptr(), rhs.cptr());
    }

    bool operator==(const char* sz) const {
        return dstr_equal_sz(cptr(), sz);
    }
    bool operator!=(const char* sz) const {
        return !dstr_equal_sz(cptr(), sz);
    }
    bool operator<(const char* sz) const  { return (compare(sz) < 0);   }
    bool operator>(const char* sz) const  { return (compare(sz) > 0);   }
    bool operator<=(const char* sz) const { return (compare(sz) <= 0);  }
    bool operator>=(const char* sz) const { return (compare(sz) >= 0);  }

    bool operator==(const DString& rhs) const {
        return dstr_equal_ds(cptr(), rhs.cptr());
    }
    bool operator!=(const DString& rhs) const {
        return !dstr_equal_ds(cptr(), rhs.cptr());
    }
    bool operator<(const DString& rhs) const  { return (compare(rhs) < 0);  }
    bool operator>(const DString& rhs) const  { return (compare(rhs) > 0);  }
    bool operator<=(const DString& rhs) const { return (compare(rhs) <= 0); }
    bool operator>=(const DString& rhs) const { return (compare(rhs) >= 0); }

    bool iequal(const char* sz) const {
        return dstr_iequal_sz(cptr(), sz);
    }

    int fgets(FILE* fp) {
        return dstr_fgets(ptr(), fp);
    }

    int fgetline(FILE* fp) {
        return dstr_fgetline(ptr(), fp);
    }

private:
    void init() {
        dstr_init_data(ptr());
    }

    const DSTR_IMP* cptr() const { return &m_imp; }
    DSTR_IMP* ptr()              { return &m_imp; }

    DSTR_IMP m_imp;
};


#endif
