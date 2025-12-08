/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#include <iostream>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <functional>
#include <unordered_map>
#include <thread>

#include <dstr/dstring.hpp>


#if defined(__BORLANDC__) || (defined(_MSC_VER) && (_MSC_VER <= 1200))
#define TRACE_FN() printf("%d\n", __LINE__)
#else
#define TRACE_FN() printf("\nFUNCTION: %s()\n", __FUNCTION__)
#endif

std::ostream& operator<<(std::ostream& out, const DString::Match& m)
{
    out << "Match: \"" << m.name << "\", pos=" << m.offset << ", len=" << m.length;
    return out;
}
//--------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const std::vector<DString>& v)
{
    out << "[\"" << DString("\", \"").join(v) << "\"]";
    return out;
}
//--------------------------------------------------------------------------------

using namespace std;

void test_dstring_extract_numbers()
{
    TRACE_FN();

    const char *pattern = R"/((\d+)-(\d+)-(\d+))/";
    DString subject = "Today is 2025-10-29 and tomorrow is 2025-10-30";

    // Extract with groups into a vector
    std::vector<DString> parts;
    subject.capture(pattern, parts);

    cout << "Split parts (captured groups):" << endl;
    for (const auto& part : parts) {
        cout << "...." << part << endl;
    }

    size_t offset = 0;
    size_t match_group_count = 0;
    for (;;) {
        DString::MatchVector matches;
        int n = subject.match_groups(pattern, offset, matches);
        if (n == 0) break;

        ++match_group_count;
        for (const auto& m : matches) {
            cout << match_group_count << ") " << m << " ==> ";
            cout << subject.substr(m.offset, m.length) << endl;
        }

        offset = matches[0].offset + matches[0].length;
    }

    // Extract with groups into a vector
    std::vector<DString> v;
    DString("This is some string").capture("\\w+", v, "g");
    cout << v << endl;
}
//--------------------------------------------------------------------------------

void test_dstring_group_patterns()
{
    TRACE_FN();

    // Pattern with groups: (username)@(domain)
    const char* pattern = R"(([\w\.-]+)@([\w\.-]+\.\w+))";

    DString subject = "Emails: alice@foo.com, bob@bar.org, charlie@baz.net";

    // Find matches with groups starting from offset 8 ("Emails: ")
    //
    size_t offset = 8;
    for (;;) {
        DString::MatchVector matches;
        int n = subject.match_groups(pattern, offset, matches);
        if (n == 0) break;

        cout << "Found " << n << " matches:" << endl;
        for (const auto& match : matches) {
            cout << "  Offset: " << match.offset << ", Length: " << match.length << " ==> ";
            cout << "\"" << subject.substr(match.offset, match.length) << "\"" << endl;
        }

        offset = matches[0].offset + matches[0].length;
    }
}
//--------------------------------------------------------------------------------

void test_dstring_group_extract()
{
    TRACE_FN();

    const char *pattern = "(?<date>(?<year>\\d{4})-(?<month>\\d{2})-(?<day>\\d{2}))";
    DString subject = "Today is 2025-10-29 and tomorrow is 2025-10-30";

    size_t offset = 0;
    size_t match_group_count = 0;
    for (;;) {
        DString::MatchVector matches;
        int n = subject.match_groups(pattern, offset, matches);
        if (n == 0) break;

        ++match_group_count;
        for (const auto& m : matches) {
            cout << match_group_count << ") " << m << " ==> ";
            cout << subject.substr(m.offset, m.length) << endl;
        }

        offset = matches[0].offset + matches[0].length;
    }
}
//--------------------------------------------------------------------------------

void test_dstring_match()
{
    TRACE_FN();

    DString s = "123";
    assert(s.match("[0-9]+"));

    s = "abc";
    assert(!s.match("[0-9]+"));

    s = "abc123";
    assert(!s.match("[0-9]+"));
    assert(s.match("[0-9]+", 3));

    //s = "Emails: alice@foo.com, bob@bar.org, charlie@baz.net";
    s = "alice@foo.com";
    assert(s.match(R"(([\w\.-]+)@([\w\.-]+\.\w+))"));

    s = "true";
    assert(s.match("yes|on|true|1"));

    s = "true";
    assert(s.match("yes|on|true|1"));

    s = "yes";
    assert(s.match("yes|on|true|1"));

    s = "on";
    assert(s.match("yes|on|true|1"));

    s = "1";
    assert(s.match("yes|on|true|1"));

    s = "Hmmm...";
    assert(!s.match("yes|on|true|1"));
}
//--------------------------------------------------------------------------------

void test_dstring_match_contains()
{
    TRACE_FN();

    DString s = "123";
    assert(s.match_contains("[0-9]+") <= s.length());

    s = "abc";
    assert(s.match_contains("[0-9]+") == DString::NPOS);

    s = "abc123";
    assert(s.match_contains("[0-9]+") <= s.length());
    assert(!s.match("[0-9]+"));
    assert(s.match("[0-9]+", 3));

    s = "Emails: alice@foo.com, bob@bar.org, charlie@baz.net";
    assert(s.match_contains(R"(([\w\.-]+)@([\w\.-]+\.\w+))") <= s.length());
}
//--------------------------------------------------------------------------------

void test_dstring_capture()
{
    TRACE_FN();

    const char *pattern = R"/((\d+)-(\d+)-(\d+))/";
    DString subject = "Today is 2025-10-29 and tomorrow is 2025-10-30";

    // Extract with groups into a vector
    //
    std::vector<DString> parts;
    subject.capture(pattern, parts);
    assert(parts.size() == 4);
    size_t index = 0;
    assert(parts[index++] == "2025-10-29");
    assert(parts[index++] == "2025");
    assert(parts[index++] == "10");
    assert(parts[index++] == "29");

    subject = "123";
    DString s = subject.capture("[0-9]+");
    assert(s == "123");

    subject = "ab12de";
    s = subject.capture("[0-9]+");
    assert(s == "12");

    subject = "abcd";
    s = subject.capture("[0-9]+");
    assert(s == "");
}
//--------------------------------------------------------------------------------

void test_dstring_match_struct()
{
    TRACE_FN();

    DString::Match m;
    DString s = "123";
    s.match("[0-9]+", m);
    assert(m.offset == 0 && m.length == 3);

    s = "ab12de";
    s.match("[0-9]+", m);
    assert(m.offset == 2 && m.length == 2);

    s = "abcd";
    s.match("[0-9]+", m);
    assert(m.offset == DString::NPOS);

    const char* pattern = "([0-9]+) ([0-9]+)";
    DString::MatchVector v;
    s = "123 456";
    s.match_groups(pattern, v);
    assert(v.size() == 3);
    assert(v[0].offset == 0 && v[0].length == 7);
    assert(v[1].offset == 0 && v[1].length == 3);
    assert(v[2].offset == 4 && v[2].length == 3);

    std::vector<DString> vec;
    s.capture(pattern, vec);
    assert(vec.size() == 3);
    assert(vec[0] == "123 456");
    assert(vec[1] == "123");
    assert(vec[2] == "456");
}
//--------------------------------------------------------------------------------


DString dstring_replace_all(const DString& data,
                            const DString& pattern,
                            const std::function<DString(const DString&, const DString::MatchVector&)>& cb)
{
    DString result = data;
    DString::MatchVector matches;
    int offset = 0;

    while (result.match_groups(pattern, offset, matches)) {
        DString repl = cb(result, matches);
        result.replace(matches[0].offset, matches[0].length, repl);
        offset = matches[0].offset + repl.size();
    }

    return result;
}
//--------------------------------------------------------------------------------

void test_dstring_replace_all()
{
    TRACE_FN();

    DString tmplt = "Hello ${USER}! Your IP: ${IP}. Balance: ${BALANCE}";

    std::unordered_map<DString, DString, DStringHasher> env = {
        {"USER",    "alice"},
        {"IP",      "1.2.3.4"},
        {"BALANCE", "42.00"}
    };

    auto rendered = dstring_replace_all(
        tmplt,
        R"(\$\{([^}]+)\})",
        [&env](const DString& subject, const DString::MatchVector& mvec)->DString {
            assert(mvec.size() == 2);
            DString key = subject.substr(mvec[1].offset, mvec[1].length);
            auto it = env.find(key);
            return it == env.end() ? "N/A" : it->second;
        });

    cout << rendered.c_str() << endl;

    tmplt = "Emails: alice@foo.com, bob@bar.org, charlie@baz.net";
    rendered = dstring_replace_all(
        tmplt,
        R"(([\w\.-]+)@([\w\.-]+\.\w+))",
        [] (const DString& subject, const DString::MatchVector& mvec)->DString {
            assert(mvec.size() == 3);
            DString domain = subject.substr(mvec[2].offset, mvec[2].length);
            return "***@" + domain;
        });

    cout << rendered.c_str() << endl;
}
//--------------------------------------------------------------------------------

void test_dstring_subst()
{
    TRACE_FN();

    DString s = "123";
    cout << s.subst("[0-9]+", "ABC") << endl;
    assert(s.subst("[0-9]+", "ABC") == "ABC");

    s = "123 456";
    assert(s.subst("([0-9]+) ([0-9]+)", "$2 $1 $2") == "456 123 456");

    const char* pattern = R"(([\w\.-]+)@([\w\.-]+\.\w+))";
    DString emails = "Emails: alice@foo.com, bob@bar.org, charlie@baz.net";
    emails.subst_inplace(pattern, "***@$2", "/g");
    cout << emails << endl;

    DString hello = "hello";

    assert(hello.subst("[aeiou]", "*") == "h*llo");
    assert(hello.subst("[aeiou]", "*", "/g") == "h*ll*");
    assert(hello.subst("[aeiou]", "",  "/g") == "hll");
    assert(hello.subst("ell", "al") == "halo");
    assert(hello.subst("xyzzy", "*") == "hello");
    assert(DString("THX1138").subst("\\d+", "00") == "THX00");

    assert(DString("string    methods in C++").subst("\\s+", "-", "/g") == "string-methods-in-C++");

    s = "abc123def123ghi";
    s.subst_inplace("123", "XYZ", "/g");
    cout << s << endl;
    assert(s == "abcXYZdefXYZghi");
    assert(hello.subst("(?<vowel>[aeiou])", "${vowel}${vowel}", "/g") == "heelloo");
    assert(DString("aaaa").subst("a*", "X", "/g") == "XX");

    hello = "/this/IS/a/linux/path/name";
    hello.subst_inplace("/", "\\\\", "/g");
    puts(hello.data());
    hello.subst_inplace("IS", "is_NOT");
    puts(hello.data());
    assert(hello == "\\this\\is_NOT\\a\\linux\\path\\name");
}
//--------------------------------------------------------------------------------

bool isIP(const DString& s)
{
    return s.match(R"(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})");
}
//--------------------------------------------------------------------------------

void test_ip_address()
{
    TRACE_FN();

    assert(isIP("1.2.3.4"));
    assert(isIP("192.168.1.26"));
    assert(!isIP("1922.168.1.26"));
    assert(!isIP("192.1681.1.26"));
    assert(!isIP("192.168.1111.26"));
    assert(!isIP("192.168.1.2611"));
}
//--------------------------------------------------------------------------------

void test_pattern() try
{
    TRACE_FN();

    DString hello("hello");
    DString str = hello.capture("(a(b|c$");

    // If we are here - exception was not thrown
    //
    abort();
}
catch(DStringError& ex)
{
    cerr << "Execption cought as expeted: " << ex.what() << endl;
}
//--------------------------------------------------------------------------------

// To check that REGEX cache behaves nicely from different threads
//
void test_within_threads()
{
    std::vector<std::thread> threads;

    threads.emplace_back([] { test_dstring_match(); });
    threads.emplace_back([] { test_dstring_match_contains(); });
    threads.emplace_back([] { test_dstring_capture(); });
    threads.emplace_back([] { test_dstring_match_struct(); });
    threads.emplace_back([] { test_dstring_replace_all(); });
    threads.emplace_back([] { test_dstring_subst(); });
    threads.emplace_back([] { test_dstring_group_extract(); });
    threads.emplace_back([] { test_dstring_group_patterns(); });
    threads.emplace_back([] { test_dstring_extract_numbers(); });
    threads.emplace_back([] { test_ip_address(); });
    threads.emplace_back([] { test_pattern(); });

    for (auto& t : threads)
        t.join();
}
//--------------------------------------------------------------------------------

int main()
{
    try {
        test_dstring_match();
        test_dstring_match_contains();
        test_dstring_capture();
        test_dstring_match_struct();
        test_dstring_replace_all();
        test_dstring_subst();
        test_dstring_group_extract();
        test_dstring_group_patterns();
        test_dstring_extract_numbers();
        test_ip_address();
        test_pattern();

        test_within_threads();
    }
    catch (const std::exception& ex) {
        cerr << "*** Error: " << ex.what() << endl;
        return 1;
    }
}
//--------------------------------------------------------------------------------
