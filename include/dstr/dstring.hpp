/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#ifndef DSTRING_HPP_INCLUDED
#define DSTRING_HPP_INCLUDED

#include <iosfwd>
#include <vector>
#include <exception>

#if __cplusplus >= 201703L
#include <string_view>
#endif

#include <dstr/dstr.h>

// For DStringView. This is a helper header that should not
// be included directly
//
#include "dstring_view.hpp"

// A C++ wrapper around C DSTR_TYPE
//
class DString {
public:
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
            dstr_init_data(pImp()); }
        else {
            init_capacity(count);
            init_data(c, count);
            init_length(count); }
    }

    // DString s("A C string");
    //
    DString(const char* sz)
    {
        if (!sz) {
            dstr_init_data(pImp()); }
        else {
            size_t len = strlen(sz);
            init_capacity(len);
            init_data(sz, len);
            init_length(len); }
    }

    // DString s(other_DString);
    //
    DString(const DString& rhs)
    {
        if (rhs.size() == 0) {
            dstr_init_data(pImp()); }
        else {
            init_capacity(rhs.size());
            init_data(rhs.data(), rhs.size());
            init_length(rhs.size()); }
    }

#if __cplusplus >= 201103L
    DString(DString&& rhs)
    {
        // shallow copy data from rhs (+ sso_buffer fix if needed)
        //
        m_imp.length   = rhs.size();
        m_imp.capacity = rhs.capacity();
        if (rhs.capacity() == DSTR_INITIAL_CAPACITY) {
            memcpy(m_imp.sso_buffer, rhs.m_imp.sso_buffer, DSTR_INITIAL_CAPACITY);
            m_imp.data = m_imp.sso_buffer;  }
        else {
            m_imp.data = rhs.m_imp.data; }

        // put rhs in an intial state
        //
        dstr_init_data(&rhs.m_imp);
    }
#endif

    // DString sub_string(other_dstr, 5, 5);
    //
    DString(DStringView rhs, size_t pos, size_t count)
    {
        if (pos >= rhs.size() || count == 0) {
            dstr_init_data(pImp()); }
        else {
            if (count > rhs.size() - pos)
                count = rhs.size() - pos;
            init_capacity(count);
            init_data(rhs.data() + pos, count);
            init_length(count); }
    }

    // DString s("abcdefg", 3) -> "abc"
    //
    DString(const char* buffer, size_t len)
    {
        if (!buffer || (len = strnlen(buffer, len)) == 0) {
            dstr_init_data(pImp()); }
        else {
            init_capacity(len);
            init_data(buffer, len);
            init_length(len); }
    }

    // DString s(&str[5], &str[15]);
    //
    DString(const char* first, const char* last)
    {
        size_t len;

        if (!first ||
            (len = (last - first)) == 0 ||
            (len = strnlen(first, len)) == 0)
        {
            dstr_init_data(pImp());
            return;
        }

        init_capacity(len);
        init_data(first, len);
        init_length(len);
    }

    // Support DStringView (same code as copy ctor)
    //
    DString(DStringView sv)
    {
        if (sv.size() == 0) {
            dstr_init_data(pImp()); }
        else {
            init_capacity(sv.size());
            init_data(sv.data(), sv.size());
            init_length(sv.size()); }
    }

    // Static constructors
    //
    // * slurp a text file into a DString (throws)
    // * create from a printf-like format
    //
    static DString from_file(const char* fname);
    static DString from_cfile(FILE* fp);
    static DString format(const char* fmt, ...);

    // DstringView conversion
    //
    DStringView view() const noexcept
    {
        return DStringView(data(), size());
    }

    operator DStringView() const noexcept
    {
        return view();
    }

    // Support C++17 std::string_view
    //
#if __cplusplus >= 201703L
    operator std::string_view() const noexcept
    {
        return std::string_view(data(), size());
    }

    explicit DString(std::string_view sv)
    {
        if (sv.size() == 0) {
            dstr_init_data(pImp()); }
        else {
            init_capacity(sv.size());
            init_data(sv.data(), sv.size());
            init_length(sv.size()); }
    }
#endif

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
        // no op on s = s
        if (&rhs != this)
            assign(rhs);
        return *this;
    }

    DString& operator=(DStringView sv)
    {
        assign(sv);
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

    DString& assign(const char* sz)
    {
        dstr_assign_sz(pImp(), sz);
        return *this;
    }

    DString& assign(DStringView sv)
    {
        dstr_assign_ds(pImp(), sv.pImp());
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

    DString& assign(DStringView rhs, size_t pos, size_t count)
    {
        dstr_assign_substr(pImp(), rhs.pImp(), pos, count);
        return *this;
    }

    // Auto expanding as needed
    //
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

    DString& insert(size_t pos, const char* buff, size_t len)
    {
        dstr_insert_bl(pImp(), pos, buff, len);
        return *this;
    }

    DString& insert(size_t pos, DStringView rhs)
    {
        dstr_insert_ds(pImp(), pos, rhs.pImp());
        return *this;
    }

    DString& insert(size_t pos, const char* first, const char* last)
    {
        dstr_insert_range(pImp(), pos, first, last);
        return *this;
    }

    DString& align_center_inplace(size_t width, char fill = ' ')
    {
        if (length() < width)
            dstr_align_center(pImp(), width, fill);
        return *this;
    }

    DString& align_right_inplace(size_t width, char fill = ' ')
    {
        if (length() < width)
            dstr_align_right(pImp(), width, fill);
        return *this;
    }

    DString& align_left_inplace(size_t width, char fill = ' ')
    {
        if (length() < width)
            dstr_align_left(pImp(), width, fill);
        return *this;
    }

    DString align_center(size_t width, char fill = ' ') const
    {
        DString result(*this);
        result.align_center_inplace(width, fill);
        return result;
    }

    DString align_right(size_t width, char fill = ' ') const
    {
        DString result(*this);
        result.align_right_inplace(width, fill);
        return result;
    }

    DString align_left(size_t width, char fill = ' ') const
    {
        DString result(*this);
        result.align_left_inplace(width, fill);
        return result;
    }

    DString& zfill_inplace(size_t width)
    {
        dstr_zfill(pImp(), width);
        return *this;
    }

    DString zfill(size_t width) const
    {
        DString result(*this);
        result.zfill_inplace(width);
        return result;
    }

    DString& expandtabs_inplace(size_t width = 8)
    {
        dstr_expand_tabs(pImp(), width);
        return *this;
    }

    DString expandtabs(size_t width = 8) const
    {
        DString result(*this);
        result.expandtabs_inplace(width);
        return result;
    }

    DString& title_inplace()
    {
        dstr_title(pImp());
        return *this;
    }

    DString title() const
    {
        DString result(*this);
        result.title_inplace();
        return result;
    }

    void push_back(char c)
    {
        dstr_append_char(pImp(), c);
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

    void chomp()
    {
        rstrip_inplace(" \t\n\r");
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

    DString& append(DStringView rhs)
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

    DString& operator+=(DStringView ds) {
        return append(ds);
    }

    // Join inplace
    //
    DString& join_inplace(DStringView sep, const std::vector<DString>& v);

    DString& join_inplace(DStringView sep, const char* argv[], size_t argc)
    {
        dstr_join_ds(pImp(), sep.pImp(), argv, argc);
        return *this;
    }

    DString& join_inplace(DStringView sep, char* argv[], size_t argc)
    {
        return join_inplace(sep, (const char**)argv, argc);
    }

    // join
    //
    DString join(const std::vector<DString>& v) const
    {
        DString result;
        result.join_inplace(*this, v);
        return result;
    }

    DString join(const char* argv[], size_t argc) const
    {
        DString result;
        result.join_inplace(*this, argv, argc);
        return result;
    }

    DString& times_inplace(size_t n) {
        dstr_multiply(pImp(), n);
        return *this;
    }

    // duplicate *this n times
    //
    DString times(size_t n) const
    {
        DString result(*this);
        result.times_inplace(n);
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

    DString& replace(size_t pos, size_t len, const char* buff, size_t bufflen)
    {
        dstr_replace_bl(pImp(), pos, len, buff, bufflen);
        return *this;
    }

    DString& replace(size_t pos, size_t len, DStringView rhs)
    {
        dstr_replace_ds(pImp(), pos, len, rhs.pImp());
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

    DString& replace_all(DStringView oldstr,
                         DStringView newstr,
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

    DString& succ_inplace()
    {
        dstr_increment(pImp());
        return *this;
    }

    DString succ() const
    {
        DString res(*this);
        return res.succ_inplace();
    }

    // split: empty string between separator character or string
    //
    void split(char c, std::vector<DString>& dest) const
    {
        view().split(c, dest);
    }
    void split(const char* sep, std::vector<DString>& dest) const
    {
        view().split(sep, dest);
    }

    void split(DStringView sep, std::vector<DString>& dest)  const
    {
        split(sep.data(), dest);
    }

    void splitlines(std::vector<DString>& dest) const
    {
        split('\n', dest);
    }

    void tokenize(const char* separators, std::vector<DString>& dest) const
    {
        view().tokenize(separators, dest);
    }

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

    void partition(const char* s, DString& l, DString& m, DString& r) const
    {
        view().partition(s, l, m, r);
    }

    void rpartition(const char* s, DString& l, DString& m, DString& r) const
    {
        view().rpartition(s, l, m, r);
    }

    void partition(DStringView s, DString& l, DString& m, DString& r) const
    {
        partition(s.c_str(), l, m, r);
    }

    void rpartition(DStringView s, DString& l, DString& m, DString& r) const
    {
        rpartition(s.c_str(), l, m, r);
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
    DString& upper_inplace()
    {
        dstr_ascii_upper(pImp());
        return *this;
    }

    DString upper() const
    {
        DString r(*this);
        return r.upper_inplace();
    }

    DString& lower_inplace()
    {
        dstr_ascii_lower(pImp());
        return *this;
    }

    DString lower() const
    {
        DString r(*this);
        return r.lower_inplace();
    }

    DString& swapcase_inplace()
    {
        dstr_ascii_swapcase(pImp());
        return *this;
    }

    DString swapcase() const
    {
        DString r(*this);
        return r.swapcase_inplace();
    }

    DString& reverse_inplace()
    {
        dstr_reverse(pImp());
        return *this;
    }

    DString reverse() const
    {
        DString r(*this);
        return r.reverse_inplace();
    }

    DString& trim_right_inplace()
    {
        dstr_trim_right(pImp());
        return *this;
    }

    DString trim_right() const {
        DString r(*this);
        return r.trim_right_inplace();
    }

    DString& trim_left_inplace()
    {
        dstr_trim_left(pImp());
        return *this;
    }

    DString trim_left() const {
        DString r(*this);
        return r.trim_left_inplace();
    }

    DString& trim_inplace()
    {
        dstr_trim(pImp());
        return *this;
    }

    DString trim() const {
        DString r(*this);
        return r.trim_inplace();
    }

    DString& lstrip_inplace(char c)
    {
        dstr_lstrip_c(pImp(), c);
        return *this;
    }

    DString& lstrip_inplace(const char* sz)
    {
        dstr_lstrip_sz(pImp(), sz);
        return *this;
    }

    DString& rstrip_inplace(char c)
    {
        dstr_rstrip_c(pImp(), c);
        return *this;
    }

    DString& rstrip_inplace(const char* sz)
    {
        dstr_rstrip_sz(pImp(), sz);
        return *this;
    }

    DString& strip_inplace(char c)
    {
        return rstrip_inplace(c).lstrip_inplace(c);
    }

    DString& strip_inplace(const char* sz)
    {
        return rstrip_inplace(sz).lstrip_inplace(sz);
    }

    DString lstrip(char c) const
    {
        DString result(*this);
        return result.lstrip_inplace(c);
    }

    DString lstrip(const char* sz) const
    {
        DString result(*this);
        return result.lstrip_inplace(sz);
    }

    DString rstrip(char c) const
    {
        DString result(*this);
        return result.rstrip_inplace(c);
    }

    DString rstrip(const char* sz) const
    {
        DString result(*this);
        return result.rstrip_inplace(sz);
    }

    DString strip(char c) const
    {
        DString result(*this);
        return result.strip_inplace(c);
    }

    DString strip(const char* sz) const
    {
        DString result(*this);
        return result.strip_inplace(sz);
    }

    void clear()
    {
        dstr_clear(pImp());
    }

    DString& erase(size_t pos, size_t count = 1)
    {
        dstr_remove(pImp(), pos, count);
        return *this;
    }

    DString& remove_inplace(char c)
    {
        dstr_remove_char(pImp(), c);
        return *this;
    }

    DString& remove_any_inplace(DStringView selectors)
    {
        dstr_remove_any(pImp(), selectors.c_str());
        return *this;
    }

    DString& remove_prefix_inplace(DStringView prefix)
    {
        dstr_remove_prefix(pImp(), prefix.c_str());
        return *this;
    }

    DString& remove_suffix_inplace(DStringView suffix)
    {
        dstr_remove_suffix(pImp(), suffix.c_str());
        return *this;
    }

    DString& iremove_prefix_inplace(DStringView prefix)
    {
        dstr_iremove_prefix(pImp(), prefix.c_str());
        return *this;
    }

    DString& iremove_suffix_inplace(DStringView suffix)
    {
        dstr_iremove_suffix(pImp(), suffix.c_str());
        return *this;
    }

    DString remove(char c) const
    {
        DString res(*this);
        return res.remove_inplace(c);
    }

    DString remove_any(DStringView selectors) const
    {
        DString res(*this);
        return res.remove_any_inplace(selectors);
    }

    DString remove_prefix(DStringView prefix) const
    {
        DString res(*this);
        return res.remove_prefix_inplace(prefix);
    }

    DString remove_suffix(DStringView suffix) const
    {
        DString res(*this);
        return res.remove_suffix_inplace(suffix);
    }

    DString iremove_prefix(DStringView prefix) const
    {
        DString res(*this);
        return res.iremove_prefix_inplace(prefix);
    }

    DString iremove_suffix(DStringView suffix) const
    {
        DString res(*this);
        return res.iremove_suffix_inplace(suffix);
    }

    void swap(DString& rhs)
    {
        dstr_swap(pImp(), rhs.pImp());
    }

    size_t copy_substr(size_t pos, size_t numbytes, char dest[], size_t destsize)
    {
        return dstr_substr(pImp(), pos, numbytes, dest, destsize);
    }

    const char* c_str()    const noexcept { return m_imp.data;     }
    const char* data()     const noexcept { return m_imp.data;     }
    size_t      length()   const noexcept { return m_imp.length;   }
    size_t      capacity() const noexcept { return m_imp.capacity; }
    bool        empty()    const noexcept { return length() == 0;  }

    bool index_ok(size_t pos) const
    {
        return dstr_valid_index(pImp(), pos);
    }

    char get(long pos) const
    {
        return dstr_getchar_safe(pImp(), pos);
    }

    void put(long pos, char c)
    {
        dstr_putchar_safe(pImp(), pos, c);
    }

    char operator[](size_t pos) const
    {
        return m_imp.data[pos];
    }

    char& operator[](size_t pos)
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

    void itos(long long n)
    {
        dstr_itos(pImp(), n);
    }

    void itos_ul(unsigned long long n, unsigned int base = 10)
    {
        dstr_itos_ul(pImp(), n, base);
    }

    size_t find(char c, size_t pos=0) const
    {
        return dstr_find_c(pImp(), pos, c);
    }

    size_t find(const char* sz, size_t pos=0) const
    {
        return dstr_find_sz(pImp(), pos, sz);
    }

    size_t ifind(char c, size_t pos=0) const
    {
        return dstr_ifind_c(pImp(), pos, c);
    }

    size_t ifind(const char* sz, size_t pos=0) const
    {
        return dstr_ifind_sz(pImp(), pos, sz);
    }

    size_t rfind(char c, size_t pos = NPOS) const
    {
        return dstr_rfind_c(pImp(), pos, c);
    }

    size_t rfind(const char* sz, size_t pos = NPOS) const
    {
        return dstr_rfind_sz(pImp(), pos, sz);
    }

    size_t irfind(char c, size_t pos = NPOS) const
    {
        return dstr_irfind_c(pImp(), pos, c);
    }

    size_t irfind(const char* sz, size_t pos = NPOS) const
    {
        return dstr_irfind_sz(pImp(), pos, sz);
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
#if !defined(isascii)
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
    int compare(const char* sz)      const { return strcmp(data(), sz ? sz : ""); }
    int icompare(const char* sz)     const { return strcasecmp(data(), sz ? sz : ""); }
    int compare(DStringView rhs)     const { return strcmp(data(), rhs.data()); }
    int icompare(DStringView rhs)    const { return strcasecmp(data(), rhs.data()); }

    bool iequal(const char* sz)      const { return (icompare(sz) == 0);  }
    bool iequal(DStringView rhs)     const { return (icompare(rhs) == 0); }

    bool operator!=(const char* sz)  const { return (compare(sz) != 0);  }
    bool operator<(const char* sz)   const { return (compare(sz) < 0);   }
    bool operator>(const char* sz)   const { return (compare(sz) > 0);   }
    bool operator<=(const char* sz)  const { return (compare(sz) <= 0);  }
    bool operator>=(const char* sz)  const { return (compare(sz) >= 0);  }

    bool operator<(DStringView rhs)  const { return (compare(rhs) < 0);  }
    bool operator>(DStringView rhs)  const { return (compare(rhs) > 0);  }
    bool operator<=(DStringView rhs) const { return (compare(rhs) <= 0); }
    bool operator>=(DStringView rhs) const { return (compare(rhs) >= 0); }

    // micro optimization - check length before buffer
    //
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

    int fgets(FILE* fp)
    {
        return dstr_fgets(pImp(), fp);
    }

    int fgetline(FILE* fp)
    {
        return dstr_fgetline(pImp(), fp);
    }

    void fputline(FILE* fp)
    {
        fprintf(fp, "%s\n", data());
    }

    //////////////////////////////////////////////////////
    //
    // Conversions  Built in types <==> DString
    //
    //////////////////////////////////////////////////////
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

    // Built in types -> DString
    //
    static DString to_string(int val)
    {
        DString r;
        r.itos(val);
        return r;
    }

    static DString to_string(unsigned int val)
    {
        DString r;
        r.itos_ul(val);
        return r;
    }

    static DString to_string(long val)
    {
        DString r;
        r.itos(val);
        return r;
    }

    static DString to_string(unsigned long val)
    {
        DString r;
        r.itos_ul(val);
        return r;
    }

    static DString to_string(long long val)
    {
        DString r;
        r.itos(val);
        return r;
    }

    static DString to_string(unsigned long long val)
    {
        DString r;
        r.itos_ul(val);
        return r;
    }

    static DString to_string(float val)
    {
        DString r;
        r.append_sprintf("%f", val);
        return r;
    }

    static DString to_string(double val)
    {
        DString r;
        r.append_sprintf("%g", val);
        return r;
    }

    static DString to_string(long double val)
    {
        DString r;
        r.append_sprintf("%Lf", val);
        return r;
    }

    static void puts(DStringView vw)
    {
        ::puts(vw.c_str());
    }

    // C++ algorithms support : functions
    //
    size_type      size()  const { return length(); }
    iterator       begin()       { return m_imp.data; }
    const_iterator begin() const { return m_imp.data; }
    iterator       end()         { return &m_imp.data[m_imp.length]; }
    const_iterator end()   const { return &m_imp.data[m_imp.length]; }

#if !defined(NO_DSTRING_REGEX)
    ///////////////////////////////////////////////////
    //
    //   Regexp Support
    //
    ///////////////////////////////////////////////////

    // Regexp Match type
    //
    typedef DSTR_Regex_Match  Match;
    typedef DSMatchVector     MatchVector;

    // *this is an exact match
    //
    bool match(DStringView pattern, size_t offset = 0) const;

    // pattern appears somewhere in *this. Returns position or NPOS
    //
    size_t match_contains(DStringView pattern, size_t offset = 0) const;

    // store match info in Match parameter
    //
    int match(DStringView ptrn,
              size_t offset, Match& m,
              const char* opts = nullptr) const;

    int match(DStringView pattern, Match& mtch, const char* options = nullptr) const
    {
        return match(pattern, 0, mtch, options);
    }

    // match groups into a vector of matches
    //
    int match_groups(DStringView pattern, size_t offset,
                     MatchVector& matches,
                     const char* options = nullptr) const;

    int match_groups(DStringView pattern,
                     MatchVector& matches,
                     const char* options = nullptr) const
    {
        return match_groups(pattern, 0, matches, options);
    }

    // capture - returns matches as strings or vector of strings
    //
    DString capture(DStringView pattern,
                    size_t offset,
                    const char* options = nullptr) const;

    DString capture(DStringView pattern,
                    const char* options = nullptr) const
    {
        return capture(pattern, 0, options);
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

    // substitute
    //
    int subst_inplace(DStringView pattern, size_t offset,
                      DStringView replacement,
                      const char* options = nullptr);

    int subst_inplace(DStringView pattern,
                      DStringView r,
                      const char* options = nullptr)
    {
        return subst_inplace(pattern, 0, r, options);
    }

    DString subst(DStringView pattern,
                  size_t offset,
                  DStringView replacement,
                  const char* options = nullptr) const
    {
        DString result(*this);
        result.subst_inplace(pattern, offset, replacement, options);
        return result;
    }

    DString subst(DStringView pattern,
                  DStringView r,
                  const char* options = nullptr) const
    {
        return subst(pattern, 0, r, options);
    }
#endif // NO_DSTRING_REGEX

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

    void init_data(char c, size_t count)
    {
        memset(m_imp.data, c, count);
    }

    void init_data(const char* p, size_t count)
    {
        memcpy(m_imp.data, p, count);
    }

    void init_length(size_t count)
    {
        m_imp.length = count;
        m_imp.data[count] = '\0';
    }
};
/*-------------------------------------------------------------------------------*/

// For use with std::unordered_map
//
struct DStringHasher {
    size_t operator()(DStringView d) const { return d.hash(); }
};

// for use with std::map + no case comparisons
//
struct DString_NoCase {
    bool operator()(DStringView s1, DStringView s2) const {
        return s1.icompare(s2) < 0;
    }
};
/*-------------------------------------------------------------------------------*/

class DStringError : public std::exception
{
public:
#if __cplusplus >= 201103L
    DStringError(DString&& s) : m_s(s) {}
#endif
    DStringError(DStringView s) : m_s(s) {}
    DStringError(const char* s) : m_s(s) {}
    const char* what() const noexcept { return m_s.c_str(); }
private:
    DString m_s;
};
/*-------------------------------------------------------------------------------*/

// various operator+()
//
inline DString operator+(DStringView lhs, DStringView rhs)
{
    DString result(lhs);
    result.append(rhs);
    return result;
}

inline DString operator+(DStringView lhs, char ch)
{
    DString result(lhs);
    result.append(ch);
    return result;
}

inline DString operator+(char ch, DStringView rhs)
{
    DString result(ch, 1);
    result.append(rhs);
    return result;
}

inline DString operator+(DStringView lhs, const char* sz)
{
    DString result(lhs);
    result.append(sz);
    return result;
}

inline DString operator+(const char* sz, DStringView rhs)
{
    DString result(sz);
    result.append(rhs);
    return result;
}
//----------------------------------------------------------------

// DString and iostream
//
std::ostream& operator<<(std::ostream& out, DStringView s);
std::istream& operator>>(std::istream& in, DString& s);
std::istream& io_getline(std::istream& in, DString& s);
//----------------------------------------------------------------

//////////////////////////////////////////////////////
//
//  DStringView inline functions that can be defined
//  only after DString type is fully known.
//
//////////////////////////////////////////////////////
inline DString DStringView::substr(size_t pos, size_t len) const
{
    return DString(*this, pos, len);
}
//----------------------------------------------------------------

inline DString DStringView::left(size_t count) const
{
    return substr(0, count);
}
//----------------------------------------------------------------

inline DString DStringView::mid(size_t pos, size_t count) const
{
    return substr(pos, count);
}
//----------------------------------------------------------------

inline DString DStringView::right(size_t count) const
{
    return (size() <= count) ?
        DString(*this) :
        substr(size() - count, count);
}
//----------------------------------------------------------------

inline DString DStringView::expandtabs(size_t width) const
{
    DString result(*this);
    result.expandtabs_inplace(width);
    return result;
}
//----------------------------------------------------------------

inline DString DStringView::title() const
{
    DString result(*this);
    result.title_inplace();
    return result;
}
//----------------------------------------------------------------

inline DString DStringView::join(const std::vector<DString>& v) const
{
    DString result;
    result.join_inplace(*this, v);
    return result;
}
//----------------------------------------------------------------

inline DString DStringView::join(const char* argv[], size_t argc) const
{
    DString result;
    result.join_inplace(*this, argv, argc);
    return result;
}
//----------------------------------------------------------------

inline DString DStringView::times(size_t n) const
{
    DString result(*this);
    result.times_inplace(n);
    return result;
}
//----------------------------------------------------------------

inline DString DStringView::align_center(size_t width, char fill) const
{
    DString result(*this);
    result.align_center_inplace(width, fill);
    return result;
}
//----------------------------------------------------------------

inline DString DStringView::align_right(size_t width, char fill) const
{
    DString result(*this);
    result.align_right_inplace(width, fill);
    return result;
}
//----------------------------------------------------------------

inline DString DStringView::align_left(size_t width, char fill) const
{
    DString result(*this);
    result.align_left_inplace(width, fill);
    return result;
}
//----------------------------------------------------------------

inline DString DStringView::zfill(size_t width) const
{
    DString result(*this);
    result.zfill_inplace(width);
    return result;
}
//----------------------------------------------------------------

inline DString DStringView::lstrip(char c) const
{
    DString result(*this);
    return result.lstrip_inplace(c);
}
//----------------------------------------------------------------

inline DString DStringView::lstrip(const char* sz) const
{
    DString result(*this);
    return result.lstrip_inplace(sz);
}
//----------------------------------------------------------------

inline DString DStringView::rstrip(char c) const
{
    DString result(*this);
    return result.rstrip_inplace(c);
}
//----------------------------------------------------------------

inline DString DStringView::rstrip(const char* sz) const
{
    DString result(*this);
    return result.rstrip_inplace(sz);
}
//----------------------------------------------------------------

inline DString DStringView::strip(char c) const
{
    DString result(*this);
    return result.strip_inplace(c);
}
//----------------------------------------------------------------

inline DString DStringView::strip(const char* sz) const
{
    DString result(*this);
    return result.strip_inplace(sz);
}
//----------------------------------------------------------------

inline DString DStringView::upper() const
{
    DString r(*this);
    return r.upper_inplace();
}
//----------------------------------------------------------------

inline DString DStringView::lower() const
{
    DString r(*this);
    return r.lower_inplace();
}
//----------------------------------------------------------------

inline DString DStringView::swapcase() const
{
    DString r(*this);
    return r.swapcase_inplace();
}
//----------------------------------------------------------------

inline DString DStringView::reverse() const
{
    DString r(*this);
    return r.reverse_inplace();
}
//----------------------------------------------------------------

inline DString DStringView::trim_right() const
{
    DString r(*this);
    return r.trim_right_inplace();
}
//----------------------------------------------------------------

inline DString DStringView::trim_left() const
{
    DString r(*this);
    return r.trim_left_inplace();
}
//----------------------------------------------------------------

inline DString DStringView::trim() const
{
    DString r(*this);
    return r.trim_inplace();
}
//----------------------------------------------------------------

inline DString DStringView::succ() const
{
    DString r(*this);
    return r.succ_inplace();
}
//----------------------------------------------------------------

inline DString DStringView::remove(char c) const
{
    DString res(*this);
    return res.remove_inplace(c);
}
//----------------------------------------------------------------

inline DString DStringView::remove_any(DStringView selectors) const
{
    DString res(*this);
    return res.remove_any_inplace(selectors);
}
//----------------------------------------------------------------

inline DString DStringView::remove_prefix(DStringView prefix) const
{
    DString res(*this);
    return res.remove_prefix_inplace(prefix);
}
//----------------------------------------------------------------

inline DString DStringView::remove_suffix(DStringView suffix) const
{
    DString res(*this);
    return res.remove_suffix_inplace(suffix);
}
//----------------------------------------------------------------

inline DString DStringView::iremove_prefix(DStringView prefix) const
{
    DString res(*this);
    return res.iremove_prefix_inplace(prefix);
}
//----------------------------------------------------------------

inline DString DStringView::iremove_suffix(DStringView suffix) const
{
    DString res(*this);
    return res.iremove_suffix_inplace(suffix);
}
//----------------------------------------------------------------



// Include guard
//
#endif
