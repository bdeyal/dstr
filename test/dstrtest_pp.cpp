/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <vector>
#include <list>
#include <deque>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#if __cplusplus >= 201703L
#include <string_view>
#endif
#include <dstr/dstring.hpp>


#if defined(__BORLANDC__) || (defined(_MSC_VER) && (_MSC_VER <= 1200))
#define TRACE_FN() printf("%d\n", __LINE__)
#else
#define TRACE_FN() puts(__FUNCTION__)
#endif

#define TRACE_LN() printf("%d\n", __LINE__)
//-------------------------------------------------

using namespace std;

void test_ctor()
{
    TRACE_FN();

    DString s1("hello");

    assert(s1.length() == strlen("hello"));
    assert(strcmp(s1.c_str(), "hello") == 0);

    DString s2;
    assert(s2.length() == 0);
}
//-------------------------------------------------

void test_eq()
{
    TRACE_FN();
    DString s1("hello");
    DString s2("hello");
    DString s3("HELLO");
    DString s4("Hello World");
    DString s5;

    assert(s1 == s2);
    assert(s1 == "hello");
    assert(s1 != s3);
    assert(s1 != "ABC");
    assert(s1 != s4);

    assert(s1.compare(s2) == 0);
    assert(s1.compare("hello") == 0);
    assert(s1.compare(s3) != 0);
    assert(s1.compare("ABC") != 0);

    assert(s5 == nullptr);
    assert(s1 > nullptr);

    assert(s5 == nullptr);
    assert(s1 != nullptr);

    assert(s3.iequal("hello"));
    assert(s3.iequal("heLLo"));
    assert(s3.iequal("hEllo"));
    assert(!s3.iequal("Xhello"));
}
//-------------------------------------------------

void test_empty_dstr()
{
    TRACE_FN();

    DString s1;
    DString s2((char*)nullptr, size_t(0));
    DString s5(nullptr);
    DString s6('\0', 100);
    DString s7('C', 0);
    DString s8("", 100);

    assert(s1.length() == 0);
    assert(s2.length() == 0);
    assert(s5.length() == 0);
    assert(s6.length() == 0);
    assert(s7.length() == 0);
    assert(s8.length() == 0);

    assert(s1.empty());
    assert(s2.empty());
    assert(s5.empty());
    assert(s6.empty());
    assert(s7.empty());
    assert(s8.empty());

    assert( s1 == s2 );
    assert( s2 == s5 );
    assert( s5 == s6 );
    assert( s6 == s7 );
    assert( s7 == s8 );
    assert( s8 == s1 );

    assert(strlen(s1.c_str()) == 0);
    assert(strlen(s2.c_str()) == 0);
    assert(strlen(s5.c_str()) == 0);
    assert(strlen(s6.c_str()) == 0);
    assert(strlen(s7.c_str()) == 0);
    assert(strlen(s8.c_str()) == 0);

    assert(strcmp(s1.c_str(), "") == 0);
    assert(strcmp(s2.c_str(), "") == 0);
    assert(strcmp(s5.c_str(), "") == 0);
    assert(strcmp(s6.c_str(), "") == 0);
    assert(strcmp(s7.c_str(), "") == 0);
    assert(strcmp(s8.c_str(), "") == 0);
}
//-------------------------------------------------

void test_char_ctor()
{
    TRACE_FN();
    DString s1('A', 5);
    assert(s1.length() == 5 );
    assert(s1 == "AAAAA");

    DString s2('B', 100);
    assert(s2.length() == 100 );
    for (int i = 0; i < 100; ++i) {
        assert(s2[i] == 'B');
    }

    DString s3('\0', 5);
    assert(s3.length() == 0);
    assert(s3 == "");
}
//-------------------------------------------------

void test_copy_ctor()
{
    TRACE_FN();
    DString s1("hello");
    DString s2(s1);

    assert(s1 == s2);
    assert(s2 == "hello");

    DString s3("Hello world this is a longer string");
    DString s4(s3);
    assert(s3 == s4);
}
//-------------------------------------------------

void test_buff_ctor()
{
    TRACE_FN();

    DString s1("ABCDEFGH", 5);
    assert(s1.length() == 5);
    assert(s1 == "ABCDE");

    DString s1a("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 20);
    assert(s1a.length() == 20);
    assert(s1a == "ABCDEFGHIJKLMNOPQRST");

    DString s2("ABCDEFGH", size_t(0));
    assert(s2.length() == 0);
    assert(s2 == "");

    // Test assign_buff too
    //
    DString s3;
    s3.assign("ABCDEFGH", 5);
    assert(s3.length() == 5);
    assert(s3 == "ABCDE");

    DString s4;
    s4.assign("ABCDEFGH", size_t(0));
    assert(s4.length() == 0);
    assert(s4 == "");

    DString s5("", 10);
    assert(s5 == "");
    assert(s5.length() == 0);
}
//-------------------------------------------------

void test_range_ctor()
{
    TRACE_FN();

    const char* cstr = "ABCDEFGH";
    DString s1(cstr, cstr + 5);
    assert(s1.size() == 5);
    assert(s1 == "ABCDE");

    DString s2;
    s2.assign(cstr, cstr + 5);
    assert(s1 == s2);

    DString s3("Hello");
    s3.append(cstr, cstr + 3);
    assert(s3.size() == 8);
    assert(s3 == "HelloABC");

    DString s4("Hello  World");
    s4.insert(6, cstr, cstr+3);
    assert(s4.size() == 15);
    assert(s4 == "Hello ABC World");

    s4.replace(6, 3, cstr + 3, cstr + 8);
    assert(s4 == "Hello DEFGH World");
}
//-------------------------------------------------

void test_assign()
{
    TRACE_FN();

    DString s1;
    s1 = "hello";
    assert(s1 == "hello");
    assert(s1.length() == strlen("hello"));

    DString s2;
    s2 = s1;
    assert(s1 == s2);
    assert(s2 == "hello");
    assert(s2.length() == strlen("hello"));

    s2.assign('C', 8);
    assert(s2.length() == 8);
    assert(s2 == "CCCCCCCC");
    assert(strcmp(s2.c_str(), "CCCCCCCC") == 0);

    s2 = (char*)nullptr;
    assert(s2.length() == 0);

    DString s3("hello");
    DString s4;
    s3 = s4;
    assert(s3 == s4);
    assert(s3.length() == 0);

    DString s5("hello");
    s5 = "world";
    assert(s5 == "world");
    assert(s5.length() == strlen("world"));

    DString s6;
    s6.reserve(100);
    s6.assign(s5, 0, DSTR_NPOS);
    assert(s5 == s6);

    s6.assign(s5, 0, 2);
    assert(s6 == "wo");

    s6.assign(s5, 1, 3);
    assert(s6 == "orl");

    s6.assign(s5, 3, 2);
    assert(s6 == "ld");

    s6.assign(s5, 3, 100);
    assert(s6 == "ld");

    DString s7;
    s7 = 'C';
    s7 += '+';
    s7 += '+';
    assert(s7 == "C++");

    DString s8("Hello World");
    s8 = 'C';
    s8 += '+';
    s8 += '+';
    assert(s8 == "C++");
}
//-------------------------------------------------

void test_append()
{
    TRACE_FN();

    DString s1("hello");
    DString s2("world");
    DString s3(s1);
    s3.append(s2);

    assert(s3 == "helloworld");
    assert(s3.length() == s2.length() + s1.length());

    DString s4;
    s4 = s1;
    s4.append("world");
    assert(s4 == "helloworld");

    s4.append((char*)nullptr);
    assert( s4 == "helloworld");

    s4.append("");
    assert(s4 == "helloworld");

    DString s5;
    s5.append('h', 1);
    assert(s5 == "h");

    s5.append('e');
    s5.append('l', 2);
    s5.append('o');
    assert(s5 == "hello");

    s5.append('\0');
    assert(s5 == "hello");

    s5.append("_ABC\0EFG", 10);
    assert(s5 == "hello_ABC");

    s5.append((char*)nullptr, 10);
    assert(s5 == "hello_ABC");

    s5.append("Hi", size_t(0));
    assert(s5 == "hello_ABC");

    s5.append("_DEF", 3);
    assert(s5 == "hello_ABC_DE");

    s5.append("FGH", 1);
    assert(s5 == "hello_ABC_DEF");

    s5.append(s5);
    assert(s5 == "hello_ABC_DEFhello_ABC_DEF");

    int step = 1000;
    int limit = 10 * step;
    for (int i = 0; i < limit; ++i) {
        if ((i % step) == 0) {
            printf("step: %d LEN=%zu, CAP=%zu\n", i, s5.length(), s5.capacity());
        }
        s5.append('X');
    }

    s5.clear();

    for (int i = 0; i < limit; ++i) {
        if ((i % step) == 0) {
            printf("step: %d LEN=%zu, CAP=%zu\n", i, s5.length(), s5.capacity());
        }
        s5.append("hello");
    }

    DString s6;
    s6 += "Hello";

    DString w(", World");
    s6 += w;

    s6 += ' ';
    s6 += 'X';
    assert(s6 == "Hello, World X");

    for (int i = 0; i < 20; ++i) {
        printf("step: %d LEN=%zu, CAP=%zu\n", i, s6.length(), s6.capacity());
        s6 += s6;
    }

    putchar('\n');
}
//-------------------------------------------------

void test_format()
{
    TRACE_FN();

    DString res1;
    char res2[4096];

    // simple int
    //
    res1.sprintf("%d", 100);
    sprintf(res2, "%d", 100);
    assert(res1 == res2);
    TRACE_LN();

    // simple double
    //
    res1.sprintf("%g", 100.0);
    sprintf(res2, "%g", 100.0);
    assert(res1 == res2);
    TRACE_LN();

    // hex
    //
    res1.sprintf("%08X", 100);
    sprintf(res2, "%08X", 100);
    assert(res1 == res2);
    TRACE_LN();

    // long string
    //
    char buffer[2048];
    memset(buffer, 'a', sizeof buffer);
    buffer[sizeof(buffer) - 1] = '\0';
    TRACE_LN();

    res1.sprintf("%s", buffer);
    sprintf(res2, "%s", buffer);
    assert(res1 == res2);
    TRACE_LN();

    // Complicated format
    //
    res1.sprintf("%0X %g %10s %10d %-20s", 100, 4.55, "Hello World", 10, "A_String");
    sprintf(res2, "%0X %g %10s %10d %-20s", 100, 4.55, "Hello World", 10, "A_String");
    assert(res1 == res2);
    TRACE_LN();

    res1.sprintf("%s", "Hello ");
    res1.append_sprintf("%s", "World");
    sprintf(res2, "%s", "Hello World");
    assert(res1 == res2);
    TRACE_LN();

    TRACE_LN();
}
//-------------------------------------------------

void test_remove()
{
    TRACE_FN();

    const char* hello = "XhellXXoXXX";
    DString s1;
    s1 = hello;
    assert(s1 == hello);

    // erase at loc 1000 -> NOOP
    s1.erase(1000, 1);
    assert(s1 == hello);

    s1.erase(0);
    assert(s1 == "hellXXoXXX");

    s1.erase(4, 2);
    assert(s1 == "helloXXX");

    s1.erase(5, 3);
    assert(s1 == "hello");

    s1.erase(4, 100);
    assert(s1 == "hell");

    s1.erase(2, 0);
    assert(s1 == "hell");

    s1.erase(0, DString::NPOS);
    assert(s1 == "");
}
//-------------------------------------------------

#define TEST_TRIM_LEFT(before, after) do {                  \
        DString s(before);                                  \
        s.trim_left();                                      \
        assert(s == after);                                 \
    } while (0)
//-------------------------------------------------

#define TEST_TRIM_RIGHT(before, after) do {                 \
        DString s(before);                                  \
        s.trim_right();                                     \
        assert(s == after);                                 \
    } while (0)
//-------------------------------------------------

#define TEST_TRIM_BOTH(before, after) do {                  \
        DString s(before);                                  \
        s.trim();                                           \
        assert(s == after);                                 \
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

void test_insert()
{
    TRACE_FN();

    const char* hello = "hello";
    DString s1;
    s1 = hello;
    assert(s1 == hello);

    s1.insert(1000, 'X', 1);
    assert(s1 == "helloX");

    s1.insert(4, "XX");
    assert(s1 == "hellXXoX");

    s1.insert(4, "");
    assert(s1 == "hellXXoX");

    s1.insert(4, nullptr);
    assert(s1 == "hellXXoX");

    s1.insert(0, "XXX", DString::NPOS);
    assert( s1 == "XXXhellXXoX");

    s1.insert(0, (char*)nullptr, DString::NPOS);
    assert( s1 == "XXXhellXXoX");

    s1.insert(0, "XXX", size_t(0));
    assert( s1 == "XXXhellXXoX");

    s1.insert(0, 'A', 3);
    assert( s1 == "AAAXXXhellXXoX");

    s1.insert(1000, 'B', 2);
    assert(s1 == "AAAXXXhellXXoXBB");

    s1 = "ABCDE";
    s1.insert(3, 'Z', 5);
    assert(s1 == "ABCZZZZZDE");

    DString s2('X', 2);
    s1.insert(2, s2);
    assert(s1 == "ABXXCZZZZZDE");

    s2 = "0123456789";
    s2.insert(s2.length(), "ABCDE");
    assert(s2 == "0123456789ABCDE");
}
//-------------------------------------------------

void test_replace1()
{
    TRACE_FN();

    DString s1("hell@@@@@oXXX");
    DString sn("hello");
    s1.replace(10, 5, sn);
    assert(s1 == "hell@@@@@ohello");
}
//-------------------------------------------------

void test_replace()
{
    TRACE_FN();

    const char* hello = "XhellXXoXXX";
    DString s1;
    s1 = hello;
    assert(s1 == hello);

    s1.replace(0, 1, "");
    assert( s1 == "hellXXoXXX");

    s1.replace(4, 2, "@@@@@");
    assert( s1 == "hell@@@@@oXXX");

    DString sn("hello");
    s1.replace(10, 5, sn);
    assert( s1 == "hell@@@@@ohello");

    s1.replace(4, 5, nullptr);
    assert( s1 == "hellohello");

    s1.replace(5, 5, '\0', 1);
    assert( s1 == "hello" );

    s1.replace(0, DString::NPOS, "world");
    assert( s1 == "world");

    DString s2(s1);
    s1.replace(0, DString::NPOS, (char*)nullptr, 100);
    assert(s1 == "");

    s2.replace(2, 8,'\0', 100);
    assert(s2 == "wo");

    s2.replace(0, 2,'\0', 100);
    assert(s2 == "");

    DString s3("H World");
    const char* ps3 = s3.c_str() + s3.find("World");
    s3.replace(0, 1, ps3);
    printf("%s\n", s3.c_str());
    assert( s3 == "World World");
}
//-------------------------------------------------

// Compare to 'DString::fgets' to 'cin >> str' must be equal
//
void test_fgets()
{
    const char* fname = __FILE__;
    FILE* fp = fopen(fname, "r");
    if (!fp) {
        fprintf(stderr, "couldn't open %s: %s\n", fname, strerror(errno));
        exit(1);
    }

    ifstream in(fname);
    if (!in) {
        cerr << "couldn't open "<< fname << ": " << strerror(errno) << endl;
        exit(1);
    }

    DString d1;
    std::string s1;

    for (;;) {
        int r1 = d1.fgets(fp);
        in >> s1;

        if (r1 == EOF) {
            assert(in.eof());
            break;
        }

        assert(s1 == d1.c_str());
        assert(d1 == s1.c_str());
    }

    fclose(fp);

    cout << "test_dgets: Good! all lines were equal" << endl;
}
//-------------------------------------------------

// Compare to 'dstr_getline' to 'std::getline(in, str)' must be equal
//
void test_getline()
{
    const char* fname = __FILE__;
    FILE* fp = fopen(fname, "r");
    if (!fp) {
        fprintf(stderr, "couldn't open %s: %s\n", fname, strerror(errno));
        exit(1);
    }

    ifstream in(fname);
    if (!in) {
        cerr << "couldn't open "<< fname << ": " << strerror(errno) << endl;
        exit(1);
    }

    DString d1;
    std::string s1;

    for (;;) {
        int r1 = d1.fgetline(fp);
        std::getline(in, s1);

        if (r1 == EOF) {
            assert(in.eof());
            break;
        }

        assert(s1 == d1.c_str());
        assert(d1 == s1.c_str());
    }
    fclose(fp);

    cout << "test_getline: Good! all lines were equal" << endl;
}
//-------------------------------------------------

// Compare to 'dstr_getline' to 'std::getline(in, str)' must be equal
//
std::string file_slurp(const char* fname)
{
    ifstream in(fname);
    if (!in) {
        cerr << "couldn't open "<< fname << ": " << strerror(errno) << endl;
        exit(1);
    }

    ostringstream out;
    out << in.rdbuf();

    return out.str();
}

void test_fromfile()
{
    const char* fname = __FILE__;

    DString d1;
    if (!d1.from_file(fname)) {
        perror("from_file");
        return;
    }
    std::string s1 = file_slurp(fname);

    assert(s1 == d1.c_str());
    assert(d1 == s1.c_str());

    cout << "test_fromfile: Good! file content equal" << endl;
}
//-------------------------------------------------

void test_left_mid_right()
{
    TRACE_FN();

    DString src("ABCD EFGH IJKL MNOP QRST UVWX YZ");
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

void test_substr()
{
    TRACE_FN();

    char buff[2048];
    const char* longstr = "Good morning today is Friday";
    DString s1(longstr);

    // verifiy that no byte was written
    size_t n = s1.copy_substr(100, DString::NPOS, buff, 5);
    assert(n == 0);

    n = s1.copy_substr(0, DString::NPOS, buff, 0);
    assert(n == 0);

    n = s1.copy_substr(0, DString::NPOS, buff, 1);
    assert(n == 1);
    assert(strcmp(buff, "") == 0);

    n = s1.copy_substr(0, 0, buff, 100);
    assert(n == 0);

    n = s1.copy_substr(0, DString::NPOS, buff, sizeof(buff));
    assert(n == strlen(longstr) + 1);
    assert(strcmp(buff, longstr) == 0);

    n = s1.copy_substr(0, DString::NPOS, buff, 5);
    assert(n == 5);
    assert(strcmp(buff, "Good") == 0);

    n = s1.copy_substr(15, DString::NPOS, buff, 10);
    assert(n == 10);
    assert(strcmp(buff, "day is Fr") == 0);

    DString s2(s1, 5, 7);
    assert( s2 == "morning");

    s2.assign(s1, 0, 4);
    assert( s2 == "Good");

    s2.assign(s1, 15, 10);
    assert(s2 == "day is Fri");
}
//-------------------------------------------------

void test_truncate()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DString s1(longstr);

    s1.resize(100);
    assert(s1 == longstr);

    s1.resize(10);
    assert(strncmp(s1.c_str(), longstr, 10) == 0);

    s1.clear();
    assert(s1.length() == 0);
    assert(s1 == "");
}
//-------------------------------------------------

void test_find()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DString s1(longstr);

    assert( s1.find("good") == DString::NPOS);
    assert( s1.find("Good") == 0);
    assert( s1.find("morning") == 5);
    assert( s1.find("morning", 6) == DString::NPOS);
    assert( s1.find("morning", 100) == DString::NPOS);
    assert( s1.find("day") == 15);
    assert( s1.find("date", 3) == DString::NPOS);

    assert( s1.contains("is"));
    assert( s1.contains("morn"));
    assert(!s1.contains("XXX"));

    assert( s1.ifind("good") == 0);
    assert( s1.ifind("good", 1) == DString::NPOS);
    assert( s1.ifind("GOOD") == 0);
    assert( s1.ifind("MoRnInG") == 5);
    assert( s1.ifind("MoRnInG", 6) == DString::NPOS);
    assert( s1.ifind("MoRnInG", 1000) == DString::NPOS);
    assert( s1.ifind("DAY", 8) == 15);
    assert( s1.ifind("Date") == DString::NPOS);

    assert( s1.icontains("IS"));
    assert( s1.icontains("MoRn"));
    assert(!s1.icontains("xXx"));

    assert( s1.find('g') == 11);
    assert( s1.find('g', 1) == 11);
    assert( s1.find('g', 2) == 11);
    assert( s1.find('g', 11) == 11);
    assert( s1.find('g', 12) == DString::NPOS);
    assert( s1.find('g', 100) == DString::NPOS);
    assert( s1.find('G') == 0);
    assert( s1.find('m') == 5);
    assert( s1.find('d', 1) == 3);
    assert( s1.find('X') == DString::NPOS);

    assert( s1.ifind('G') == 0);
    assert( s1.ifind('g') == 0);
    assert( s1.ifind('m') == 5);
    assert( s1.ifind('M', 3) == 5);
    assert( s1.ifind('f', 15) == 22);
    assert( s1.ifind('f', 100) == DString::NPOS);
    assert( s1.ifind('X') == DString::NPOS);
}
//-------------------------------------------------


void test_rfind()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DString s1(longstr);

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

    s1 = "";
    assert(s1.irfind('X') == DSTR_NPOS);
    assert(s1.irfind("good") == DSTR_NPOS);
    assert(s1.rfind('X') == DSTR_NPOS);
    assert(s1.rfind("good") == DSTR_NPOS);

    assert(DString("hello").rfind("hello") == 0);
    assert(DString("hello").rfind("l") == 3);
    assert(DString("he@@@XXX@@@llo").rfind("@@@") == 8);
    assert(DString("he@@@XXX@@@llo").rfind("@@@", 9) == 8);
    assert(DString("hello").irfind("HeLlO") == 0);
    assert(DString('C', 1).irfind('c') == 0);

    assert(std::string("he@@@XXX@@@llo").rfind("@@@") == 8);
    assert(std::string("he@@@XXX@@@llo").rfind("@@@", 9) == 8);
    assert(std::string("").rfind("") == 0);
    assert(std::string("").rfind("XX") == string::npos);
    assert(std::string("XX").rfind("") == 2);

    assert(DString("").rfind("") == 0);
    assert(DString("").rfind("XX") == DString::NPOS);
    assert(DString("XX").rfind("") == 2);

}
//-------------------------------------------------

void test_put_get()
{
    TRACE_FN();

    const char * str_lower = "abcdefghijklmnopqrstuvwxyz";
    const char* str_upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    DString s1(str_lower);

    for (size_t n = 0; n < s1.length(); ++n) {
        assert(s1.index_ok(n));
        char c = s1[n];
        c = toupper(c);
        assert(c == str_upper[n]);
        s1[n] = c;
    }

    assert(!s1.index_ok(s1.length()));
    assert(!s1.index_ok(DString::NPOS));
    assert(s1 == str_upper);
}
//-------------------------------------------------

void test_put_get_safe()
{
    TRACE_FN();

    const char * str_lower = "abcdefghijklmnopqrstuvwxyz";
    const char* str_upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    DString s1(str_lower);

    // first we must support the non-safe semantics
    //
    for (size_t n = 0; n < s1.length(); ++n) {
        assert(s1.index_ok(n));
        char c = s1.get(n);
        s1.put(n, toupper(c));
    }
    assert(s1 == str_upper);

    // back to lowercase
    s1.lower();
    assert(s1 == str_lower);

    // Now test the 'safe' part
    //
    size_t index = s1.length() + 5;
    assert(s1.get(index) == '\0');
    assert(s1.get(-(long)index) == '\0');

    DString s2(s1);
    s2.put(index, 'A');
    assert(s1 == s2);
    s2.put(-(long)index, 'A');
    assert(s1 == s2);

    for (size_t n = 1; n <= s2.length(); ++n) {
        long i = -(long)n;
        char c = s2.get(i);
        assert(c == s2[s2.length() - n]);
        s2.put(i, toupper(c));
    }
    assert(s2 == str_upper);
}
//-------------------------------------------------

#define TEST_ASCII_UPPER(before, after) do {                \
        DString s(before);                                  \
        s.upper();                                          \
        assert(s == after);                                 \
        assert(s.size() == 0 || s.isupper());              \
    } while (0)
//-------------------------------------------------

#define TEST_ASCII_LOWER(before, after) do {                \
        DString s(before);                                  \
        s.lower();                                          \
        assert( s == after );                               \
        assert(s.size() == 0 || s.islower());              \
    } while (0)
//-------------------------------------------------

#define TEST_ASCII_SWAPCASE(before, after) do {             \
        DString s(before);                                  \
        s.swapcase();                                       \
        assert(s == after);                                 \
    } while (0)
//-------------------------------------------------

#define TEST_REVERSE(before, after) do {                  \
        DString s(before);                                \
        s.reverse();                                      \
        assert( s == after );                             \
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

void test_swap()
{
    TRACE_FN();

    // 'nullptr' is OK but C++98 does not have it
    //
    DString s1(nullptr);
    DString s2("hello");

    s1.swap(s2);
    assert(s2.empty());
    assert(s1 == "hello");
}
//-------------------------------------------------

void test_ffo()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DString s1(longstr);

    size_t res = s1.ffo(" \t");
    assert(res == 4);

    res = s1.ffo(" \t", 5);
    assert(res == 12);

    res = s1.ffo(" \t", 1000);
    assert(res == DString::NPOS);

    DString rhs("Frdy");
    res = s1.ffo(rhs);
    assert(res == 3);

    DString comment("# comment");
    res = comment.ffo("#;");
    assert(res == 0);

    res = comment.ffo("XYZ");
    assert(res == DString::NPOS);

    comment = "";
    assert(comment.empty());
    assert(comment.ffo("X") == DString::NPOS);
}
//-------------------------------------------------

void test_flo()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DString s1(longstr);

    size_t res = s1.flo(" \t");
    assert(res == 21);

    res = s1.flo(" \t", 5);
    assert(res == 4);

    res = s1.flo("");
    assert(res == DSTR_NPOS);

    DString rhs("Frdy");
    res = s1.flo(rhs);
    assert(res == 27);

    rhs = " F";
    res = s1.flo(rhs);
    assert(res == 22);

    DString comment("# comment");
    res = comment.flo("#;");
    assert(res == 0);

    res = comment.flo("XYZ");
    assert(res == DSTR_NPOS);

    comment = "";
    assert( comment.flo("X") == DSTR_NPOS );

    DString path = "/path/to/a/directory/with/file.txt";
    DString ext(path.substr(1 + path.flo(".")));
    assert(ext == "txt");

    DString fname(path.substr(1+path.flo("/")));
    assert(fname == "file.txt");
}
//-------------------------------------------------

void test_ffno()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DString s1(longstr);

    size_t res = s1.ffno(" \t");
    assert(res == 0);

    res = s1.ffno("Godm ");
    assert(res == 7);

    res = s1.ffno("Godm ", 5);
    assert(res == 7);

    res = s1.ffno("Godm ", 8);
    assert(res == 8);

    DString rhs("Godmr ");
    res = s1.ffno(rhs);
    assert(res == 8);

    s1 = "ABCACABCABCABC";
    res = s1.ffno("ABC");
    assert(res == DString::NPOS);

    res = s1.ffno("AB");
    assert(res == 2);

    // split a string
    DString s2(longstr);
    size_t first, last = 0;
    while ((first = s2.ffno(" \t", last)) != DString::NPOS) {
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
    DString s1(longstr);

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
    DString s1(longstr);

    assert( s1.startswith("G"));
    assert( s1.startswith("Good"));
    assert( s1.startswith(longstr));
    assert( !s1.startswith("good"));

    assert( s1.istartswith("GOOD MORNING"));
    assert( s1.istartswith("GooD"));
    assert( s1.istartswith("GooD morNiNg Today is Friday"));
    assert( !s1.startswith("H"));
    assert( !s1.istartswith("H"));

    DString s2("Good");
    const char* longer_str = "GoodXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    assert(s2.startswith("Good"));
    assert(!s2.startswith(longer_str));
}
//-------------------------------------------------

void test_suffix()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DString s1(longstr);

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
    DString s1(longstr);

    assert( !s1.isblank() );
    TRACE_LN();

    s1 = " \t";
    assert( s1.isblank() );

    s1 = " \t:";
    assert( !s1.isblank() );

    s1 = "         ";
    assert( s1.isblank() );
}
//-------------------------------------------------

#define TEST_ATOI(src, result) do {                                \
        DString s(src);                                            \
        long n = s.atoi();                                         \
        printf("Converted = %ld, Expected = %ld\n", n, result);    \
        assert( n == result );                                     \
    } while (0)
//-------------------------------------------------

#define TEST_ITOS(n, result) do {                                       \
        DString s;                                                      \
        s.itos(n);                                                      \
        printf("Converted = %s, Expected = %s\n", s.c_str(), result);   \
        assert(s == result);                                            \
    } while (0)
//-------------------------------------------------

void test_atoi_itos()
{
    TRACE_FN();

    TEST_ATOI("1", 1L);
    TEST_ATOI("\\1234", 668L);
    TEST_ATOI("0b111001", 57L);
    TEST_ATOI("0xFFFF", 65535L);

    TEST_ITOS(1, "1");
    TEST_ITOS(-1, "-1");
    TEST_ITOS(1234, "1234");
    TEST_ITOS(0, "0");
    TEST_ITOS(-1234, "-1234");

    // Verify itos conversion against plain C sprintf.
    // Must be equal
    //
    DString d;;
    for (int i = 0; i < 10000; ++i) {
        char buf[100];
        long n = rand();
        n *= rand();
        if (n % 2) n = -n;
        sprintf(buf, "%ld", n);
        d.itos(n);
        assert(d == buf);
    }
}
//-------------------------------------------------

#define TEST_DIGIT(s, result) do {                            \
        DString d(s);                                         \
        puts(s);                                              \
        assert( d.isdigits() == result );                     \
        assert( d.isnumeric() == result );                    \
        assert( d.isdecimal() == result );                    \
        assert( d.isdigits() == result );                     \
    } while (0)

#define TEST_XDIGIT(s, result) do {                         \
        DString d(s);                                       \
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
    assert(DString("MyFolder").isidentifier());
    assert(DString("Demo001").isidentifier());
    assert(DString("_Demo001").isidentifier());
    assert(!DString("4Demo001").isidentifier());
    assert(!DString("2bring").isidentifier());
    assert(!DString("my demo").isidentifier());
}
//-------------------------------------------------

DString rvo_move_ctor()
{
    return DString('X', 100000);
}

void test_move_ctor_assign()
{
#if __cplusplus >= 201103L
    TRACE_FN();

    DString src("hello");
    DString dest(std::move(src));

    cout << "SRC: " << src << ", DEST: " << dest << endl;
    assert(dest == "hello");
    assert(src.empty());

    src = "world";
    dest = std::move(src);

    assert(dest == "world");
    assert(src == "hello");

    DString x = rvo_move_ctor();
    printf("x length = %zu\n", x.length());
#endif
}
//-------------------------------------------------

void test_push_back_front()
{
    TRACE_FN();

    DString src("hello");

    src.push_back('X');
    assert(src == "helloX");

    src.chop();
    src.push_front('X');
    assert(src == "Xhello");

}
//-------------------------------------------------

void test_pop_back_front()
{
    TRACE_FN();

    DString src("hello");

    src.pop_back();
    assert(src == "hell");

    src.pop_front();
    assert(src == "ell");
}
//-------------------------------------------------

void test_operator_plus()
{
    TRACE_FN();

    DString s1("abc");
    DString s2("def");
    assert(s1 + s2 == "abcdef");

    char c = 'X';
    assert(s1 + c == "abcX");
    assert(c + s1 == "Xabc");
    assert(s1 + s2 + c == "abcdefX");
    assert(s1 + c + s2 == "abcXdef");
    assert(c + s1 + s2 == "Xabcdef");

    const char* sz = "ghi";
    assert(s1 + sz == "abcghi");
    assert(sz + s1 == "ghiabc");
    assert(s1 + s2 + sz == "abcdefghi");
    assert(s1 + sz + s2 == "abcghidef");
    assert(sz + s1 + s2 == "ghiabcdef");
    assert(sz + s1 + c == "ghiabcX");
    assert(c + s1 + sz == "Xabcghi");
}
//-------------------------------------------------

void test_operator_square_braces()
{
    TRACE_FN();

    DString s1("abc");
    s1[2] = toupper(s1[2]);
    assert(s1 == "abC");

    DString s2("Hello World Today is WED at midnight");
    for (size_t i = 0; i < s2.length(); ++i) {
        assert(s2[i] == s2.get(i));
        assert(&s2[i] == s2.data() + i);
    }
}
//-------------------------------------------------

void test_center()
{
    TRACE_FN();

    DString s1("Hello");

    DString s2 = s1.align_center(30, '@');
    cout << s2 << endl;

    s2 = s1.align_center(s1.length(), '@');
    cout << s2 << endl;
    assert(s1 == s2);

    s2 = s1.align_center(s1.length() - 1, '@');
    cout << s2 << endl;
    assert(s1 == s2);

    s2 = s1.align_center(20, '+');
    cout << s2 << endl;

    s2 = s1.align_center(20, ' ');
    cout << "\"" << s2 << "\"" << endl;

    s2 = s1.align_center(30);
    cout << "\"" << s2 << "\"" << endl;
}
//-------------------------------------------------

void test_align()
{
    TRACE_FN();

    DString s1("Hello");

    DString s2 = s1.align_left(30, '@');
    cout << s2 << endl;

    s2 = s1.align_right(30, '@');
    cout << s2 << endl;

    s2 = s1.align_left(s1.length(), '@');
    cout << s2 << endl;
    assert(s1 == s2);

    s2 = s1.align_right(s1.length(), '@');
    cout << s2 << endl;
    assert(s1 == s2);

    s2 = s1.align_left(s1.length() - 1, '@');
    cout << s2 << endl;
    assert(s1 == s2);

    s2 = s1.align_right(s1.length() - 1, '@');
    cout << s2 << endl;
    assert(s1 == s2);

    s2 = s1.align_left(20, '+');
    cout << s2 << endl;

    s2 = s1.align_right(20, '+');
    cout << s2 << endl;

    s2 = s1.align_left(20, ' ');
    cout << "\"" << s2 << "\"" << endl;

    s2 = s1.align_right(20, ' ');
    cout << "\"" << s2 << "\"" << endl;

    s2 = s1.align_left(30);
    cout << "\"" << s2 << "\"" << endl;

    s2 = s1.align_right(30);
    cout << "\"" << s2 << "\"" << endl;
}
//-------------------------------------------------


// TODO: complete this
//
void test_conversion()
{
    TRACE_FN();

    DString s("15");
    size_t index;

    int n = s.stoi(&index);
    printf("%d, %zu\n", n, index);
}
//-------------------------------------------------

void test_replace_all()
{
    TRACE_FN();

    const char* origstr = "I love apple apple apple apple apple";
    DString orig(origstr);
    cout << "ORIG:\n" << orig << endl;

    orig.replace_all("apple", "@bannana@");
    cout << orig << endl;

    orig = origstr;
    orig.replace_all("apple", "fig");
    cout << orig << endl;

    orig = origstr;
    orig.replace_all("apple", "@bannana@", 2);
    cout << orig << endl;

    orig = origstr;
    orig.replace_all(DString("apple"), DString("@DString@"));
    cout << orig << endl;

    orig = origstr;
    orig.replace_all(DString("apple"), DString("@DString@"), 0);
    cout << orig << endl;

}
//-------------------------------------------------

void test_count()
{
    TRACE_FN();

    assert(DString("").count("") == 1);
    assert(DString("A").count("") == 2);
    assert(DString("ABC").count("") == 4);
    assert(DString("ppppp").count("pp") == 2);
    assert(DString("ppppp").count("ppp") == 1);

    assert(DString("").icount("") == 1);
    assert(DString("A").icount("") == 2);
    assert(DString("ABC").icount("") == 4);
    assert(DString("ppppp").icount("pp") == 2);
    assert(DString("ppppp").icount("ppp") == 1);
    assert(DString("ppppp").icount("PpP") == 1);

    assert(DString("").count(DString("")) == 1);
    assert(DString("ABC").count(DString("")) == 4);
    assert(DString("ppppp").count(DString("pp")) == 2);
    assert(DString("ppppp").count(DString("ppp")) == 1);

    assert(DString("Hello World").count("hell") == 0);
    assert(DString("Hello World").icount("hell") == 1);

    assert(DString("Hello World").count("RLD") == 0);
    assert(DString("Hello World").icount("RLD") == 1);
}
//-------------------------------------------------

void test_expandtabs()
{
    TRACE_FN();

    DString s("\t");

    assert(s.expandtabs() == "        ");
    assert(s.expandtabs(1) == " ");
    assert(s.expandtabs(2) == "  ");
    assert(s.expandtabs(60) == DString(' ', 60));

    s = "Hello\tWorld\tToday\tIs\tSaturday";
    assert(s.expandtabs()  == "Hello   World   Today   Is      Saturday");
    assert(s.expandtabs(7) == "Hello  World  Today  Is     Saturday");
    assert(s.expandtabs(6) == "Hello World Today Is    Saturday");
    assert(s.expandtabs(5) == "Hello     World     Today     Is   Saturday");
    assert(s.expandtabs(4) == "Hello   World   Today   Is  Saturday");
    assert(s.expandtabs(3) == "Hello World Today Is Saturday");
    assert(s.expandtabs(2) == "Hello World Today Is  Saturday");
    assert(s.expandtabs(1) == "Hello World Today Is Saturday");
    assert(s.expandtabs(0) == "HelloWorldTodayIsSaturday");
}
//-------------------------------------------------

void test_title()
{
    TRACE_FN();

    const char* s = "hello world today 33 is SAT";
    DString d1(s);

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

    DString s1;
    s1.join_inplace("...", argv, argc);
    cout << s1 << endl;
    assert(s1 == "hello...world...good...morning");

    s1.clear();
    s1.join_inplace("...", argv, 100);
    cout << s1 << endl;
    assert(s1 == "hello...world...good...morning");

    s1.clear();
    s1.join_inplace("...", argv, 2);
    cout << s1 << endl;
    assert(s1 == "hello...world");

    DString sep("+++");
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
    s1.clear();
    s1.join_inplace("...", v);
    cout << s1 << endl;
    assert(s1 == "hello...world...good...morning");

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

    DString s2 = "  Hello,World,,,\t\tToday is Tuesday 1.1.1.1, 2.2.2.2,33.33.33";

    s2.tokenize(", \t", dest);
    assert(dest.size() == 8);
    assert(dest[0] == "Hello");
    assert(dest[1] == "World");
    assert(dest[6] == "2.2.2.2");
    assert(dest[7] == "33.33.33");
    copy(dest.begin(), dest.end(), ostream_iterator<DString>(cout, "\n"));

    DString s3("Hello\nWorld\n\nGood\n\nMorning");
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
    s3 = "Hello:World::Good::Morning";
    s3.split(':', dest);
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
    s3 = "Hello,World,,Good,,Morning";
    s3.split(",", dest);
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
    s3 = "hello";
    s3.split("hello", dest);
    assert(dest.size() == 2);
    i = 0;
    assert(dest[i++] == "");
    assert(dest[i++] == "");

    dest.clear();
    s3 = "hello";
    s3.split("xxx", dest);
    assert(dest.size() == 1);
    i = 0;
    assert(dest[i++] == "hello");

    dest.clear();
    s3 = "hello";
    s3.split("helloxxx", dest);
    assert(dest.size() == 1);
    i = 0;
    assert(dest[i++] == "hello");

    dest.clear();
    s3 = "hello";
    s3.split("el", dest);
    assert(dest.size() == 2);
    i = 0;
    assert(dest[i++] == "h");
    assert(dest[i++] == "lo");
    dest.clear();

    s3 = "helello";
    s3.split("el", dest);
    assert(dest.size() == 3);
    i = 0;
    assert(dest[i++] == "h");
    assert(dest[i++] == "");
    assert(dest[i++] == "lo");

    s3 = "Hello \t\r\nWorld\n  \t\t\nGood\n\f\r\nMorning";
    dest.clear();
    s3.split(dest);
    assert(dest.size() == 4);
    i = 0;
    assert(dest[i++] == "Hello");
    assert(dest[i++] == "World");
    assert(dest[i++] == "Good");
    assert(dest[i++] == "Morning");
    copy(dest.begin(), dest.end(), ostream_iterator<DString>(cout, "\n"));
}
//-------------------------------------------------

#define DSTRTRANS(s1, a1, a2, r) do {               \
        DString dd(s1);                             \
        dd.tr((a1), (a2));                          \
        puts(dd.c_str());                           \
        assert(dd == (r)); } while (0)

#define DSTRTRSQZ(s1, a1, a2, r) do {               \
        DString dd(s1);                             \
        dd.tr_s((a1), (a2));                        \
        puts(dd.c_str());                           \
        assert(dd == (r)); } while (0)

void test_translate()
{
    TRACE_FN();

    DSTRTRANS("Hello Sam!", "S", "P", "Hello Pam!");
    DSTRTRANS("Hi Sam!", "mSa", "eJo", "Hi Joe!");

    DSTRTRANS("hello", "aeiou", "-", "h-ll-");
    DSTRTRANS("hello", "aeiou", "AA-", "hAll-");
    DSTRTRANS("hello world today is sunday", "aeiou", NULL, "hll wrld tdy s sndy");
    DSTRTRANS("hello world today is sunday", "^aeiou", NULL, "eoooaiua");

    DSTRTRANS("hello", "^aeiou", "-", "-e--o");
    DSTRTRANS("hello world today is sunday", "a-z", "A-Z", "HELLO WORLD TODAY IS SUNDAY");

    DSTRTRSQZ("aabbccddeeffgg", "", "abcd", "abcdeeffgg");
    DSTRTRSQZ("hello", "l", "r", "hero");
    DSTRTRSQZ("hello", "el", "-", "h-o");
    DSTRTRSQZ("hello", "el", "hx", "hxo");
}
//-------------------------------------------------

#define ASSERT_PARTITION(str, what, p1, p2, p3) do {  \
    std::vector<DString> v;                           \
    DString(str).partition((what), v);                \
    assert(v.size() == 3);                            \
    assert(v[0] == p1);                               \
    assert(v[1] == p2);                               \
    assert(v[2] == p3);                               \
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

#define ASSERT_RPARTITION(str, what, p1, p2, p3) do {  \
        std::vector<DString> v;                        \
        DString(str).rpartition((what), v);            \
        assert(v.size() == 3);                         \
        assert(v[0] == p1);                            \
        assert(v[1] == p2);                            \
        assert(v[2] == p3);                            \
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

void test_strip()
{
    TRACE_FN();

    DString s("####Hello####");
    s.strip('#');
    assert(s == "Hello");

    s = "#####################";
    s.rstrip('#');
    assert(s == "");

    s = "#####################";
    s.rstrip("#");
    assert(s == "");

    s = " ###Hello###";
    s.strip('#');
    assert(s == " ###Hello");

    s = " \n \t hello\n";
    s.strip('\n');
    assert(s == " \n \t hello");

    s = "\n\n \t hello\n";
    s.strip('\n');
    assert(s == " \t hello");

    s = "www.example.com";
    s.strip("cmow.");
    std::cout << s << "\n";
    assert(s == "example");

    s = " www.example.com";
    s.strip("cmow.");
    assert(s == " www.example");

    s = "www.example.com ";
    s.strip("cmow.");
    std::cout << "\"" << s << "\"\n";
    assert(s == "example.com ");

    s = "Arthur: three!";
    s.lstrip("Arthur: ");
    assert(s == "ee!");
}
//-------------------------------------------------

void test_times()
{
    TRACE_FN();

    DString s("A");
    assert(s.times(5) == "AAAAA");
    assert(s.times(2).times(2) == s.times(4));
    assert(s.times(0) == "");
    assert(s.times(1) == s);
    assert(DString('=', 50) == DString("=").times(50));

    cout << DString('=', 50) << endl;
    cout << "H E L L O   W O R L D" << endl;
    cout << DString("=").times(50) << endl;
}
//-------------------------------------------------

void test_std_string_view()
{
    TRACE_FN();
#if __cplusplus >= 201703L
    DString s1("Hello world today is Friday");
    std::string_view sv = s1;

    cout << s1 << endl;
    cout << sv << endl;

    assert(sv == s1);
    assert(sv.data() == s1.data());

    DString s2(sv);
    assert(s2 == s1);
    assert(s2 == sv);
    assert(s1.data() != s2.data());
#endif
}
//-------------------------------------------------

#define TEST_ZFILL(before, len, after) do {             \
    DString s(before);                                  \
    DString f = s.zfill(len);                           \
    cout << "\"" << f << "\"" << endl;                  \
    assert(f == (after));                               \
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


int main()
{
    test_ctor();
    test_eq();
    test_empty_dstr();
    test_char_ctor();
    test_copy_ctor();
    test_buff_ctor();
    test_range_ctor();
    test_assign();
    test_append();
    test_format();
    test_remove();
    test_trim();
    test_insert();
    test_replace();
    test_replace1();
    test_getline();
    test_fgets();
    test_fromfile();
    test_substr();
    test_left_mid_right();
    test_truncate();
    test_find();
    test_rfind();
    test_put_get();
    test_put_get_safe();
    test_ascii_upper_lower();
    test_reverse();
    test_swap();
    test_ffo();
    test_ffno();
    test_flo();
    test_flno();
    test_prefix();
    test_suffix();
    test_blank();
    test_atoi_itos();
    test_isdigit();
    test_move_ctor_assign();
    test_push_back_front();
    test_pop_back_front();
    test_operator_plus();
    test_conversion();
    test_center();
    test_align();
    test_replace_all();
    test_count();
    test_expandtabs();
    test_title();
    test_join();
    test_split();
    test_translate();
    test_is_identifier();
    test_partition();
    test_rpartition();
    test_operator_square_braces();
    test_strip();
    test_times();
    test_std_string_view();
    test_zfill();
    // last test
}
