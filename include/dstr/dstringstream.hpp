/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#if !defined(DSTRINGSTREAM_HPP_INCLUDED)
#define DSTRINGSTREAM_HPP_INCLUDED

#include <iostream>
#include <streambuf>
#include <dstr/dstring.hpp>

//  DString Buffer (<< operator)
//
//  Use this when:
//  1. You need to stringify a type that has operator<< but no std::formatter
//  2. You are on C++17 or earlier
//  3. You need to capture output from code that writes to std::ostream
//
//  Prefer DString::format() or DString::to_string() for built-in types.
//  Currently only for output (Unbuffered, slow)
//

class DStringBuf : public std::streambuf {
public:
    explicit DStringBuf() : str_() {
        setp(nullptr, nullptr);
    }

    const DString& str() const { return str_; }

    void reset() {
        str_.clear();
    }

protected:
    int_type overflow(int_type ch) {
        if (traits_type::eq_int_type(ch, traits_type::eof())) {
            return traits_type::not_eof(ch); }
        str_.push_back((char)ch);
        return traits_type::not_eof(ch);
    }

    std::streamsize xsputn(const char* s, std::streamsize n) {
        if (n <= 0) return 0;
        size_t before = str_.size();
        str_.append(s, (size_t) n);
        return (std::streamsize)(str_.size() - before);
    }

    int sync() {
        return 0;
    }

private:
    DString str_;

    // Prevent copy and assign
    //
    DStringBuf(const DStringBuf&);
    DStringBuf& operator=(const DStringBuf&);
#if __cplusplus >= 201103L
    DStringBuf(DStringBuf&&) = delete;
    DStringBuf& operator=(DStringBuf&&) = delete;
#endif
};
//----------------------------------------------
//----------------------------------------------

class DStringOut : public std::ostream {
    DStringBuf buf_;
public:
    DStringOut() : std::ostream(&buf_), buf_() {}
    const DString& str() const noexcept { return buf_.str(); }
    void reset() { buf_.reset(); }
private:
    DStringOut(const DStringOut&);
    DStringOut& operator=(const DStringOut&);
#if __cplusplus >= 201103L
    DStringOut(DStringOut&&) = delete;
    DStringOut& operator=(DStringOut&&) = delete;
#endif
};
//----------------------------------------------
//----------------------------------------------

// Slower than DString::to_string() but general for all types that
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

#endif
