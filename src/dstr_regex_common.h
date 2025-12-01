/*
 * Part of this file is based on POCO C++ Libraries
 * Original work Copyright (c) 2006-2023, Applied Informatics Software Engineering GmbH.
 * Licensed under the Boost Software License, Version 1.0.
 *
 * Modifications and additional code:
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#ifndef DSTR_REGEX_COMMON_H
#define DSTR_REGEX_COMMON_H

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

// Constants for options (based on POCO)
//
enum {
    REGEX_CASELESS        = 0x00000001, /// case insensitive matching (/i) [ctor]
    REGEX_MULTILINE       = 0x00000002, /// enable multi-line mode; affects ^ and $ (/m) [ctor]
    REGEX_DOTALL          = 0x00000004, /// dot matches all characters, including newline (/s) [ctor]
    REGEX_EXTENDED        = 0x00000008, /// totally ignore whitespace (/x) [ctor]
    REGEX_ANCHORED        = 0x00000010, /// treat pattern as if it starts with a ^ [ctor, match]
    REGEX_DOLLAR_ENDONLY  = 0x00000020, /// dollar matches end-of-string only, not last newline in string [ctor]
    REGEX_EXTRA           = 0x00000040, /// enable optional PCRE functionality [ctor]
    REGEX_NOTBOL          = 0x00000080, /// circumflex does not match beginning of string [match]
    REGEX_NOTEOL          = 0x00000100, /// $ does not match end of string [match]
    REGEX_UNGREEDY        = 0x00000200, /// make quantifiers ungreedy [ctor]
    REGEX_NOTEMPTY        = 0x00000400, /// empty string never matches [match]
    REGEX_UTF8            = 0x00000800, /// assume pattern and subject is UTF-8 encoded [ctor]
    REGEX_NO_AUTO_CAPTURE = 0x00001000, /// disable numbered capturing parentheses [ctor, match]
    REGEX_NO_UTF8_CHECK   = 0x00002000, /// do not check validity of UTF-8 code sequences [match]
    REGEX_FIRSTLINE       = 0x00040000, /// an  unanchored  pattern  is  required  to  match
    REGEX_DUPNAMES        = 0x00080000, /// names used to identify capturing  subpatterns need not be unique [ctor]
    REGEX_NEWLINE_CR      = 0x00100000, /// assume newline is CR ('\r'), the default [ctor]
    REGEX_NEWLINE_LF      = 0x00200000, /// assume newline is LF ('\n') [ctor]
    REGEX_NEWLINE_CRLF    = 0x00300000, /// assume newline is CRLF ("\r\n") [ctor]
    REGEX_NEWLINE_ANY     = 0x00400000, /// assume newline is any valid Unicode newline character [ctor]
    REGEX_NEWLINE_ANYCRLF = 0x00500000, /// assume newline is any of CR, LF, CRLF [ctor]
    REGEX_GLOBAL          = 0x10000000, /// replace all occurences (/g) [subst]
    REGEX_NO_VARS         = 0x20000000  /// treat dollar in replacement string as ordinary character [subst]
};

static int compile_options(int options)
{
    int pcre_opts = 0;

    if (options & REGEX_CASELESS)
        pcre_opts |= PCRE2_CASELESS;
    if (options & REGEX_MULTILINE)
        pcre_opts |= PCRE2_MULTILINE;
    if (options & REGEX_DOTALL)
        pcre_opts |= PCRE2_DOTALL;
    if (options & REGEX_EXTENDED)
        pcre_opts |= PCRE2_EXTENDED;
    if (options & REGEX_ANCHORED)
        pcre_opts |= PCRE2_ANCHORED;
    if (options & REGEX_DOLLAR_ENDONLY)
        pcre_opts |= PCRE2_DOLLAR_ENDONLY;
    if (options & REGEX_UNGREEDY)
        pcre_opts |= PCRE2_UNGREEDY;
    if (options & REGEX_UTF8)
        pcre_opts |= PCRE2_UTF | PCRE2_UCP;
    if (options & REGEX_NO_AUTO_CAPTURE)
        pcre_opts |= PCRE2_NO_AUTO_CAPTURE;
    if (options & REGEX_FIRSTLINE)
        pcre_opts |= PCRE2_FIRSTLINE;
    if (options & REGEX_DUPNAMES)
        pcre_opts |= PCRE2_DUPNAMES;

    return pcre_opts;
}
/*-------------------------------------------------------------------------------*/

static int match_options(int options)
{
    int pcre_opts = 0;

    if (options & REGEX_ANCHORED)
        pcre_opts |= PCRE2_ANCHORED;
    if (options & REGEX_NOTBOL)
        pcre_opts |= PCRE2_NOTBOL;
    if (options & REGEX_NOTEOL)
        pcre_opts |= PCRE2_NOTEOL;
    if (options & REGEX_NOTEMPTY)
        pcre_opts |= PCRE2_NOTEMPTY;
    if (options & REGEX_NO_AUTO_CAPTURE)
        pcre_opts |= PCRE2_NO_AUTO_CAPTURE;
    if (options & REGEX_NO_UTF8_CHECK)
        pcre_opts |= PCRE2_NO_UTF_CHECK;

    return pcre_opts;
}
/*-------------------------------------------------------------------------------*/

static int parse_regex_options(const char* str)
{
    if (!str || !*str)
        return 0;

    int opts = 0;
    for (; *str; ++str) {
        switch (*str) {
        case ' ':
        case '/': break;
        case 'g': opts |= REGEX_GLOBAL;     break;
        case 'i': opts |= REGEX_CASELESS;   break;
        case 'm': opts |= REGEX_MULTILINE;  break;
        case 's': opts |= REGEX_DOTALL;     break;
        case 'x': opts |= REGEX_EXTENDED;   break;
        case 'X': opts |= REGEX_EXTRA;      break;
        case 'U': opts |= REGEX_UNGREEDY;   break;
        case 'D': opts |= REGEX_DOLLAR_ENDONLY;  break;
        case 'd': opts |= REGEX_NOTEOL;          break;
        case 'E': opts |= REGEX_NOTEMPTY;        break;
        case 'n': opts |= REGEX_NO_AUTO_CAPTURE; break;
        case 'F': opts |= REGEX_FIRSTLINE;       break;
        case 'A': opts |= REGEX_ANCHORED;        break;
        case 't': opts |= REGEX_DUPNAMES;        break;
        case '$': opts |= REGEX_NO_VARS;         break;
        case '\n': opts |= REGEX_NEWLINE_LF;     break;
        case '\r': opts |= REGEX_NEWLINE_CR;     break;
        default:
            break;
        }
    }
    return opts;
}
/*-------------------------------------------------------------------------------*/


#endif
