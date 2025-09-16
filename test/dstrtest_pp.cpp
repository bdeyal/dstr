#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include <dstr/dstring.hpp>

#if defined(__BORLANDC__) || (defined(_MSC_VER) && (_MSC_VER <= 1200))
#define TRACE_FN() printf("%d\n", __LINE__)
#else
#define TRACE_FN() puts(__FUNCTION__)
#endif

#define TRACE_LN() printf("%d\n", __LINE__)
//-------------------------------------------------

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

    assert(s5 == NULL);
    assert(s1 > NULL);

    assert(s5 == NULL);
    assert(s1 != NULL);

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
    DString s2((char*)NULL, 0);
    DString s3((size_t)0);
    DString s4(100);
    DString s5((char*)NULL);
    DString s6('\0', 100);
    DString s7('C', 0);
    DString s8("", 100);

    assert(s1.length() == 0);
    assert(s2.length() == 0);
    assert(s3.length() == 0);
    assert(s4.length() == 0);
    assert(s5.length() == 0);
    assert(s6.length() == 0);
    assert(s7.length() == 0);
    assert(s8.length() == 0);

    assert(s1.empty());
    assert(s2.empty());
    assert(s3.empty());
    assert(s4.empty());
    assert(s5.empty());
    assert(s6.empty());
    assert(s7.empty());
    assert(s8.empty());

    assert( s1 == s2 );
    assert( s2 == s3 );
    assert( s3 == s4 );
    assert( s4 == s5 );
    assert( s5 == s6 );
    assert( s6 == s7 );
    assert( s7 == s8 );
    assert( s8 == s1 );

    assert(strlen(s1.c_str()) == 0);
    assert(strlen(s2.c_str()) == 0);
    assert(strlen(s3.c_str()) == 0);
    assert(strlen(s4.c_str()) == 0);
    assert(strlen(s5.c_str()) == 0);
    assert(strlen(s6.c_str()) == 0);
    assert(strlen(s7.c_str()) == 0);
    assert(strlen(s8.c_str()) == 0);
}
//-------------------------------------------------

void test_char_ctor()
{
    TRACE_FN();
    DString s1('A', 5);
    assert(s1.length() == 5 );
    assert(s1 == "AAAAA");

    DString s2('B', 1);
    assert(s2.length() == 1 );
    assert(s2 == "B");
    assert(s2[0] == 'B');

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

    DString s3;
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

    DString s2("ABCDEFGH", 0);
    assert(s2.length() == 0);
    assert(s2 == "");

    // Test assign_buff too
    //
    DString s3;
    s3.assign("ABCDEFGH", 5);
    assert(s3.length() == 5);
    assert(s3 == "ABCDE");

    DString s4;
    s4.assign("ABCDEFGH", 0);
    assert(s4.length() == 0);
    assert(s4 == "");

    DString s5("", 10);
    assert(s5 == "");
    assert(s5.length() == 0);
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

    s2 = (char*)NULL;
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

    DString s6(100);
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

    s4.append((char*)NULL);
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

    s5.append((char*)NULL, 10);
    assert(s5 == "hello_ABC");

    s5.append("Hi", 0);
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

    s5.truncate();

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

    for (int i = 0; i < 25; ++i) {
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

    TEST_TRIM_LEFT("H",          "H");
    TEST_TRIM_LEFT("Hello",  "Hello");
    TEST_TRIM_LEFT("      H",        "H");
    TEST_TRIM_LEFT("      Hello", "Hello");
    TEST_TRIM_LEFT("         ",  "");
    TEST_TRIM_LEFT("",           "");

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

    s1.insert(4, (char*)NULL);
    assert(s1 == "hellXXoX");

    s1.insert(0, "XXX", DString::NPOS);
    assert( s1 == "XXXhellXXoX");

    s1.insert(0, (char*)NULL, DString::NPOS);
    assert( s1 == "XXXhellXXoX");

    s1.insert(0, "XXX", 0);
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

    s1.replace(4, 5, NULL);
    assert( s1 == "hellohello");

    s1.replace(5, 5, '\0', 1);
    assert( s1 == "hello" );

    s1.replace(0, DString::NPOS, "world");
    assert( s1 == "world");

    DString s2(s1);
    s1.replace(0, DString::NPOS, (char*)NULL, 100);
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
    using namespace std;

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

    DString d1(500);
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
    using namespace std;

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

    DString d1(500);
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
    using namespace std;

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
    using namespace std;

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

    s1.truncate();
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
        char c = s1.getchar(n);
        s1.putchar(n, toupper(c));
    }
    assert(s1 == str_upper);

    // back to lowercase
    s1.lower();
    assert(s1 == str_lower);

    // Now test the 'safe' part
    //
    size_t index = s1.length() + 5;
    assert(s1.getchar(index) == '\0');
    assert(s1.getchar(-(long)index) == '\0');

    DString s2(s1);
    s2.putchar(index, 'A');
    assert(s1 == s2);
    s2.putchar(-(long)index, 'A');
    assert(s1 == s2);

    for (size_t n = 1; n <= s2.length(); ++n) {
        long i = -(long)n;
        char c = s2.getchar(i);
        assert(c == s2[s2.length() - n]);
        s2.putchar(i, toupper(c));
    }
    assert(s2 == str_upper);
}
//-------------------------------------------------

#define TEST_ASCII_UPPER(before, after) do {                \
        DString s(before);                                  \
        s.upper();                                          \
        assert(s == after);                                 \
    } while (0)
//-------------------------------------------------

#define TEST_ASCII_LOWER(before, after) do {                \
        DString s(before);                                  \
        s.lower();                                          \
        assert( s == after );                               \
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
    DString s1((char*)NULL);
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

void test_prefix()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DString s1(longstr);

    assert( s1.startswith("G"));
    assert( s1.startswith("Good"));
    assert( s1.startswith(longstr));
    assert( !s1.startswith("good"));

    assert( s1.startswith("GOOD MORNING", DString::IgnoreCase));
    assert( s1.startswith("GooD", DString::IgnoreCase));
    assert( s1.startswith("GooD morNiNg Today is Friday", DString::IgnoreCase));
    assert( !s1.startswith("H"));
    assert( !s1.startswith("H", DString::IgnoreCase));

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
    assert( s1.endswith("GooD morNiNg Today is Friday", DString::IgnoreCase));

    assert( s1.endswith("Y", DString::IgnoreCase));
    assert( s1.endswith("DaY", DString::IgnoreCase));
    assert( !s1.endswith("H"));
    assert( !s1.endswith("H", DString::IgnoreCase));
}
//-------------------------------------------------

void test_blank()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DString s1(longstr);

    assert( !s1.isblank() );

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

#define TEST_ITOA(n, result) do {                                       \
        DString s;                                                      \
        s.itoa(n);                                                      \
        printf("Converted = %s, Expected = %s\n", s.c_str(), result);   \
        assert(s == result);                                            \
    } while (0)
//-------------------------------------------------

void test_atoi_itoa()
{
    TRACE_FN();

    TEST_ATOI("1", 1L);
    TEST_ATOI("\\1234", 668L);
    TEST_ATOI("0b111001", 57L);
    TEST_ATOI("0xFFFF", 65535L);

    TEST_ITOA(1, "1");
    TEST_ITOA(-1, "-1");
    TEST_ITOA(1234, "1234");
    TEST_ITOA(0, "0");
    TEST_ITOA(-1234, "-1234");
}
//-------------------------------------------------

#define TEST_DIGIT(s, result) do {                          \
        DString d(s);                                       \
        assert( d.isdigits() == result );                   \
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


int main()
{
    test_ctor();
    test_eq();
    test_empty_dstr();
    test_char_ctor();
    test_copy_ctor();
    test_buff_ctor();
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
    test_truncate();
    test_find();
    test_put_get();
    test_put_get_safe();
    test_ascii_upper_lower();
    test_reverse();
    test_swap();
    test_ffo();
    test_ffno();
    test_prefix();
    test_suffix();
    test_blank();
    test_atoi_itoa();
    test_isdigit();
    test_move_ctor_assign();
    test_push_back_front();
    test_pop_back_front();
    // last test
}
