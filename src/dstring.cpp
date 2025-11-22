/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#include <iostream>
#include <cctype>
#include <cerrno>
#include <dstr/dstring.hpp>
#include <dstr/dstring_view.hpp>
#include "dstr_internal.h"

// Memory management code installs a OOM handler
//
namespace {

void dstring_oom_error(void) {
    throw DStringError("Out of Memory");
}

struct Init_OOM_Handler {
    Init_OOM_Handler() {
        g_dstr_oom_handler = &dstring_oom_error;
    }
};

static Init_OOM_Handler init_oom_handler;

}
//-----------------------------------------------------------

/////////////////////////////////////////////////////////
//
//   DStringView implementation
//
/////////////////////////////////////////////////////////
// DStringView and iostream (covers DString operator << too)
//
std::ostream& operator<<(std::ostream& out, DStringView sv)
{
    out.write(sv.data(), sv.size());
    return out;
}
//----------------------------------------------------------------

void DStringView::split(char sep, std::vector<DString>& dest) const
{
    std::vector<DString> v;
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

void DStringView::split(const char* sep, std::vector<DString>& dest) const
{
    if (!sep)
        return;

    size_t start = 0;
    size_t sep_len = strlen(sep);
    std::vector<DString> v;

    for (;;) {
        size_t pos = find(sep, start);
        v.push_back(substr(start, pos - start));
        if (pos == NPOS)
            break;
        start = pos + sep_len;;
    }

    dest.swap(v);
}
//-----------------------------------------------------------

void DStringView::tokenize(const char* pattern, std::vector<DString>& dest) const
{
    std::vector<DString> tmp;

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

void DStringView::partition(const char* s, DString& left, DString& middle, DString& right) const
{
    struct DSTR_PartInfo pinfo;
    dstr_partition(pImp(), s, &pinfo);

    left   = substr(pinfo.l_pos, pinfo.l_len);
    middle = substr(pinfo.m_pos, pinfo.m_len);
    right  = substr(pinfo.r_pos, pinfo.r_len);
}
//-----------------------------------------------------------

void DStringView::rpartition(const char* s, DString& left, DString& middle, DString& right) const
{
    struct DSTR_PartInfo pinfo;
    dstr_rpartition(pImp(), s, &pinfo);

    left   = substr(pinfo.l_pos, pinfo.l_len);
    middle = substr(pinfo.m_pos, pinfo.m_len);
    right  = substr(pinfo.r_pos, pinfo.r_len);
}
//-----------------------------------------------------------

DString DStringView::substr(size_t pos, size_t len) const
{
    return DString(*this, pos, len);
}
//----------------------------------------------------------------

DString DStringView::left(size_t count) const
{
    return substr(0, count);
}
//----------------------------------------------------------------

DString DStringView::mid(size_t pos, size_t count) const
{
    return substr(pos, count);
}
//----------------------------------------------------------------

DString DStringView::right(size_t count) const
{
    return (size() <= count) ?
        DString(*this) :
        substr(size() - count, count);
}
//----------------------------------------------------------------

DString DStringView::expandtabs(size_t width) const
{
    DString result(*this);
    result.expandtabs_inplace(width);
    return result;
}
//----------------------------------------------------------------

DString DStringView::title() const
{
    DString result(*this);
    result.title_inplace();
    return result;
}
//----------------------------------------------------------------

DString DStringView::join(const std::vector<DString>& v) const
{
    DString result;
    result.join_inplace(*this, v);
    return result;
}
//----------------------------------------------------------------

DString DStringView::join(const char* argv[], size_t argc) const
{
    DString result;
    result.join_inplace(*this, argv, argc);
    return result;
}
//----------------------------------------------------------------

DString DStringView::times(size_t n) const
{
    DString result(*this);
    result.times_inplace(n);
    return result;
}
//----------------------------------------------------------------

/////////////////////////////////////////////////////////
//
//   DString implementation
//
//   Some functions delegate calls to above with view()
//
/////////////////////////////////////////////////////////

/*static*/
DString DString::from_file(const char* fname)
{
    DString result;
    if (!dstr_assign_fromfile(result.pImp(), fname)) {
        DString msg = DString::format("Could not open file: %s: %s\n",
                                      fname,
                                      strerror(errno));
        throw DStringError(std::move(msg));
    }
    return result;
}
//-----------------------------------------------------------

/*static*/
DString DString::from_cfile(FILE* fp)
{
    DString result;
    if (!dstr_slurp_stream(result.pImp(), fp)) {
        DString msg = DString::format("Could read file: %s\n",
                                      strerror(errno));
        throw DStringError(std::move(msg));
    }
    return result;
}
//-----------------------------------------------------------

/*static*/
DString DString::format(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    DString result;
    result.vsprintf(fmt, args);
    va_end(args);
    return result;
}
//-----------------------------------------------------------

DString& DString::sprintf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    dstr_assign_vsprintf(pImp(), fmt, args);
    va_end(args);
    return *this;
}
//-----------------------------------------------------------

DString& DString::append_sprintf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    dstr_append_vsprintf(pImp(), fmt, args);
    va_end(args);
    return *this;
}
//-----------------------------------------------------------

std::istream& operator>>(std::istream& in, DString& s)
{
    char c;

    /* skip blanks */
    while (in.get(c) && isspace(c))
        ;

    if (in) {
        char buf[32];
        size_t bindex = 0;

        s.clear();
        do {
            buf[bindex] = c;
            if (++bindex == sizeof(buf)) {
                s.append(buf, bindex);
                bindex = 0;
            }
        } while (in.get(c) && !isspace(c));

        if (bindex)
            s.append(buf, bindex);
    }

    if (in)
        in.putback(c);

    return in;
}
//-----------------------------------------------------------

std::istream& io_getline(std::istream& in, DString& s)
{
    char buf[128];
    size_t bindex = 0;

    s.clear();

    char c;
    while (in.get(c) && c != '\n') {
        buf[bindex] = c;
        if (++bindex == sizeof(buf)) {
            s.append(buf, bindex);
            bindex = 0;
        }
    }

    if (bindex)
        s.append(buf, bindex);

    return in;
}
//-----------------------------------------------------------

void DString::split(char sep, std::vector<DString>& dest) const
{
    view().split(sep, dest);
}
//-----------------------------------------------------------

void DString::split(const char* sep, std::vector<DString>& dest) const
{
    view().split(sep, dest);
}
//-----------------------------------------------------------

void DString::tokenize(const char* pattern, std::vector<DString>& dest) const
{
    view().tokenize(pattern, dest);
}
//-----------------------------------------------------------

void DString::partition(const char* s, DString& left, DString& middle, DString& right) const
{
    view().partition(s, left, middle, right);
}
//-----------------------------------------------------------

void DString::rpartition(const char* s, DString& left, DString& middle, DString& right) const
{
    view().rpartition(s, left, middle, right);
}
//-----------------------------------------------------------

DString& DString::join_inplace(DStringView sep, const std::vector<DString>& v)
{
    if (v.empty())
        return *this;

    typename std::vector<DString>::const_iterator p = v.begin();

    for (;;) {
        this->append(*p);
        if (++p == v.end()) break;
        this->append(sep);
    }

    return *this;
}
//-----------------------------------------------------------
