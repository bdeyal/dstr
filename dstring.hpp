#ifndef DSTRING_H_INCLUDED
#define DSTRING_H_INCLUDED

#include "dstr.h"

class DString {
public:
    static const size_t NPOS = DSTR_NPOS;
    static const bool IgnoreCase = true;

    // Constructors
    //
    DString()
        :
        p(dstr_create())
    {
    }

    explicit DString(size_t len)
        :
        p(dstr_create_reserve(len))
    {
    }

    DString(char c, size_t count)
        :
        p(dstr_create_cc(c, count))
    {
    }

    explicit DString(const char* sz)
        :
        p(dstr_create_sz(sz))
    {
    }

    DString(const DString& rhs)
        :
        p(dstr_create_ds(rhs.p))
    {
    }

#if __cplusplus >= 201103L
    DString(DString&& rhs)
    {
        p = rhs.p;
        rhs.p = dstr_create();
    }
#endif

    DString(const DString& rhs, size_t pos, size_t count)
        :
        p(dstr_create_substr(rhs.p, pos, count))
    {
    }

    DString(const char* buffer, size_t len)
        :
        p(dstr_create_bl(buffer, len))
    {
    }

    // Destructor
    //
    ~DString()
    {
        dstr_destroy(this->p);
    }

    // slurp a testfile into DString
    //
    int from_file(const char* fname)
    {
        if (dstr_assign_fromfile(this->p, fname))
            return DSTR_SUCCESS;

        return DSTR_FAIL;
    }

    // Move the C string. Caller must free()
    //
    char* move_data(size_t* plen = 0)
    {
        char* result = dstr_move_destroy(this->p, plen);
        p = dstr_create();
        return result;
    }

#if __cplusplus >= 201103L
    // Assignments operator and functions
    //
    DString& operator=(DString&& rhs) noexcept
    {
        if (&rhs != this)
            dstr_swap(p, rhs.p);
        return *this;
    }
#endif

    DString& operator=(const DString& rhs)
    {
        if (&rhs != this)
            dstr_assign_ds(this->p, rhs.p);
        return *this;
    }

    DString& operator=(const char* sz)
    {
        dstr_assign_sz(this->p, sz);
        return *this;
    }

    void assign(char c, size_t count)
    {
        dstr_assign_cc(this->p, c, count);
    }

    void assign(const char* buff, size_t len)
    {
        dstr_assign_bl(this->p, buff, len);
    }

    void assign(const DString& rhs, size_t pos, size_t count)
    {
        dstr_assign_substr(this->p, rhs.p, pos, count);
    }

    void sprintf(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        dstr_assign_vsprintf(this->p, fmt, args);
        va_end(args);
    }

    void vsprintf(const char* fmt, va_list args)
    {
        dstr_assign_vsprintf(this->p, fmt, args);
    }

    // Insertion functions
    //
    void insert(size_t pos, char c, size_t count)
    {
        dstr_insert_cc(this->p, pos, c, count);
    }

    void insert(size_t pos, const char* value)
    {
        dstr_insert_sz(this->p, pos, value);
    }

    void insert(size_t pos, const DString& rhs)
    {
        dstr_insert_ds(this->p, pos, rhs.p);
    }

    void insert(size_t pos, const char* buff, size_t len)
    {
        dstr_insert_bl(this->p, pos, buff, len);
    }

    // Append at the end
    //
    void append(char c)
    {
        dstr_append_c(this->p, c);
    }

    void push_back(char c)
    {
        append(c);
    }

    void pop_back()
    {
        chop();
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
        dstr_append_cc(this->p, c, count);
    }

    void append(const char* value)
    {
        dstr_append_sz(this->p, value);
    }

    void append(const DString& rhs)
    {
        dstr_append_ds(this->p, rhs.p);
    }

    void append(const char* buff, size_t len)
    {
        dstr_append_bl(this->p, buff, len);
    }

    void append_sprintf(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        dstr_append_vsprintf(this->p, fmt, args);
        va_end(args);
    }

    void append_vsprintf(const char* fmt, va_list args)
    {
        dstr_append_vsprintf(this->p, fmt, args);
    }

    // Replace functions
    //
    void replace(size_t pos, size_t len, char c, size_t count)
    {
        dstr_replace_cc(this->p, pos, len, c, count);
    }

    void replace(size_t pos, size_t len, const char* value)
    {
        dstr_replace_sz(this->p, pos, len, value);
    }

    void replace(size_t pos, size_t len, const DString& rhs)
    {
        dstr_replace_ds(this->p, pos, len, rhs.p);
    }

    void replace(size_t pos, size_t len, const char* buff, size_t bufflen)
    {
        dstr_replace_bl(this->p, pos, len, buff, bufflen);
    }

    // shrink
    //
    void resize(size_t len)
    {
        dstr_resize(this->p, len);
    }

    // Text manipulations (upper, lower etc)
    //
    void upper()
    {
        dstr_ascii_upper(this->p);
    }

    void lower()
    {
        dstr_ascii_lower(this->p);
    }

    void reverse()
    {
        dstr_reverse(this->p);
    }

    void trim_right()
    {
        dstr_trim_right(this->p);
    }

    void trim_left()
    {
        dstr_trim_left(this->p);
    }

    void trim()
    {
        dstr_trim_both(this->p);
    }

    void truncate()
    {
        dstr_truncate(this->p);
    }

    void erase(size_t pos, size_t count = 1)
    {
        dstr_remove(this->p, pos, count);
    }

    void chop()
    {
        dstr_chop(this->p);
    }

    void swap(DString& rhs)
    {
        dstr_swap(this->p, rhs.p);
    }

    size_t copy_substr(size_t pos, size_t numbytes, char dest[], size_t destsize)
    {
        return dstr_substr(this->p, pos, numbytes, dest, destsize);
    }

    // Inline fast queries
    //
    const char* c_str() const {
        return dstr_cstring(this->p);
    }

    size_t length() const {
        return dstr_length(this->p);
    }

    size_t size() const {
        return length();
    }

    size_t capacity() const {
        return dstr_capacity(this->p);
    }

    bool empty() const {
        return dstr_isempty(this->p);
    }

    bool index_ok(size_t pos) const {
        return dstr_valid_index(this->p, pos);
    }

    char getchar(long pos) const {
        return dstr_getchar_safe(this->p, pos);
    }

    void putchar(long pos, char c) {
        dstr_putchar_safe(this->p, pos, c);
    }

    char operator[](size_t pos) const {
        return dstr_getchar(this->p, pos);
    }

    char& operator[](size_t pos) {
        return DBUF(this->p)[pos];
    }

    unsigned int hash() const {
        return dstr_hash(this->p);
    }

    long atoi() const {
        return dstr_atoi(this->p);
    }

    long atoll() const {
        return dstr_atoll(this->p);
    }

    void itoa(long n) {
        dstr_itoa(this->p, n);
    }

    bool isdigits() const {
        return dstr_isdigits(this->p);
    }

    bool isxdigits() const {
        return dstr_isxdigits(this->p);
    }

    size_t find(char c, size_t pos=0) const {
        return dstr_find_c(this->p, pos, c);
    }

    size_t find(const char* sz, size_t pos=0) const {
        return dstr_find_sz(this->p, pos, sz);
    }

    size_t ifind(char c, size_t pos=0) const {
        return dstr_ifind_c(this->p, pos, c);
    }

    size_t ifind(const char*sz, size_t pos=0) const {
        return dstr_ifind_sz(this->p, pos, sz);
    }

    bool contains(const char* s) const {
        return dstr_contains_sz(this->p, s);
    }

    bool icontains(const char* s) const {
        return dstr_icontains_sz(this->p, s);
    }

    bool isblank() const {
        return dstr_isblank(this->p);
    }

    bool startswith(const char* s, bool ignore_case=false) const {
        return ignore_case ?
            dstr_iprefix_sz(this->p, s) :
            dstr_prefix_sz(this->p, s);
    }

    bool endswith(const char* s, bool ignore_case=false) const {
        return ignore_case ?
            dstr_isuffix_sz(this->p, s) :
            dstr_suffix_sz(this->p, s);
    }

    size_t ffo(const char* s, size_t pos = 0) const {
        return dstr_ffo_sz(this->p, pos, s);
    }

    size_t ffo(const DString& rhs, size_t pos = 0) const {
        return dstr_ffo_ds(this->p, pos, rhs.p);
    }

    size_t ffno(const char* s, size_t pos = 0) const {
        return dstr_ffno_sz(this->p, pos, s);
    }

    size_t ffno(const DString& rhs, size_t pos = 0) const {
        return dstr_ffno_ds(this->p, pos, rhs.p);
    }

    int compare(const char* sz) const {
        return dstr_compare_sz(this->p, sz);
    }

    int compare(const DString& rhs) const {
        return dstr_compare_ds(this->p, rhs.p);
    }

    bool operator==(const char* sz) const {
        return dstr_equal_sz(this->p, sz);
    }
    bool operator!=(const char* sz) const {
        return !dstr_equal_sz(this->p, sz);
    }
    bool operator<(const char* sz) const  { return (compare(sz) < 0);   }
    bool operator>(const char* sz) const  { return (compare(sz) > 0);   }
    bool operator<=(const char* sz) const { return (compare(sz) <= 0);  }
    bool operator>=(const char* sz) const { return (compare(sz) >= 0);  }

    bool operator==(const DString& rhs) const {
        return dstr_equal_ds(this->p, rhs.p);
    }
    bool operator!=(const DString& rhs) const {
        return !dstr_equal_ds(this->p, rhs.p);
    }
    bool operator<(const DString& rhs) const  { return (compare(rhs) < 0);  }
    bool operator>(const DString& rhs) const  { return (compare(rhs) > 0);  }
    bool operator<=(const DString& rhs) const { return (compare(rhs) <= 0); }
    bool operator>=(const DString& rhs) const { return (compare(rhs) >= 0); }

    bool iequal(const char* sz) const {
        return dstr_iequal_sz(this->p, sz);
    }

    int fgets(FILE* fp) {
        return dstr_fgets(this->p, fp);
    }

    int fgetline(FILE* fp) {
        return dstr_fgetline(this->p, fp);
    }

private:
    DSTR p;
};


#endif
