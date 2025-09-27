#include <iostream>
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
