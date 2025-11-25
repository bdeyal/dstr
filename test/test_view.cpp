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
#include <dstr/dstring.hpp>
#include <dstr/dstring_view.hpp>

#if defined(__BORLANDC__) || (defined(_MSC_VER) && (_MSC_VER <= 1200))
#define TRACE_FN() printf("%d\n", __LINE__)
#else
#define TRACE_FN() puts(__FUNCTION__)
#endif

#define TRACE_LN() printf("%d\n", __LINE__)
//-------------------------------------------------

using namespace std;

void test_ffo()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DStringView s1(longstr);

    size_t res = s1.ffo(" \t");
    assert(res == 4);

    res = s1.ffo(" \t", 5);
    assert(res == 12);

    res = s1.ffo(" \t", 1000);
    assert(res == DStringView::NPOS);

    DStringView rhs("Frdy");
    res = s1.ffo(rhs);
    assert(res == 3);

    DStringView comment("# comment");
    res = comment.ffo("#;");
    assert(res == 0);

    res = comment.ffo("XYZ");
    assert(res == DStringView::NPOS);

    DStringView emp = "";
    assert(emp.empty());
    assert(emp.ffo("X") == DStringView::NPOS);
}
//-------------------------------------------------

void test_various()
{
    TRACE_FN();

    DStringView sv("Hello DStringView");
    cout << sv << endl;

    assert(sv == "Hello DStringView");

    DString s1("Hi There");
    DStringView sv2 = s1;
    assert(sv2 == "Hi There");

    cout << sv2 << endl;
    assert(sv.istitle());
    assert(sv2.contains("There"));
    assert(sv2.icontains("hi"));
    assert(sv2.istartswith("Hi"));
    assert(sv2.endswith("There"));
    assert(sv2.iendswith("there"));
    assert(sv2.hash() == s1.hash());

    for (int i = 0; i < 1000; ++i) {
        long n = rand();
        n *= rand();
        if (i % 2) n = -n;
        DString nstr = DString::to_string(n);
        assert(nstr.stol() == n);

        DStringView sv3(nstr);
        assert(sv3.stol() == n);
    }

    DStringView sv4("0123456789");
    assert(sv4.isdigits());
    assert(sv4.isxdigits());

    // test iteration
    //
    for (char c : sv4) {
        cout << c << " ";
    }
    cout << endl;

    // test operator []
    //
    for (size_t i = 0; i < sv4.length(); ++i) {
        assert(sv4[i] == '0' + (int)i);
    }
}
//-------------------------------------------------

void test_flo()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DStringView s1(longstr);

    size_t res = s1.flo(" \t");
    assert(res == 21);

    res = s1.flo(" \t", 5);
    assert(res == 4);

    res = s1.flo("");
    assert(res == DSTR_NPOS);

    DStringView rhs("Frdy");
    res = s1.flo(rhs);
    assert(res == 27);

    DStringView f(" F");
    res = s1.flo(f);
    assert(res == 22);

    DStringView comment("# comment");
    res = comment.flo("#;");
    assert(res == 0);

    res = comment.flo("XYZ");
    assert(res == DSTR_NPOS);

    DStringView emp("");
    assert( comment.flo("X") == DSTR_NPOS );

#if 1
    DStringView path = "/path/to/a/directory/with/file.txt";
    DString ext(path.substr(1 + path.flo(".")));
    assert(ext == "txt");

    DString fname(path.substr(1+path.flo("/")));
    assert(fname == "file.txt");
#endif
}
//-------------------------------------------------

void test_ffno()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DStringView s1(longstr);

    size_t res = s1.ffno(" \t");
    assert(res == 0);

    res = s1.ffno("Godm ");
    assert(res == 7);

    res = s1.ffno("Godm ", 5);
    assert(res == 7);

    res = s1.ffno("Godm ", 8);
    assert(res == 8);

    DStringView rhs("Godmr ");
    res = s1.ffno(rhs);
    assert(res == 8);

    DStringView abc = "ABCACABCABCABC";
    res = abc.ffno("ABC");
    assert(res == DStringView::NPOS);

    res = abc.ffno("AB");
    assert(res == 2);

    // split a string
    DStringView s2(longstr);
    size_t first, last = 0;
    while ((first = s2.ffno(" \t", last)) != DStringView::NPOS) {
        last = s2.ffo(" \t", first);
        DString sub(s2, first, (last - first));
        printf("%s\n", sub.c_str());
    }
}
//-------------------------------------------------

void test_flno()
{
    TRACE_FN();

    const char* longstr = "ABCDEF_123456";
    DStringView s1(longstr);

    size_t res = s1.flno("0123456789");
    assert(res == 6);

    res = s1.flno("0123456789", 5);
    assert(res == 5);

    res = s1.flno("ABCDEF", 5);
    assert(res == DSTR_NPOS);
}
//-------------------------------------------------

void test_prefix()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DStringView s1(longstr);

    assert( s1.startswith("G"));
    assert( s1.startswith("Good"));
    assert( s1.startswith(longstr));
    assert( !s1.startswith("good"));

    assert( s1.istartswith("GOOD MORNING"));
    assert( s1.istartswith("GooD"));
    assert( s1.istartswith("GooD morNiNg Today is Friday"));
    assert( !s1.startswith("H"));
    assert( !s1.istartswith("H"));

    DStringView s2("Good");
    const char* longer_str = "GoodXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    assert(s2.startswith("Good"));
    assert(!s2.startswith(longer_str));
}
//-------------------------------------------------

void test_suffix()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DStringView s1(longstr);

    assert( s1.endswith("y"));
    assert( s1.endswith("day"));
    assert(!s1.endswith("D") );
    assert( s1.endswith(longstr));
    assert( s1.iendswith("GooD morNiNg Today is Friday"));

    assert( s1.iendswith("Y"));
    assert( s1.iendswith("DaY"));
    assert( !s1.endswith("H"));
    assert( !s1.iendswith("H"));
}
//-------------------------------------------------

void test_blank()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DStringView s1(longstr);

    assert( !s1.isblank() );
    TRACE_LN();

    DStringView bl(" \t");
    assert( bl.isblank() );

    DStringView bl2(" \t:");
    assert( !bl2.isblank() );

    DStringView bl3("         ");
    assert( bl3.isblank() );
}
//-------------------------------------------------

#define TEST_ATOI(src, result) do {                                \
        DStringView s(src);                                        \
        long n = s.atoi();                                         \
        printf("Converted = %ld, Expected = %ld\n", n, result);    \
        assert( n == result );                                     \
    } while (0)
//-------------------------------------------------

void test_atoi()
{
    TRACE_FN();

    TEST_ATOI("1", 1L);
    TEST_ATOI("\\1234", 668L);
    TEST_ATOI("0b111001", 57L);
    TEST_ATOI("0xFFFF", 65535L);
}
//-------------------------------------------------

#define TEST_DIGIT(s, result) do {                            \
        DStringView d(s);                                     \
        puts(s);                                              \
        assert( d.isdigits() == result );                     \
        assert( d.isnumeric() == result );                    \
        assert( d.isdecimal() == result );                    \
        assert( d.isdigits() == result );                     \
    } while (0)

#define TEST_XDIGIT(s, result) do {                         \
        DStringView d(s);                                   \
        assert( d.isxdigits() == result );                  \
    } while (0)
//-------------------------------------------------

void test_isdigit()
{
    TRACE_FN();

    TEST_DIGIT("1", DSTR_TRUE);
    TEST_DIGIT("ABC", DSTR_FALSE);
    TEST_DIGIT("XYZ", DSTR_FALSE);
    TEST_DIGIT("1XAYBZ2", DSTR_FALSE);
    TEST_DIGIT("1234567890", DSTR_TRUE);
    TEST_DIGIT("1234567890ABCDEF", DSTR_FALSE);

    TEST_XDIGIT("1",     DSTR_TRUE);
    TEST_XDIGIT("ABC", DSTR_TRUE);
    TEST_XDIGIT("XYZ", DSTR_FALSE);
    TEST_XDIGIT("1XAYBZ2", DSTR_FALSE);
    TEST_XDIGIT("1234567890", DSTR_TRUE);
    TEST_XDIGIT("1234567890ABCDEF", DSTR_TRUE);
}
//-------------------------------------------------

void test_is_identifier()
{
    assert(DStringView("MyFolder").isidentifier());
    assert(DStringView("Demo001").isidentifier());
    assert(DStringView("_Demo001").isidentifier());
    assert(!DStringView("4Demo001").isidentifier());
    assert(!DStringView("2bring").isidentifier());
    assert(!DStringView("my demo").isidentifier());
}
//-------------------------------------------------

void test_count()
{
    TRACE_FN();

    assert(DStringView("").count("") == 1);
    assert(DStringView("A").count("") == 2);
    assert(DStringView("ABC").count("") == 4);
    assert(DStringView("ppppp").count("pp") == 2);
    assert(DStringView("ppppp").count("ppp") == 1);

    assert(DStringView("").icount("") == 1);
    assert(DStringView("A").icount("") == 2);
    assert(DStringView("ABC").icount("") == 4);
    assert(DStringView("ppppp").icount("pp") == 2);
    assert(DStringView("ppppp").icount("ppp") == 1);
    assert(DStringView("ppppp").icount("PpP") == 1);

    assert(DStringView("").count(DStringView("")) == 1);
    assert(DStringView("ABC").count(DStringView("")) == 4);
    assert(DStringView("ppppp").count(DStringView("pp")) == 2);
    assert(DStringView("ppppp").count(DStringView("ppp")) == 1);

    assert(DStringView("Hello World").count("hell") == 0);
    assert(DStringView("Hello World").icount("hell") == 1);

    assert(DStringView("Hello World").count("RLD") == 0);
    assert(DStringView("Hello World").icount("RLD") == 1);
}
//-------------------------------------------------

void test_find()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DStringView s1(longstr);

    assert( s1.find("good") == DStringView::NPOS);
    assert( s1.find("Good") == 0);
    assert( s1.find("morning") == 5);
    assert( s1.find("morning", 6) == DStringView::NPOS);
    assert( s1.find("morning", 100) == DStringView::NPOS);
    assert( s1.find("day") == 15);
    assert( s1.find("date", 3) == DStringView::NPOS);

    assert( s1.contains("is"));
    assert( s1.contains("morn"));
    assert(!s1.contains("XXX"));

    assert( s1.ifind("good") == 0);
    assert( s1.ifind("good", 1) == DStringView::NPOS);
    assert( s1.ifind("GOOD") == 0);
    assert( s1.ifind("MoRnInG") == 5);
    assert( s1.ifind("MoRnInG", 6) == DStringView::NPOS);
    assert( s1.ifind("MoRnInG", 1000) == DStringView::NPOS);
    assert( s1.ifind("DAY", 8) == 15);
    assert( s1.ifind("Date") == DStringView::NPOS);

    assert( s1.icontains("IS"));
    assert( s1.icontains("MoRn"));
    assert(!s1.icontains("xXx"));

    assert( s1.find('g') == 11);
    assert( s1.find('g', 1) == 11);
    assert( s1.find('g', 2) == 11);
    assert( s1.find('g', 11) == 11);
    assert( s1.find('g', 12) == DStringView::NPOS);
    assert( s1.find('g', 100) == DStringView::NPOS);
    assert( s1.find('G') == 0);
    assert( s1.find('m') == 5);
    assert( s1.find('d', 1) == 3);
    assert( s1.find('X') == DStringView::NPOS);

    assert( s1.ifind('G') == 0);
    assert( s1.ifind('g') == 0);
    assert( s1.ifind('m') == 5);
    assert( s1.ifind('M', 3) == 5);
    assert( s1.ifind('f', 15) == 22);
    assert( s1.ifind('f', 100) == DStringView::NPOS);
    assert( s1.ifind('X') == DStringView::NPOS);
}
//-------------------------------------------------

void test_rfind()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DStringView s1(longstr);

    assert(s1.rfind("good") == DSTR_NPOS);
    assert(s1.rfind("Good") == 0);
    assert(s1.rfind("morning") == 5);
    assert(s1.rfind("morning", 4) == DSTR_NPOS);
    assert(s1.rfind("morning") == 5);
    assert(s1.rfind("day") == 25);
    assert(s1.rfind("date", 3) == DSTR_NPOS);

    assert(s1.irfind("good") == 0);
    assert(s1.irfind("GOOD") == 0);
    assert(s1.irfind("MoRnInG") == 5);
    assert(s1.irfind("MoRnInG", 2) == DSTR_NPOS);
    assert(s1.irfind("MoRnInG", 0) == DSTR_NPOS);
    assert(s1.irfind("DAY") == 25);
    assert(s1.irfind("DAY", 18) == 15);
    assert(s1.irfind("Date") == DSTR_NPOS);

    assert(s1.rfind('g') == 11);
    assert(s1.rfind('y') == 27);
    assert(s1.rfind('g', 2) == DSTR_NPOS);
    assert(s1.rfind('g', 11) == 11);
    assert(s1.rfind('g', 12) == 11);

    assert(s1.rfind('X') == DSTR_NPOS);

    assert( s1.irfind('G') == 11);
    assert( s1.irfind('g') == 11);
    assert( s1.irfind('m') == 5);
    assert( s1.irfind('M') == 5);
    assert( s1.irfind('f') == 22);
    assert( s1.irfind('f') == 22);
    assert( s1.irfind('X', 0) == DSTR_NPOS);

    DStringView s2 = "";
    assert(s2.irfind('X') == DSTR_NPOS);
    assert(s2.irfind("good") == DSTR_NPOS);
    assert(s2.rfind('X') == DSTR_NPOS);
    assert(s2.rfind("good") == DSTR_NPOS);

    assert(DStringView("hello").rfind("hello") == 0);
    assert(DStringView("hello").rfind("l") == 3);
    assert(DStringView("he@@@XXX@@@llo").rfind("@@@") == 8);
    assert(DStringView("he@@@XXX@@@llo").rfind("@@@", 9) == 8);
    assert(DStringView("hello").irfind("HeLlO") == 0);

    assert(DStringView("").rfind("") == 0);
    assert(DStringView("").rfind("XX") == DStringView::NPOS);
    assert(DStringView("XX").rfind("") == 2);

}
//-------------------------------------------------

size_t count_substrings(DStringView hive, DStringView bee)
{
    if (hive.empty() || bee.empty())
        return 0U;

    size_t buzz = 0;
    while (bee.size() <= hive.size())
    {
        auto pos = hive.find(bee);
        if (pos == hive.NPOS)
            break;
        ++buzz;

        hive.remove_prefix(pos + bee.size());
    }
    return buzz;
}
//-------------------------------------------------

void test_remove_profix()
{
    TRACE_FN();

    DString str   = "   trim me";
    DStringView v = str;
    v.remove_prefix(std::min(v.ffno(" "), v.size()));
    cout << "String: '" << str << "'\n"
         << "View  : '" << v << "'\n";

    DStringView hive{"bee buzz bee buzz bee"};
    cout << "There are " << count_substrings(hive, "bee") << " bees in this hive.\n";

    hive.remove_prefix(hive.size());
    assert(hive == "");
}
//-------------------------------------------------

void test_expandtabs()
{
    TRACE_FN();

    DStringView s1("\t");

    assert(s1.expandtabs() == "        ");
    assert(s1.expandtabs(1) == " ");
    assert(s1.expandtabs(2) == "  ");
    assert(s1.expandtabs(60) == DString(' ', 60));

    DStringView s2 = "Hello\tWorld\tToday\tIs\tSaturday";
    assert(s2.expandtabs()  == "Hello   World   Today   Is      Saturday");
    assert(s2.expandtabs(7) == "Hello  World  Today  Is     Saturday");
    assert(s2.expandtabs(6) == "Hello World Today Is    Saturday");
    assert(s2.expandtabs(5) == "Hello     World     Today     Is   Saturday");
    assert(s2.expandtabs(4) == "Hello   World   Today   Is  Saturday");
    assert(s2.expandtabs(3) == "Hello World Today Is Saturday");
    assert(s2.expandtabs(2) == "Hello World Today Is  Saturday");
    assert(s2.expandtabs(1) == "Hello World Today Is Saturday");
    assert(s2.expandtabs(0) == "HelloWorldTodayIsSaturday");
}
//-------------------------------------------------

void test_title()
{
    TRACE_FN();

    const char* s = "hello world today 33 is SAT";
    DStringView d1(s);

    cout << d1.title() << endl;
    assert(d1.title() == "Hello World Today 33 Is SAT");
    assert(d1.title().istitle());

    d1 = "Welcome to my 2nd world";
    cout << d1.title() << endl;
    assert(d1.title() == "Welcome To My 2Nd World");
    assert(d1.title().istitle());

    d1 = "hello b2b2b2 and 3g3g3g";
    cout << d1.title() << endl;
    assert(d1.title() == "Hello B2B2B2 And 3G3G3G");
    assert(d1.title().istitle());

    d1 = "hello b2bb2b2 a.n.d 3g3g3g";
    cout << d1.title() << endl;
    assert(d1.title() == "Hello B2Bb2B2 A.N.D 3G3G3G");
    assert(d1.title().istitle());

    d1 = "hello b2b2b2 a_n_d 3g3g3g";
    cout << d1.title() << endl;
    assert(d1.title() == "Hello B2B2B2 A_N_D 3G3G3G");
    assert(d1.title().istitle());
}
//-------------------------------------------------

void test_join()
{
    TRACE_FN();

    const char* argv[] = {"hello", "world", "good", "morning", NULL};
    size_t argc = 4;

    DStringView s1("...");
    assert(s1.join(argv, argc) == "hello...world...good...morning");

    assert(s1.join(argv, 100) == "hello...world...good...morning");

    assert(s1.join(argv, 2) == "hello...world");

    DStringView sep("+++");
    assert(sep.join(argv, argc) == "hello+++world+++good+++morning");

#if __cplusplus >= 201103L
    std::vector<DString> v{"hello", "world", "good", "morning"};
#else
    std::vector<DString> v;
    v.push_back("hello");
    v.push_back("world");
    v.push_back("good");
    v.push_back("morning");
#endif
    assert(s1.join(v) == "hello...world...good...morning");
    assert(sep.join(v) == "hello+++world+++good+++morning");

#if __cplusplus >= 201103L
    assert(sep.join(std::vector<DString>{"Hi", "Eyal"}) == "Hi+++Eyal");
#endif
}
//-------------------------------------------------

void test_split()
{
    TRACE_FN();

    std::vector<DString> dest;

    DStringView s2 = "  Hello,World,,,\t\tToday is Tuesday 1.1.1.1, 2.2.2.2,33.33.33";

    s2.tokenize(", \t", dest);
    assert(dest.size() == 8);
    assert(dest[0] == "Hello");
    assert(dest[1] == "World");
    assert(dest[6] == "2.2.2.2");
    assert(dest[7] == "33.33.33");
    copy(dest.begin(), dest.end(), ostream_iterator<DString>(cout, "\n"));

    DStringView s3("Hello\nWorld\n\nGood\n\nMorning");
    s3.splitlines(dest);
    assert(dest.size() == 6);
    int i = 0;
    assert(dest[i++] == "Hello");
    assert(dest[i++] == "World");
    assert(dest[i++] == "");
    assert(dest[i++] == "Good");
    assert(dest[i++] == "");
    assert(dest[i++] == "Morning");
    copy(dest.begin(), dest.end(), ostream_iterator<DString>(cout, "\n"));

    dest.clear();
    DStringView s4 = "Hello:World::Good::Morning";
    s4.split(':', dest);
    assert(dest.size() == 6);
    i = 0;
    assert(dest[i++] == "Hello");
    assert(dest[i++] == "World");
    assert(dest[i++] == "");
    assert(dest[i++] == "Good");
    assert(dest[i++] == "");
    assert(dest[i++] == "Morning");
    copy(dest.begin(), dest.end(), ostream_iterator<DString>(cout, "\n"));

    dest.clear();
    DStringView s5 = "Hello,World,,Good,,Morning";
    s5.split(",", dest);
    assert(dest.size() == 6);
    i = 0;
    assert(dest[i++] == "Hello");
    assert(dest[i++] == "World");
    assert(dest[i++] == "");
    assert(dest[i++] == "Good");
    assert(dest[i++] == "");
    assert(dest[i++] == "Morning");
    copy(dest.begin(), dest.end(), ostream_iterator<DString>(cout, "\n"));

    dest.clear();
    DStringView s6 = "hello";
    s6.split("hello", dest);
    assert(dest.size() == 2);
    i = 0;
    assert(dest[i++] == "");
    assert(dest[i++] == "");

    dest.clear();
    DStringView s7 = "hello";
    s7.split("xxx", dest);
    assert(dest.size() == 1);
    i = 0;
    assert(dest[i++] == "hello");

    dest.clear();
    DStringView s8 = "hello";
    s8.split("helloxxx", dest);
    assert(dest.size() == 1);
    i = 0;
    assert(dest[i++] == "hello");

    dest.clear();
    DStringView s9 = "hello";
    s9.split("el", dest);
    assert(dest.size() == 2);
    i = 0;
    assert(dest[i++] == "h");
    assert(dest[i++] == "lo");
    dest.clear();

    DStringView s10 = "helello";
    s10.split("el", dest);
    assert(dest.size() == 3);
    i = 0;
    assert(dest[i++] == "h");
    assert(dest[i++] == "");
    assert(dest[i++] == "lo");

    DStringView s11 = "Hello \t\r\nWorld\n  \t\t\nGood\n\f\r\nMorning";
    dest.clear();
    s11.split(dest);
    assert(dest.size() == 4);
    i = 0;
    assert(dest[i++] == "Hello");
    assert(dest[i++] == "World");
    assert(dest[i++] == "Good");
    assert(dest[i++] == "Morning");
    copy(dest.begin(), dest.end(), ostream_iterator<DString>(cout, "\n"));
}
//-------------------------------------------------

#define ASSERT_PARTITION(str, what, p1, p2, p3) do {      \
        DString  l, m, r;                                 \
        DStringView(str).partition((what), l, m, r);      \
        assert(l == p1);                                  \
        assert(m == p2);                                  \
        assert(r == p3);                                  \
    } while(0)

void test_partition()
{
    TRACE_FN();

    ASSERT_PARTITION("I could eat bananas all day",
                     "bananas",
                     "I could eat ",
                     "bananas",
                     " all day");

    ASSERT_PARTITION("I could eat bananas all day",
                     "apples",
                     "I could eat bananas all day",
                     "",
                     "");

    ASSERT_PARTITION("I could eat bananas all day",
                     "I could eat bananas all day",
                     "",
                     "I could eat bananas all day",
                     "");

    ASSERT_PARTITION("I could eat bananas all day",
                     "",
                     "",
                     "",
                     "I could eat bananas all day");

    ASSERT_PARTITION("x=y", "=", "x", "=", "y");
    ASSERT_PARTITION("=y", "=", "", "=", "y");
    ASSERT_PARTITION("x=", "=", "x", "=", "");
}
//-------------------------------------------------

#define ASSERT_RPARTITION(str, what, p1, p2, p3) do {     \
        DString  l, m, r;                                 \
        DStringView(str).rpartition((what), l, m, r);     \
        assert(l == p1);                                  \
        assert(m == p2);                                  \
        assert(r == p3);                                  \
    } while(0)

void test_rpartition()
{
    TRACE_FN();

    ASSERT_PARTITION("We all could eat bananas all day",
                     "all",
                     "We ",
                     "all",
                     " could eat bananas all day");

    ASSERT_RPARTITION("We all could eat bananas all day",
                      "all",
                      "We all could eat bananas ",
                      "all",
                      " day");

    ASSERT_RPARTITION("We all could eat bananas all day",
                      "apples",
                      "",
                      "",
                      "We all could eat bananas all day");

    ASSERT_RPARTITION("We all could eat bananas all day",
                      "We all could eat bananas all day",
                      "",
                      "We all could eat bananas all day",
                      "");

    ASSERT_RPARTITION("We all could eat bananas all day",
                      "",
                      "We all could eat bananas all day",
                      "",
                      "");

    ASSERT_RPARTITION("x=y", "=", "x", "=", "y");
    ASSERT_RPARTITION("=y", "=", "", "=", "y");
    ASSERT_RPARTITION("x=", "=", "x", "=", "");

}
//-------------------------------------------------

void test_times()
{
    TRACE_FN();

    DStringView s("A");
    assert(s.times(5) == "AAAAA");
    assert(s.times(2).times(2) == s.times(4));
    assert(s.times(0) == "");
    assert(s.times(1) == s);
    assert(DString('=', 50) == DStringView("=").times(50));

    cout << DString('=', 50) << endl;
    cout << "H E L L O   W O R L D" << endl;
    cout << DStringView("=").times(50) << endl;
}
//-------------------------------------------------

#define TEST_ASCII_UPPER(before, after) do {                \
        DStringView s(before);                              \
        assert(s.upper() == after);                         \
    } while (0)
//-------------------------------------------------

#define TEST_ASCII_LOWER(before, after) do {                \
        DStringView s(before);                              \
        assert(s.lower() == after );                        \
    } while (0)
//-------------------------------------------------

#define TEST_ASCII_SWAPCASE(before, after) do {             \
        DStringView s(before);                              \
        assert(s.swapcase() == after);                      \
    } while (0)
//-------------------------------------------------

#define TEST_REVERSE(before, after) do {                  \
        DStringView s(before);                            \
        assert(s.reverse() == after );                    \
    } while (0)
//-------------------------------------------------

void test_ascii_upper_lower()
{
    TRACE_FN();

    TEST_ASCII_UPPER("a", "A");
    TEST_ASCII_UPPER("a_b_c234DeF", "A_B_C234DEF");

    TEST_ASCII_LOWER( "A", "a");
    TEST_ASCII_LOWER("A_B_c234DeF", "a_b_c234def");

    TEST_ASCII_LOWER( "", "");
    TEST_ASCII_UPPER( "", "");

    TEST_ASCII_SWAPCASE("Hello World", "hELLO wORLD");
    TEST_ASCII_SWAPCASE("", "");
    TEST_ASCII_SWAPCASE("a_b_c234DeF", "A_B_C234dEf");
}
//-------------------------------------------------

void test_reverse()
{
    TRACE_FN();

    TEST_REVERSE("A", "A");
    TEST_REVERSE("AB", "BA");
    TEST_REVERSE("ABC", "CBA");
    TEST_REVERSE("ABCD", "DCBA");
    TEST_REVERSE("Good Morning", "gninroM dooG");
}
//-------------------------------------------------


#define TEST_TRIM_LEFT(before, after) do {                  \
        DStringView s(before);                              \
        assert(s.trim_left() == after);                     \
    } while (0)
//-------------------------------------------------

#define TEST_TRIM_RIGHT(before, after) do {                 \
        DStringView s(before);                              \
        assert(s.trim_right() == after);                    \
    } while (0)
//-------------------------------------------------

#define TEST_TRIM_BOTH(before, after) do {                  \
        DStringView s(before);                              \
        assert(s.trim() == after);                          \
    } while (0)
//-------------------------------------------------

void test_trim()
{
    TRACE_FN();

    TEST_TRIM_LEFT("H", "H");
    TEST_TRIM_LEFT("Hello", "Hello");
    TEST_TRIM_LEFT("      H", "H");
    TEST_TRIM_LEFT("      Hello", "Hello");
    TEST_TRIM_LEFT("         ",  "");
    TEST_TRIM_LEFT("", "");

    TEST_TRIM_RIGHT("Hello",      "Hello");
    TEST_TRIM_RIGHT("Hello    ", "Hello");
    TEST_TRIM_RIGHT("H",          "H");
    TEST_TRIM_RIGHT("H   " ,      "H");
    TEST_TRIM_RIGHT("     ",      "");
    TEST_TRIM_RIGHT("",           "");

    TEST_TRIM_BOTH("     H   ",   "H");
    TEST_TRIM_BOTH("     Hello  ", "Hello");
    TEST_TRIM_BOTH("Hello",   "Hello");
    TEST_TRIM_BOTH("      Hello",  "Hello");
    TEST_TRIM_BOTH("Hello    ",  "Hello");
    TEST_TRIM_BOTH("         ",   "");
    TEST_TRIM_BOTH("",            "");
}
//-------------------------------------------------

void test_left_mid_right()
{
    TRACE_FN();

    DStringView src("ABCD EFGH IJKL MNOP QRST UVWX YZ");
    assert( src.left(4) == "ABCD");
    assert( src.left(9) == "ABCD EFGH");
    assert( src.left(20) == "ABCD EFGH IJKL MNOP ");
    assert( src.left(100) == src);

    assert( src.right(4) == "X YZ");
    assert( src.right(7) == "UVWX YZ");
    assert( src.right(23) == " IJKL MNOP QRST UVWX YZ");
    assert( src.right(100) == src);

    assert( src.mid(5, 4) == "EFGH");
    assert( src.mid(5, 20) == "EFGH IJKL MNOP QRST ");
    assert( src.mid(0, 100) == src);
    assert( src.mid(4, 1000) == " EFGH IJKL MNOP QRST UVWX YZ");
}
//-------------------------------------------------

void test_strip()
{
    TRACE_FN();

    DStringView s("####Hello####");
    assert(s.strip('#') == "Hello");

    s = "#####################";
    assert(s.rstrip('#') == "");

    s = "#####################";
    assert(s.rstrip("#") == "");

    s = " ###Hello###";
    assert(s.strip('#') == " ###Hello");

    s = " \n \t hello\n";
    assert(s.strip('\n') == " \n \t hello");

    s = "\n\n \t hello\n";
    assert(s.strip('\n') == " \t hello");

    s = "www.example.com";
    assert(s.strip("cmow.") == "example");

    s = " www.example.com";
    assert(s.strip("cmow.") == " www.example");

    s = "www.example.com ";
    assert(s.strip("cmow.") == "example.com ");

    s = "Arthur: three!";
    assert(s.lstrip("Arthur: ") == "ee!");
}
//-------------------------------------------------

#define TEST_ZFILL(before, len, after) do {              \
        DStringView sv(before);                          \
        assert(sv.zfill(len) == (after));                \
    } while (0)

void test_zfill()
{
    TEST_ZFILL("35",    5, "00035");
    TEST_ZFILL("+xyz", 10, "+000000xyz");
    TEST_ZFILL("-100",  8, "-0000100");
    TEST_ZFILL("++100", 8, "+000+100");
    TEST_ZFILL("",      5, "00000");
    TEST_ZFILL("+",     5, "+0000");
    TEST_ZFILL("123456",  5, "123456");
    TEST_ZFILL("123456",  6, "123456");
    TEST_ZFILL("123456",  7, "0123456");
}
//-------------------------------------------------

void test_align()
{
    TRACE_FN();

    DStringView sv("Hello");

    assert(sv.align_center(8, '@') == "@Hello@@");
    assert(sv.align_right(8, '@')  == "@@@Hello");
    assert(sv.align_left(8, '@')   == "Hello@@@");

    assert(sv.align_center(2, '@') == "Hello");
    assert(sv.align_right(2, '@')  == "Hello");
    assert(sv.align_left(2, '@')   == "Hello");
}
//-------------------------------------------------

#define TEST_SUCC(before, after) do {                                    \
        DStringView s(before);                                           \
        assert(s.succ() == after);                                       \
        cout << "\"" << before << "\".succ() ==> \"" << after << "\"\n"; \
    } while(0)

void test_succ()
{
    TRACE_FN();

    TEST_SUCC("USA", "USB");
    TEST_SUCC("abcd", "abce");
    TEST_SUCC("THX1138", "THX1139");
    TEST_SUCC("<<koala>>", "<<koalb>>");
    TEST_SUCC("1999zzz", "2000aaa");
    TEST_SUCC("ZZZ9999", "AAAA0000");
    TEST_SUCC("***", "**+");
    TEST_SUCC("9", "10");
    TEST_SUCC("z", "aa");
    TEST_SUCC("zz", "aaa");
    TEST_SUCC("zz", "aaa");
    TEST_SUCC("hell!z99", "helm!a00");
    TEST_SUCC("hell!9", "hell!10");
    TEST_SUCC("hell!99", "hell!100");
    TEST_SUCC("hell!zz", "helm!aa");
    TEST_SUCC("abc-43", "abc-44");
    TEST_SUCC("abc-99", "abc-100");
    TEST_SUCC("abc-a99", "abc-b00");
    TEST_SUCC("abc-z99", "abd-a00");
    TEST_SUCC("hell2!99", "hell3!00");
    TEST_SUCC("hell2!@$99", "hell3!@$00");
    TEST_SUCC("hella!@$zz", "hellb!@$aa");
    TEST_SUCC("a/@z", "b/@a");
    TEST_SUCC("/@z", "/@aa");
}
//-------------------------------------------------

int main()
{
    test_various();
    test_ffo();
    test_flo();
    test_ffno();
    test_flno();
    test_prefix();
    test_suffix();
    test_blank();
    test_atoi();
    test_isdigit();
    test_is_identifier();
    test_count();
    test_find();
    test_rfind();
    test_remove_profix();
    test_expandtabs();
    test_title();
    test_join();
    test_split();
    test_partition();
    test_rpartition();
    test_times();
    test_ascii_upper_lower();
    test_reverse();
    test_trim();
    test_left_mid_right();
    test_strip();
    test_zfill();
    test_align();
    test_succ();
}
