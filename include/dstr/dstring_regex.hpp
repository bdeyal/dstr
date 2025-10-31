#ifndef DSTRING_REGEXP_INCLUDED
#define DSTRING_REGEXP_INCLUDED

#include <vector>
#include <map>
#include <dstr/dstring.hpp>

class DStringRegex
{
public:
	enum Options
	{
		RE_CASELESS        = 0x00000001, /// case insensitive matching (/i) [ctor]
		RE_MULTILINE       = 0x00000002, /// enable multi-line mode; affects ^ and $ (/m) [ctor]
		RE_DOTALL          = 0x00000004, /// dot matches all characters, including newline (/s) [ctor]
		RE_EXTENDED        = 0x00000008, /// totally ignore whitespace (/x) [ctor]
		RE_ANCHORED        = 0x00000010, /// treat pattern as if it starts with a ^ [ctor, match]
		RE_DOLLAR_ENDONLY  = 0x00000020, /// dollar matches end-of-string only, not last newline in string [ctor]
		RE_EXTRA           = 0x00000040, /// enable optional PCRE functionality [ctor]
		RE_NOTBOL          = 0x00000080, /// circumflex does not match beginning of string [match]
		RE_NOTEOL          = 0x00000100, /// $ does not match end of string [match]
		RE_UNGREEDY        = 0x00000200, /// make quantifiers ungreedy [ctor]
		RE_NOTEMPTY        = 0x00000400, /// empty string never matches [match]
		RE_UTF8            = 0x00000800, /// assume pattern and subject is UTF-8 encoded [ctor]
		RE_NO_AUTO_CAPTURE = 0x00001000, /// disable numbered capturing parentheses [ctor, match]
		RE_NO_UTF8_CHECK   = 0x00002000, /// do not check validity of UTF-8 code sequences [match]
		RE_FIRSTLINE       = 0x00040000, /// an  unanchored  pattern  is  required  to  match
		                                 /// before  or  at  the  first  newline  in  the subject string,
		                                 /// though the matched text may continue over the newline [ctor]
		RE_DUPNAMES        = 0x00080000, /// names used to identify capturing  subpatterns need not be unique [ctor]
		RE_NEWLINE_CR      = 0x00100000, /// assume newline is CR ('\r'), the default [ctor]
		RE_NEWLINE_LF      = 0x00200000, /// assume newline is LF ('\n') [ctor]
		RE_NEWLINE_CRLF    = 0x00300000, /// assume newline is CRLF ("\r\n") [ctor]
		RE_NEWLINE_ANY     = 0x00400000, /// assume newline is any valid Unicode newline character [ctor]
		RE_NEWLINE_ANYCRLF = 0x00500000, /// assume newline is any of CR, LF, CRLF [ctor]
		RE_GLOBAL          = 0x10000000, /// replace all occurences (/g) [subst]
		RE_NO_VARS         = 0x20000000  /// treat dollar in replacement string as ordinary character [subst]
	};

	struct Match
	{
        // zero based offset (std::string::npos if subexpr does not match)
		size_t offset;

        // length of substring
		size_t length;

        // name of group
		DString name;
	};

	typedef std::vector<Match> MatchVector;

	DStringRegex(const DString& pattern, int options = 0);
	~DStringRegex();

	int match(const DString& subject, Match& mtch, int options = 0) const;

	int  match(const DString& subject, size_t offset, Match& m, int options = 0) const;
	int  match(const DString& subject, size_t offset, MatchVector& matches, int options = 0) const;
	bool match(const DString& subject, size_t offset = 0) const;
	bool match(const DString& subject, size_t offset, int options) const;
	int  extract(const DString& subject, DString& str, int options = 0) const;
	int  extract(const DString& subject, size_t offset, DString& str, int options = 0) const;
	int  split(const DString& subject, std::vector<DString>& strings, int options = 0) const;
	int  split(const DString& subject, size_t offset, std::vector<DString>& strings, int options = 0) const;
	int  subst(DString& subject, const DString& replacement, int options = 0) const;
	int  subst(DString& subject, size_t offset, const DString& replacement, int options = 0) const;
	static bool match(const DString& subject, const DString& pattern, int options = 0);

	bool operator == (const DString& subject) const;
	bool operator != (const DString& subject) const;

private:
	void* _pcre;  // Actual type is pcre2_code_8*
    typedef std::map<int, DString> GroupDict;
	GroupDict _groups;

private:
	size_t substOne(DString& subject, size_t offset, const DString& replacement, int options) const;
	static int compileOptions(int options);
	static int matchOptions(int options);

	DStringRegex();
	DStringRegex(const DStringRegex&);
	DStringRegex& operator=(const DStringRegex&);
};

// inlines
//
inline int DStringRegex::match(const DString& subject, Match& mtch, int options) const
{
	return match(subject, 0, mtch, options);
}


inline int DStringRegex::split(const DString& subject, std::vector<DString>& strings, int options) const
{
	return split(subject, 0, strings, options);
}


inline int DStringRegex::subst(DString& subject, const DString& replacement, int options) const
{
	return subst(subject, 0, replacement, options);
}


inline bool DStringRegex::operator == (const DString& subject) const
{
	return match(subject);
}


inline bool DStringRegex::operator != (const DString& subject) const
{
	return !match(subject);
}


#endif
