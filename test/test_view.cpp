/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#include <iostream>
#include <algorithm>
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
}
