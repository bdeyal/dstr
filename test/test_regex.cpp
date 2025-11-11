#include <iostream>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <functional>
#include <unordered_map>

#include <dstr/dstring.hpp>
#include <dstr/dstring_regex.hpp>

#if defined(__BORLANDC__) || (defined(_MSC_VER) && (_MSC_VER <= 1200))
#define TRACE_FN() printf("%d\n", __LINE__)
#else
#define TRACE_FN() printf("\nFUNCTION: %s()\n", __FUNCTION__)
#endif

inline std::ostream& operator<<(std::ostream& out, const RE_Match& m)
{
    out << "Match: \"" << m.name << "\", pos=" << m.offset << ", len=" << m.length;
    return out;
}
//--------------------------------------------------------------------------------

using namespace std;

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
        int n = re.match_groups(subject, offset, matches);
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

void test_extract_all()
{
    TRACE_FN();

    const char *pattern = "(?<date>(?<year>\\d{4})-(?<month>\\d{2})-(?<day>\\d{2}))";
    // const char *pattern = R"(?<date>(?<year>\d{4})-(?<month>\d{2})-(?<day>\d{2}))";
    // const char *pattern = R"((\d{4})-(\d{2})-(\d{2}))";

    DStringRegex re(pattern);
    DString subject = "Today is 2025-10-29 and tomorrow is 2025-10-30";

    size_t offset = 0;
    size_t match_group_count = 0;
    for (;;) {
        RE_MatchVector matches;
        int n = re.match_groups(subject, offset, matches);
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

void test_extract_numbers()
{
    TRACE_FN();

    const char *pattern = R"/((\d+)-(\d+)-(\d+))/";
    DStringRegex re(pattern);

    DString subject = "Today is 2025-10-29 and tomorrow is 2025-10-30";

    // Extract with groups into a vector
    std::vector<DString> parts;
    re.capture(subject, parts);
    cout << "Split parts (captured groups):" << endl;
    for (const auto& part : parts) {
        cout << "...." << part << endl;
    }

    size_t offset = 0;
    size_t match_group_count = 0;
    for (;;) {
        RE_MatchVector matches;
        int n = re.match_groups(subject, offset, matches);
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

    DString s = re1.capture("123");
    assert(s == "123");

    s = re1.capture("ab12de");
    assert(s == "12");

    s = re1.capture("abcd");
    assert(s == "");

    DStringRegex re2("([0-9]+) ([0-9]+)");
    RE_MatchVector v;

    re2.match_groups("123 456", v);
    assert(v.size() == 3);
    assert(v[0].offset == 0 && v[0].length == 7);
    assert(v[1].offset == 0 && v[1].length == 3);
    assert(v[2].offset == 4 && v[2].length == 3);

    std::vector<DString> vec;
    re2.capture("123 456", vec, DSTR_REGEX_GLOBAL);
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

    DStringRegex re5(R"(([\w\.-]+)@([\w\.-]+\.\w+))");
    DString tmplt = "Emails: alice@foo.com, bob@bar.org, charlie@baz.net";
    re5.subst(tmplt, "***@$2", DSTR_REGEX_GLOBAL);
    cout << tmplt << endl;

}
//--------------------------------------------------------------------------------

DString replace_all(const DString& data,
                    const DString& pattern,
                    const std::function<DString(const DString&, const RE_MatchVector&)>& cb)
{
    DStringRegex re(pattern);
    DString result = data;
    RE_MatchVector matches;
    int offset = 0;

    while (re.match_groups(result, offset, matches)) {
        DString repl = cb(result, matches);
        result.replace(matches[0].offset, matches[0].length, repl);
        offset = matches[0].offset + repl.size();
    }

    return result;
}
//--------------------------------------------------------------------------------

void test_replace_all()
{
    TRACE_FN();

    DString tmplt = "Hello ${USER}! Your IP: ${IP}. Balance: ${BALANCE}";

    std::unordered_map<DString, DString, DStringHasher> env = {
        {"USER",    "alice"},
        {"IP",      "1.2.3.4"},
        {"BALANCE", "42.00"}
    };

    auto rendered = replace_all(
        tmplt,
        R"(\$\{([^}]+)\})",
        [&env](const DString& subject, const RE_MatchVector& mvec)->DString {
            assert(mvec.size() == 2);
            DString key = subject.substr(mvec[1].offset, mvec[1].length);
            auto it = env.find(key);
            return it == env.end() ? "N/A" : it->second;
        });

    cout << rendered.c_str() << endl;

    tmplt = "Emails: alice@foo.com, bob@bar.org, charlie@baz.net";
    rendered = replace_all(
        tmplt,
        R"(([\w\.-]+)@([\w\.-]+\.\w+))",
        [] (const DString& subject, const RE_MatchVector& mvec)->DString {
            assert(mvec.size() == 3);
            DString domain = subject.substr(mvec[2].offset, mvec[2].length);
            return "***@" + domain;
        });

    cout << rendered.c_str() << endl;
}
//--------------------------------------------------------------------------------

void test_dstring_re_match()
{
    TRACE_FN();

    DString s = "123";
    assert(s.match("[0-9]+"));

    s = "abc";
    assert(!s.match("[0-9]+"));

    s = "abc123";
    assert(!s.match("[0-9]+"));
    assert(s.match("[0-9]+", 3));

    cout << "***********************************************************" << endl;
    cout << "***********************************************************" << endl;
    cout << "***********************************************************" << endl;

    //s = "Emails: alice@foo.com, bob@bar.org, charlie@baz.net";
    s = "alice@foo.com";
    assert(s.match(R"(([\w\.-]+)@([\w\.-]+\.\w+))"));
}
//--------------------------------------------------------------------------------

void test_dstring_match_within()
{
    TRACE_FN();

    DString s = "123";
    assert(s.match_within("[0-9]+") <= s.length());

    s = "abc";
    assert(s.match_within("[0-9]+") == DString::NPOS);

    s = "abc123";
    assert(s.match_within("[0-9]+") <= s.length());
    assert(!s.match("[0-9]+"));
    assert(s.match("[0-9]+", 3));

    cout << "***********************************************************" << endl;
    cout << "***********************************************************" << endl;
    cout << "***********************************************************" << endl;

    s = "Emails: alice@foo.com, bob@bar.org, charlie@baz.net";
    assert(s.match_within(R"(([\w\.-]+)@([\w\.-]+\.\w+))") <= s.length());
}
//--------------------------------------------------------------------------------

void test_dstring_re_capture()
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

void test_dstring_re_match_struct()
{
    TRACE_FN();

    RE_Match m;
    DString s = "123";
    s.match("[0-9]+", m);
    assert(m.offset == 0 && m.length == 3);

    s = "ab12de";
    s.match("[0-9]+", m);
    assert(m.offset == 2 && m.length == 2);

    s = "abcd";
    s.match("[0-9]+", m);
    assert(m.offset == DString::NPOS);
}
//--------------------------------------------------------------------------------


DString dstring_replace_all(const DString& data,
                            const DString& pattern,
                            const std::function<DString(const DString&, const RE_MatchVector&)>& cb)
{
    DString result = data;
    RE_MatchVector matches;
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
        [&env](const DString& subject, const RE_MatchVector& mvec)->DString {
            assert(mvec.size() == 2);
            DString key = subject.substr(mvec[1].offset, mvec[1].length);
            auto it = env.find(key);
            return it == env.end() ? "N/A" : it->second;
        });

    cout << rendered.c_str() << endl;

    tmplt = "Emails: alice@foo.com, bob@bar.org, charlie@baz.net";
    rendered = replace_all(
        tmplt,
        R"(([\w\.-]+)@([\w\.-]+\.\w+))",
        [] (const DString& subject, const RE_MatchVector& mvec)->DString {
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
    emails.subst_inplace(pattern, "***@$2", DSTR_REGEX_GLOBAL);
    cout << emails << endl;

    DString hello = "hello";

    assert(hello.subst("[aeiou]", "*") == "h*llo");
    assert(hello.subst("[aeiou]", "*", DSTR_REGEX_GLOBAL) == "h*ll*");
    assert(hello.subst("[aeiou]", "",  DSTR_REGEX_GLOBAL) == "hll");
    assert(hello.subst("ell", "al") == "halo");
    assert(hello.subst("xyzzy", "*") == "hello");
    assert(DString("THX1138").subst("\\d+", "00") == "THX00");
}
//--------------------------------------------------------------------------------

int main()
{
    try {
        test_pattern_with_groups();
        test_extract_all();
        test_extract_numbers();
        test_various();
        test_replace_all();
        test_dstring_re_match();
        test_dstring_match_within();
        test_dstring_re_capture();
        test_dstring_re_match_struct();
        test_dstring_replace_all();
        test_dstring_subst();
    }
    catch (const std::exception& ex) {
        cerr << "*** Error: " << ex.what() << endl;
    }
}
//--------------------------------------------------------------------------------
