#include <iostream>
#include <dstr/dstring.hpp>
#include <dstr/dstring_regex.hpp>

using namespace std;

#if 1
int main()
{
    try {
        // Pattern with groups: (username)@(domain)
        DStringRegex re(R"(([\w\.-]+)@([\w\.-]+\.\w+))", DStringRegex::RE_CASELESS);

        DString subject = "Emails: alice@foo.com, bob@bar.org, charlie@baz.net";

        // Find matches with groups starting from offset 8 ("Emails: ")
        //
        DStringRegex::MatchVector matches;
        int numMatches = re.match(subject, 8, matches);

        cout << "Found " << numMatches << " matches:" << endl;
        for (const auto& match : matches) {
            cout << "  Offset: " << match.offset << ", Length: " << match.length << endl;
        }

        // Extract with groups into a vector
        std::vector<DString> parts;
        re.split(subject, parts);
        cout << "Split parts (captured groups):" << endl;
        for (const auto& part : parts) {
            cout << "  - " << part << endl;
        }

        // Replace with group reference: e.g., $1@$2 (preserves but could transform)
        DString repl = "$1 [at] $2";  // Avoids raw '@' if needed
        re.subst(subject, repl, DStringRegex::RE_GLOBAL);
        cout << "After replacement: " << subject << endl;

    } catch (const std::exception& ex) {
        cerr << "Error: " << ex.what() << endl;
    }

    return 0;
}

#else
int main()
{
    try {
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
        int replacements = re.subst(subject, replacement, DStringRegex::RE_GLOBAL);
        cout << "Replaced " << replacements << " occurrences: " << subject << endl;

    }
    catch (const std::exception& ex) {
        cerr << "Standard Error: " << ex.what() << endl;
    }

    return 0;
}

#endif
