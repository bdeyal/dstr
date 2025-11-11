#ifndef DSTR_REGEX_FWD_H
#define DSTR_REGEX_FWD_H

// Constants for options
//
enum {
    DSTR_REGEX_CASELESS        = 0x00000001, /// case insensitive matching (/i) [ctor]
    DSTR_REGEX_MULTILINE       = 0x00000002, /// enable multi-line mode; affects ^ and $ (/m) [ctor]
    DSTR_REGEX_DOTALL          = 0x00000004, /// dot matches all characters, including newline (/s) [ctor]
    DSTR_REGEX_EXTENDED        = 0x00000008, /// totally ignore whitespace (/x) [ctor]
    DSTR_REGEX_ANCHORED        = 0x00000010, /// treat pattern as if it starts with a ^ [ctor, match]
    DSTR_REGEX_DOLLAR_ENDONLY  = 0x00000020, /// dollar matches end-of-string only, not last newline in string [ctor]
    DSTR_REGEX_EXTRA           = 0x00000040, /// enable optional PCRE functionality [ctor]
    DSTR_REGEX_NOTBOL          = 0x00000080, /// circumflex does not match beginning of string [match]
    DSTR_REGEX_NOTEOL          = 0x00000100, /// $ does not match end of string [match]
    DSTR_REGEX_UNGREEDY        = 0x00000200, /// make quantifiers ungreedy [ctor]
    DSTR_REGEX_NOTEMPTY        = 0x00000400, /// empty string never matches [match]
    DSTR_REGEX_UTF8            = 0x00000800, /// assume pattern and subject is UTF-8 encoded [ctor]
    DSTR_REGEX_NO_AUTO_CAPTURE = 0x00001000, /// disable numbered capturing parentheses [ctor, match]
    DSTR_REGEX_NO_UTF8_CHECK   = 0x00002000, /// do not check validity of UTF-8 code sequences [match]
    DSTR_REGEX_FIRSTLINE       = 0x00040000, /// an  unanchored  pattern  is  required  to  match
    DSTR_REGEX_DUPNAMES        = 0x00080000, /// names used to identify capturing  subpatterns need not be unique [ctor]
    DSTR_REGEX_NEWLINE_CR      = 0x00100000, /// assume newline is CR ('\r'), the default [ctor]
    DSTR_REGEX_NEWLINE_LF      = 0x00200000, /// assume newline is LF ('\n') [ctor]
    DSTR_REGEX_NEWLINE_CRLF    = 0x00300000, /// assume newline is CRLF ("\r\n") [ctor]
    DSTR_REGEX_NEWLINE_ANY     = 0x00400000, /// assume newline is any valid Unicode newline character [ctor]
    DSTR_REGEX_NEWLINE_ANYCRLF = 0x00500000, /// assume newline is any of CR, LF, CRLF [ctor]
    DSTR_REGEX_GLOBAL          = 0x10000000, /// replace all occurences (/g) [subst]
    DSTR_REGEX_NO_VARS         = 0x20000000  /// treat dollar in replacement string as ordinary character [subst]
};

struct RE_Match
{
    // zero based offset (std::string::npos if subexpr does not match)
    size_t offset;

    // length of substring
    size_t length;

    // name of group
    char name[64];
};

#endif
