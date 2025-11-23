/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#if !defined(DSTRINGSTREAM_HPP_INCLUDED)
#define DSTRINGSTREAM_HPP_INCLUDED
#endif

#include <iostream>
#include <streambuf>
#include <dstr/dstring.hpp>

// Currently only for output
//
class DStringBuf : public std::streambuf {
public:
    explicit DStringBuf(DString& str) : str_(str) {
        reset_put_area();
    }

    DString&       str()       { return str_; }
    const DString& str() const { return str_; }

    void clear() {
        str_.clear();
        reset_put_area();
    }

protected:
    int_type overflow(int_type ch) {
        str_.push_back((char)ch);
        reset_put_area();
        return ch;
    }

    std::streamsize xsputn(const char* s, std::streamsize n) {
        if (n <= 0) return 0;
        str_.append(s, (size_t) n);
        reset_put_area();
        return n;
    }

    int sync() {
        return 0;
    }

private:
    void reset_put_area() {
        if (str_.empty()) {
            setp(nullptr, nullptr); }
        else {
            char* p = (char*)str_.data();
            setp(p, p + str_.capacity());
            pbump((int)str_.size()); }
    }

    DString& str_;
};
//----------------------------------------------
//----------------------------------------------

class DStringOut : public std::ostream {
    DStringBuf buf_;
    DString    str_;
public:
    DStringOut() : std::ostream(&buf_), buf_(str_) {}
    const DString& str() const noexcept { return str_; }
    void clear() { str_.clear(); buf_.clear(); }
};
//----------------------------------------------
//----------------------------------------------

// Slower that DString::to_string() but general for all types that
// have io with std::ostream
//
template <typename T>
inline DString to_dstring(const T& t)
{
    DStringOut out;
    out << t;
    return out.str();
}
//----------------------------------------------
