#include <iostream>
#include <ctype.h>
#include <dstr/dstring.hpp>

DString& DString::sprintf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    dstr_assign_vsprintf(pImp(), fmt, args);
    va_end(args);
    return *this;
}
//-----------------------------------------------------------

DString& DString::vsprintf(const char* fmt, va_list args)
{
    dstr_assign_vsprintf(pImp(), fmt, args);
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

DString& DString::append_vsprintf(const char* fmt, va_list args)
{
    dstr_append_vsprintf(pImp(), fmt, args);
    return *this;
}
//-----------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const DString& s)
{
    out.write(s.c_str(), s.size());
    return out;
}
//-----------------------------------------------------------

std::istream& operator>>(std::istream& in, DString& s)
{
    char c;

    /* skip blanks */
    while (in.get(c) && isspace(c)) {
    }

    if (in) {
        s.truncate();
        do {
            s.append(c);
        } while (in.get(c) && !isspace(c));
    }

    if (in)
        in.putback(c);

    return in;
}
//-----------------------------------------------------------

std::istream& io_getline(std::istream& in, DString& s)
{
    char c;

    s.truncate();

    while (in.get(c) && c != '\n') {
        s += c;
    }

    return in;
}
//-----------------------------------------------------------

void DString::split(char sep, std::vector<DString>& dest) const
{
    std::vector<DString> tmp;
    DString str;

    for (char c : *this) {
        if (c == sep)
            tmp.push_back(std::move(str));
        else
            str.append(c);
    }

    if (!str.empty())
        tmp.push_back(std::move(str));

    dest.swap(tmp);
}
//-----------------------------------------------------------

void DString::split(const char* pattern, std::vector<DString>& dest) const
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
