#include <iostream>
#include <algorithm>
#include <iterator>
#include <cassert>

#include <dstr/dstring.hpp>
#include <dstr/dstring_regex.hpp>

#if defined(__BORLANDC__) || (defined(_MSC_VER) && (_MSC_VER <= 1200))
#define TRACE_FN() printf("%d\n", __LINE__)
#else
#define TRACE_FN() printf("\nFUNCTION: %s()\n", __FUNCTION__)
#endif

using namespace std;

void test_replace()
{
    TRACE_FN();

    // Compile the regex pattern (case-sensitive by default)
    DStringRegex re(R"([\w\.-]+@[\w\.-]+\.\w+)");

    DString subject = "Contact me at user@example.com or admin@domain.org.";

    // Simple match check (anchored, non-empty)
    if (re.match(subject)) {
        cout << "Full string matches the pattern." << endl;
    } else {
        cout << "Full string does not match." << endl;
    }

    // Extract the first match
    DString extracted;
    int matches = re.extract(subject, extracted);
    if (matches > 0) {
        cout << "Extracted: " << extracted << endl;
    }

    // Replace all matches (global, mask emails with '***')
    DString replacement = "***";
    int replacements = re.subst(subject, replacement, DSTR_REGEX_GLOBAL);
    cout << "Replaced " << replacements << " occurrences: " << subject << endl;
}
//--------------------------------------------------------------------------------

void test_pattern_with_groups()
{
    TRACE_FN();

    // Pattern with groups: (username)@(domain)
    DStringRegex re(R"(([\w\.-]+)@([\w\.-]+\.\w+))", DSTR_REGEX_CASELESS);

    DString subject = "Emails: alice@foo.com, bob@bar.org, charlie@baz.net";

    // Find matches with groups starting from offset 8 ("Emails: ")
    //
    size_t offset = 8;
    for (;;) {
        RE_MatchVector matches;
        int n = re.match_all(subject, offset, matches);
        if (n == 0) break;

        cout << "Found " << n << " matches:" << endl;
        for (const auto& match : matches) {
            cout << "  Offset: " << match.offset << ", Length: " << match.length << " ==> ";
            cout << "\"" << subject.substr(match.offset, match.length) << "\"" << endl;
        }

        // // Extract with groups into a vector
        // std::vector<DString> parts;
        // re.split(subject, parts);
        // cout << "Split parts (captured groups):" << endl;
        // for (const auto& part : parts) {
        //     cout << "  - " << part << endl;
        // }

        // Replace with group reference: e.g., $1@$2 (preserves but could transform)
        // DString repl = "$1 [at] $2";  // Avoids raw '@' if needed
        // re.subst(subject, repl, DSTR_REGEX_GLOBAL);
        // cout << "After replacement: " << subject << endl;

        offset = matches[0].offset + matches[0].length;
    }
}
//--------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const RE_Match& m)
{
    out << "Match: \"" << m.name << "\", pos=" << m.offset << ", len=" << m.length;
    return out;
}

void test_extract_all()
{
    TRACE_FN();

    const char *pattern = "(?<date>(?<year>\\d{4})-(?<month>\\d{2})-(?<day>\\d{2}))";

    DStringRegex re(pattern);
    DString subject = "Today is 2025-10-29 and tomorrow is 2025-10-30";

    size_t offset = 0;
    for (;;) {
        RE_MatchVector matches;
        int n = re.match_all(subject, offset, matches);
        if (n == 0) break;

        for (const auto& m : matches) {
            cout << m << " ==> ";
            cout << subject.substr(m.offset, m.length) << endl;
        }

        offset = matches[0].offset + matches[0].length;
    }
}
//--------------------------------------------------------------------------------

void test_extract_numbers()
{
    TRACE_FN();

    const char *pattern = R"/(\d+)/";
    DStringRegex re(pattern);

    DString subject = "Today is 2025-10-29 and tomorrow is 2025-10-30";

    // Extract with groups into a vector
    std::vector<DString> parts;
    re.split(subject, parts);
    cout << "Split parts (captured groups):" << endl;
    for (const auto& part : parts) {
        cout << "...." << part << endl;
    }

    size_t offset = 0;
    for (;;) {
        RE_MatchVector matches;
        int n = re.match_all(subject, offset, matches);
        if (n == 0) break;

        for (const auto& m : matches) {
            cout << m << " ==> ";
            cout << subject.substr(m.offset, m.length) << endl;
        }

        offset = matches[0].offset + matches[0].length;
    }
}
//--------------------------------------------------------------------------------

void test_various()
{
    TRACE_FN();

    DStringRegex re1("[0-9]+");
    assert(re1.match("123"));
    assert(!re1.match("abc"));
    assert(re1.match("abc123", 3));

    RE_Match pos;
    re1.match("123", 0, pos);
    assert(pos.offset == 0 && pos.length == 3);

    re1.match("ab12de", 0, pos);
    assert(pos.offset == 2 && pos.length == 2);

    re1.match("abcd", 0, pos);
    assert(pos.offset == DString::NPOS);

    DStringRegex re2("([0-9]+) ([0-9]+)");
    RE_MatchVector v;

    re2.match_all("123 456", v);
    assert(v.size() == 3);
    assert(v[0].offset == 0 && v[0].length == 7);
    assert(v[1].offset == 0 && v[1].length == 3);
    assert(v[2].offset == 4 && v[2].length == 3);

    DString s;
    int n = re1.extract("123", s);
    assert(n == 1 && s == "123");

    n = re1.extract("ab12de", 0, s);
    assert(n == 1 && s == "12");

    n = re1.extract("abcd", 0, s);
    assert(n == 0 && s == "");

    std::vector<DString> vec;
    re2.split("123 456", 0, vec, DSTR_REGEX_GLOBAL);
    cout << "VEC size = " << vec.size() << endl;
    for (uint32_t i = 0; i < vec.size(); ++i)
        cout << "vec[" << i << "] = " << vec[i] << endl;

    assert(vec.size() == 3);
    assert(vec[0] == "123 456");
    assert(vec[1] == "123");
    assert(vec[2] == "456");

    s = "123";
    re1.subst(s, "ABC");
    assert(s == "ABC");

    s = "123 456";
    re2.subst(s, "$2 $1");
    assert(s == "456 123");

    DStringRegex re3("ABC");
    DStringRegex re4("ABC", DSTR_REGEX_CASELESS);

    assert(!re3.match("abc"));
    assert(re4.match("abc"));
}
//--------------------------------------------------------------------------------


int main()
{
    try {
        test_replace();
        test_pattern_with_groups();
        test_extract_all();
        test_extract_numbers();
        test_various();
    }
    catch (const std::exception& ex) {
        cerr << "*** Error: " << ex.what() << endl;
    }
}
//--------------------------------------------------------------------------------
