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
    while (in.get(c) && c != '\n')
    {
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
