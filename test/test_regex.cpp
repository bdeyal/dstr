#include <iostream>
#include <algorithm>
#include <iterator>

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

int main()
{
    try {
        test_replace();
        test_pattern_with_groups();
        test_extract_all();
    }
    catch (const std::exception& ex) {
        cerr << "*** Error: " << ex.what() << endl;
    }
}
//--------------------------------------------------------------------------------
