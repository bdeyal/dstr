/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <dstr/dstr.h>


#if defined(__BORLANDC__) || (defined(_MSC_VER) && (_MSC_VER <= 1200))
#define TRACE_FN() printf("%d\n", __LINE__)
#else
#define TRACE_FN() puts(__FUNCTION__)
#endif

#define TRACE_LN() printf("%d\n", __LINE__)

#define TRACE_DS(s) do {                                \
        printf("LINE = %d, ", __LINE__);                \
        printf("DS name = %s, ", #s);                   \
        printf("CAPACITY = %zu, ", dstrcap(s));         \
        printf("LEN = %zu, ", dstrlen(s));              \
        printf("ADDR = %p, ", (s));                     \
        printf("BUFF = %p, ", dstrdata(s));             \
        printf("Contents = %s\n", dstrdata(s));         \
    } while (0)
//-------------------------------------------------

void test_ctor()
{
    TRACE_FN();

    DSTR s1 = dstrnew("hello");
    TRACE_DS(s1);
    assert(dstrlen(s1) == strlen("hello"));
    assert(strcmp(dstrdata(s1), "hello") == 0);

    DSTR s2 = dstrnew_empty();
    TRACE_DS(s2);
    assert(dstrlen(s2) == 0);

    TRACE_DS(s1);
    dstrfree(s1);
    dstrfree(s2);
}
//-------------------------------------------------

void test_eq()
{
    TRACE_FN();
    DSTR s1 = dstrnew("hello");
    DSTR s2 = dstrnew("hello");
    DSTR s3 = dstrnew("HELLO");
    DSTR s4 = dstrnew("Hello World");
    DSTR s5 = dstrnew_empty();

    assert(dstreq_ds(s1, s2));
    assert(dstreq(s1, "hello"));
    assert(!dstreq_ds(s1, s3));
    assert(!dstreq(s1, "ABC"));
    assert(!dstreq_ds(s1, s4));

    assert(dstrcmp_ds(s1, s2) == 0);
    assert(dstrcmp(s1, "hello") == 0);
    assert(dstrcmp_ds(s1, s3) != 0);
    assert(dstrcmp(s1, "ABC") != 0);

    assert(dstrcmp(s5, NULL) == 0);
    assert(dstrcmp(s1, NULL) > 0);

    assert(dstreq(s5, NULL));
    assert(!dstreq(s1, NULL));

    assert(dstreq_i(s3, "hello"));
    assert(dstreq_i(s3, "heLLo"));
    assert(dstreq_i(s3, "hEllo"));
    assert(!dstreq_i(s3, "Xhello"));

    dstrfree(s1);
    dstrfree(s2);
    dstrfree(s3);
    dstrfree(s4);
    dstrfree(s5);
}
//-------------------------------------------------

void test_empty_dstr()
{
    TRACE_FN();

    DSTR s1 = dstrnew_empty();
    DSTR s2 = dstrnew_bl(NULL, 0);
    DSTR s3 = dstrnew_reserve(0);
    DSTR s4 = dstrnew_reserve(100);
    DSTR s5 = dstrnew(NULL);
    DSTR s6 = dstrnew_cc('\0', 100);
    DSTR s7 = dstrnew_cc('C', 0);
    DSTR s8 = dstrnew_bl("", 100);

    assert(dstrlen(s1) == 0);
    assert(dstrlen(s2) == 0);
    assert(dstrlen(s3) == 0);
    assert(dstrlen(s4) == 0);
    assert(dstrlen(s5) == 0);
    assert(dstrlen(s6) == 0);
    assert(dstrlen(s7) == 0);
    assert(dstrlen(s8) == 0);

    assert(dstrempty(s1));
    assert(dstrempty(s2));
    assert(dstrempty(s3));
    assert(dstrempty(s4));
    assert(dstrempty(s5));
    assert(dstrempty(s6));
    assert(dstrempty(s7));
    assert(dstrempty(s8));

    assert( dstreq_ds(s1, s2) );
    assert( dstreq_ds(s2, s3) );
    assert( dstreq_ds(s3, s4) );
    assert( dstreq_ds(s4, s5) );
    assert( dstreq_ds(s5, s6) );
    assert( dstreq_ds(s6, s7) );
    assert( dstreq_ds(s7, s8) );
    assert( dstreq_ds(s8, s1) );

    assert(strlen(dstrdata(s1)) == 0);
    assert(strlen(dstrdata(s2)) == 0);
    assert(strlen(dstrdata(s3)) == 0);
    assert(strlen(dstrdata(s4)) == 0);
    assert(strlen(dstrdata(s5)) == 0);
    assert(strlen(dstrdata(s6)) == 0);
    assert(strlen(dstrdata(s7)) == 0);
    assert(strlen(dstrdata(s8)) == 0);

    dstrfree(s1);
    dstrfree(s2);
    dstrfree(s3);
    dstrfree(s4);
    dstrfree(s5);
    dstrfree(s6);
    dstrfree(s7);
    dstrfree(s8);
}
//-------------------------------------------------

void test_char_ctor()
{
    TRACE_FN();
    DSTR s1 = dstrnew_cc('A', 5);
    assert( dstrlen(s1) == 5 );
    assert( dstreq(s1, "AAAAA") );

    DSTR s2 = dstrnew_cc('B', 1);
    assert( dstrlen(s2) == 1 );
    assert( dstreq(s2, "B") );
    assert( dstrgetc(s2, 0) == 'B' );

    DSTR s3 = dstrnew_cc('\0', 5);
    assert( dstrlen(s3) == 0 );
    assert( dstreq(s3, "") );

    dstrfree(s1);
    dstrfree(s2);
    dstrfree(s3);
}
//-------------------------------------------------

void test_copy_ctor()
{
    TRACE_FN();
    DSTR s1 = dstrnew("hello");
    DSTR s2 = dstrnew_ds(s1);

    assert(dstreq_ds(s1, s2));
    assert(dstreq(s2, "hello"));

    DSTR s3 = dstrnew_empty();
    DSTR s4 = dstrnew_ds(s3);
    assert( dstreq_ds(s3, s4));

    dstrfree(s1);
    dstrfree(s2);
    dstrfree(s3);
    dstrfree(s4);
}
//-------------------------------------------------

void test_buff_ctor()
{
    TRACE_FN();

    DSTR s1 = dstrnew_bl("ABCDEFGH", 5);
    assert(dstrlen(s1) == 5);
    assert(dstreq(s1, "ABCDE"));

    DSTR s2 = dstrnew_bl("ABCDEFGH", 0);
    assert( dstrlen(s2) == 0);
    assert( dstreq(s2,""));

    // Test assign_buff too
    //
    DSTR s3 = dstrnew_empty();
    dstrcpy_bl(s3, "ABCDEFGH", 5);
    assert(dstrlen(s3) == 5);
    assert(dstreq(s3, "ABCDE"));

    DSTR s4 = dstrnew_empty();
    dstrcpy_bl(s4, "ABCDEFGH", 0);
    assert(dstrlen(s4) == 0);
    assert(dstreq(s4, ""));

    // failed
    DSTR s5 = dstrnew_bl("", 10);
    assert( dstreq(s5, "") );
    assert(dstrlen(s5) == 0);

    dstrfree(s1);
    dstrfree(s2);
    dstrfree(s3);
    dstrfree(s4);
    dstrfree(s5);
}
//-------------------------------------------------

void test_range_ctor()
{
    TRACE_FN();

    const char* cstr = "ABCDEFGH";
    DSTR s1 = dstrnew_rng(cstr, cstr + 5);
    assert(dstrlen(s1) == 5);
    assert(dstreq(s1, "ABCDE"));

    DSTR s2 = dstrnew_empty();
    dstrcpy_rng(s2, cstr, cstr + 5);
    assert(dstreq_ds(s1, s2));

    // Test assign_buff too
    //
    DSTR s3 = dstrnew("Hello");
    dstrcat_rng(s3, cstr, cstr + 3);
    assert(dstrlen(s3) == 8);
    assert(dstreq(s3, "HelloABC"));

    DSTR s4 = dstrnew("Hello  World");
    dinsert_rng(s4, 6, cstr, cstr+3);
    assert(dstrlen(s4) == 15);
    assert(dstreq(s4, "Hello ABC World"));

    dreplace_rng(s4, 6, 3, cstr + 3, cstr + 8);
    assert(dstreq(s4, "Hello DEFGH World"));

    dstrfree(s1);
    dstrfree(s2);
    dstrfree(s3);
    dstrfree(s4);
}
//-------------------------------------------------

void test_assign()
{
    TRACE_FN();

    DSTR s1 = dstrnew_empty();
    dstrcpy(s1, "hello");
    assert( dstreq(s1, "hello"));
    assert( dstrlen(s1) == strlen("hello"));

    DSTR s2 = dstrnew_empty();
    dstrcpy_ds(s2, s1);
    assert( dstreq_ds(s1, s2));
    assert( dstreq(s2, "hello"));
    assert( dstrlen(s2) == strlen("hello"));

    dstrcpy_cc(s2, 'C', 8);
    assert( dstrlen(s2) == 8);
    assert( dstreq(s2, "CCCCCCCC"));
    assert( strcmp(dstrdata(s2), "CCCCCCCC") == 0);

    dstrcpy(s2, NULL);
    assert(dstrlen(s2) == 0);

    DSTR s3 = dstrnew("hello");
    DSTR s4 = dstrnew_empty();
    dstrcpy_ds(s3, s4);
    assert( dstreq_ds(s3, s4) );
    assert( dstrlen(s3) == 0 );

    DSTR s5 = dstrnew("hello");
    dstrcpy(s5, "world");
    assert( dstreq(s5, "world") );
    assert( dstrlen(s5) == strlen("world") );

    DSTR s6 = dstrnew_reserve(100);
    dstrcpy_substr(s6, s5, 0, DSTR_NPOS);
    assert( dstreq_ds(s5, s6));

    dstrcpy_substr(s6, s5, 0, 2);
    assert( dstreq(s6, "wo"));

    dstrcpy_substr(s6, s5, 1, 3);
    assert( dstreq(s6, "orl"));

    dstrcpy_substr(s6, s5, 3, 2);
    assert( dstreq(s6, "ld"));

    dstrcpy_substr(s6, s5, 3, 100);
    assert( dstreq(s6, "ld"));

    dstrfree(s1);
    dstrfree(s2);
    dstrfree(s3);
    dstrfree(s4);
    dstrfree(s5);
    dstrfree(s6);
}
//-------------------------------------------------

void test_fromfile()
{
    TRACE_FN();
    DSTR s1 = dstrnew_slurp("a_no_exist_file");
    assert(s1 == NULL);
    assert(errno == ENOENT);

    s1 = dstrnew_slurp(__FILE__);
    if (!s1) {
        perror("Error open file");
        return;
    }

    printf("File %s read length = %zu\n", __FILE__, dstrlen(s1));
    dstrfree(s1);
}
//-------------------------------------------------

void test_append()
{
    TRACE_FN();

    DSTR s1, s2, s3;

    s1 = dstrnew("hello");
    s2 = dstrnew("world");
    s3 = dstrnew_ds(s1);
    dstrcat_ds(s3, s2);

    assert(dstreq(s3, "helloworld"));
    assert(dstrlen(s3) == dstrlen(s2) + dstrlen(s1));

    DSTR s4 = dstrnew_empty();
    dstrcpy_ds(s4, s1);
    dstrcat(s4, "world");
    assert( dstreq(s4, "helloworld") );

    dstrcat(s4, NULL);
    assert( dstreq(s4, "helloworld") );

    dstrcat(s4, "");
    assert( dstreq(s4, "helloworld") );

    DSTR s5 = dstrnew_empty();
    dstrcat_cc(s5, 'h', 1);
    assert( dstreq(s5, "h") );

    dstrcat_c(s5, 'e');
    dstrcat_cc(s5, 'l', 2);
    dstrcat_cc(s5, 'o', 1);
    assert( dstreq(s5, "hello"));

    dstrcat_c(s5, '\0');
    assert( dstreq(s5, "hello"));

    dstrcat_bl(s5, "_ABC\0EFG", 10);
    assert( dstreq(s5, "hello_ABC"));

    dstrcat_bl(s5, NULL, 10);
    assert( dstreq(s5, "hello_ABC"));

    dstrcat_bl(s5, "Hi", 0);
    assert( dstreq(s5, "hello_ABC"));

    dstrcat_bl(s5, "_DEF", 3);
    assert( dstreq(s5, "hello_ABC_DE"));

    dstrcat_bl(s5, "FGH", 1);
    assert( dstreq(s5, "hello_ABC_DEF"));

    dstrcat_ds(s5, s5);
    assert( dstreq(s5, "hello_ABC_DEFhello_ABC_DEF"));

    int step = 1000;
    int limit = 10 * step;
    for (int i = 0; i < limit; ++i) {
        if ((i % step) == 0) {
            printf("step: %d LEN=%zu, CAP=%zu\n", i, dstrlen(s5), dstrcap(s5));
        }
        dstrcat_c(s5, 'X');
    }

    dstrclear(s5);

    for (int i = 0; i < limit; ++i) {
        if ((i % step) == 0) {
            printf("step: %d LEN=%zu, CAP=%zu\n", i, dstrlen(s5), dstrcap(s5));
        }
        dstrcat(s5, "hello");
    }

    dstrclear(s5);
    dstrcat(s5, "hello");

    for (int i = 0; i < 20; ++i) {
        printf("step: %d LEN=%zu, CAP=%zu\n", i, dstrlen(s5), dstrcap(s5));
        if (!dstrcat_ds(s5, s5)) {
            perror("dstrcat_ds");
            break;
        }
    }

    putchar('\n');

    dstrfree(s1);
    dstrfree(s2);
    dstrfree(s3);
    dstrfree(s4);
    dstrfree(s5);
}
//-------------------------------------------------

void test_format()
{
    TRACE_FN();

    DSTR res1;
    char res2[4096];

    // simple int
    //
    res1 = dstrnew_sprintf("%d", 100);
    sprintf(res2, "%d", 100);
    printf("res1=\"%s\", res2=\"%s\"\n", dstrdata(res1), res2);
    assert( dstreq(res1, res2) );
    TRACE_LN();

    // simple double
    //
    dstrclear(res1);
    dsprintf(res1, "%g", 100.0);
    sprintf(res2, "%g", 100.0);
    assert( dstreq(res1, res2) );
    TRACE_LN();

    // hex
    //
    dsprintf(res1, "%08X", 100);
    sprintf(res2, "%08X", 100);
    assert( dstreq(res1, res2) );
    TRACE_LN();

    // long string
    //
    char buffer[2048];
    memset(buffer, 'a', sizeof buffer);
    buffer[sizeof(buffer) - 1] = '\0';
    TRACE_LN();

    dsprintf(res1, "%s", buffer);
    sprintf(res2, "%s", buffer);
    assert( dstreq(res1, res2) );
    TRACE_LN();

    // Complicated format
    //
    dsprintf(res1, "%0X %g %10s %10d %-20s", 100, 4.55, "Hello World", 10, "A_String");
    sprintf(res2, "%0X %g %10s %10d %-20s", 100, 4.55, "Hello World", 10, "A_String");
    assert( dstreq(res1, res2) );
    TRACE_LN();

    dsprintf(res1, "%s", "Hello ");
    dstrcat_sprintf(res1, "%s", "World");
    sprintf(res2, "%s", "Hello World");
    assert(dstreq(res1, res2 ));
    TRACE_LN();

    dstrfree(res1);
    TRACE_LN();
}
//-------------------------------------------------

#define TEST_TRIM_LEFT(before, after) do {                  \
        DSTR s = dstrnew(before);                           \
        dstrtrim_l(s);                                      \
        assert( dstreq(s, after) );                      \
        dstrfree(s);                                        \
    } while (0)
//-------------------------------------------------

#define TEST_TRIM_RIGHT(before, after) do {                 \
        DSTR s = dstrnew(before);                           \
        dstrtrim_r(s);                                      \
        assert( dstreq(s, after) );                      \
        dstrfree(s);                                        \
    } while (0)
//-------------------------------------------------

#define TEST_TRIM_BOTH(before, after) do {                  \
        DSTR s = dstrnew(before);                           \
        dstrtrim(s);                                        \
        assert( dstreq(s, after) );                      \
        dstrfree(s);                                        \
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

void test_remove()
{
    TRACE_FN();

    const char* hello = "XhellXXoXXX";
    DSTR s1 = dstrnew_empty();
    dstrcpy(s1, hello);
    assert( dstreq(s1, hello));

    // erase at loc 1000 -> NOOP
    dstrerase(s1, 1000, 1);
    assert( dstreq(s1, hello));

    dstrerase(s1, 0, 1);
    assert( dstreq(s1, "hellXXoXXX"));

    dstrerase(s1, 4, 2);
    assert( dstreq(s1, "helloXXX"));

    dstrerase(s1, 5, 3);
    assert( dstreq(s1, "hello"));

    dstrerase(s1, 4, 100);
    assert( dstreq(s1, "hell") );

    dstrerase(s1, 2, 0);
    assert( dstreq(s1, "hell") );

    dstrerase(s1, 0, DSTR_NPOS);
    assert( dstreq(s1, ""));

    dstrfree(s1);
}
//-------------------------------------------------

void test_insert()
{
    TRACE_FN();

    const char* hello = "hello";
    DSTR s1 = dstrnew_empty();
    dstrcpy(s1, hello);
    assert( dstreq(s1, hello));

    dinsert_cc(s1, 1000, 'X', 1);
    assert( dstreq(s1, "helloX"));

    dinsert(s1, 4, "XX");
    assert( dstreq(s1, "hellXXoX"));

    dinsert(s1, 4, "");
    assert( dstreq(s1, "hellXXoX"));

    dinsert(s1, 4, NULL);
    assert( dstreq(s1, "hellXXoX"));

    dinsert_bl(s1, 0, "XXX", DSTR_NPOS);
    assert( dstreq(s1, "XXXhellXXoX"));

    dinsert_bl(s1, 0, NULL, DSTR_NPOS);
    assert( dstreq(s1, "XXXhellXXoX"));

    dinsert_bl(s1, 0, "XXX", 0);
    assert( dstreq(s1, "XXXhellXXoX"));

    dinsert_cc(s1, 0, 'A', 3);
    assert( dstreq(s1, "AAAXXXhellXXoX"));

    dinsert_cc(s1, 1000, 'B', 2);
    assert( dstreq(s1, "AAAXXXhellXXoXBB"));

    dstrcpy(s1, "ABCDE");
    dinsert_cc(s1, 3, 'Z', 5);
    assert( dstreq(s1, "ABCZZZZZDE"));

    DSTR s2 = dstrnew_cc('X', 2);
    dinsert_ds(s1, 2, s2);
    assert( dstreq(s1, "ABXXCZZZZZDE"));

    dstrcpy(s2, "0123456789");
    dinsert(s2, dstrlen(s2), "ABCDE");
    assert( dstreq(s2, "0123456789ABCDE"));

    dstrfree(s1);
    dstrfree(s2);
}
//-------------------------------------------------

void test_replace1()
{
    TRACE_FN();

    DSTR s1 = dstrnew("hell@@@@@oXXX");
    DSTR sn = dstrnew("hello");
    dreplace_ds(s1, 10, 5, sn);
    assert( dstreq(s1, "hell@@@@@ohello"));

    dstrfree(s1);
    dstrfree(sn);
}
//-------------------------------------------------

void test_replace()
{
    TRACE_FN();

    const char* hello = "XhellXXoXXX";
    DSTR s1 = dstrnew_empty();
    dstrcpy(s1, hello);
    assert( dstreq(s1, hello));

    dreplace(s1, 0, 1, "");
    assert( dstreq(s1, "hellXXoXXX"));

    dreplace(s1, 4, 2, "@@@@@");
    assert( dstreq(s1, "hell@@@@@oXXX"));

    DSTR sn = dstrnew("hello");
    dreplace_ds(s1, 10, 5, sn);
    assert( dstreq(s1, "hell@@@@@ohello"));

    dreplace(s1, 4, 5, NULL);
    assert( dstreq(s1, "hellohello"));


    dreplace_cc(s1, 5, 5, '\0', 1);
    assert( dstreq(s1, "hello") );

    dreplace(s1, 0, DSTR_NPOS, "world");
    assert( dstreq(s1, "world"));

    DSTR s2 = dstrnew_ds(s1);
    dreplace_bl(s1, 0, DSTR_NPOS, NULL, 100);
    assert( dstreq(s1, ""));

    dreplace_cc(s2, 2, 8,'\0', 100);
    assert( dstreq(s2, "wo"));

    dreplace_cc(s2, 0, 2,'\0', 100);
    assert( dstreq(s2, ""));

    DSTR s3 = dstrnew("H World");
    const char* ps3 = dstrdata(s3) + dstrstr(s3, 0, "World");
    dreplace(s3, 0, 1, ps3);
    printf("%s\n", dstrdata(s3));
    assert( dstreq(s3, "World World"));

    dstrfree(sn);
    dstrfree(s1);
    dstrfree(s2);
    dstrfree(s3);
}
//-------------------------------------------------

void test_left_mid_right()
{
    TRACE_FN();

    DSTR src = dstrnew("ABCD EFGH IJKL MNOP QRST UVWX YZ");
    DSTR dest = dstrnew_empty();

    dstr_assign_left(dest, src, 4);
    assert(dstreq(dest, "ABCD"));

    dstr_assign_left(dest, src, 9);
    assert(dstreq(dest, "ABCD EFGH"));

    dstr_assign_left(dest, src, 20);
    assert(dstreq(dest, "ABCD EFGH IJKL MNOP "));

    dstr_assign_left(dest, src, 100);
    assert(dstreq_ds(dest, src));

    dstr_assign_right(dest, src, 4);
    assert(dstreq(dest, "X YZ"));

    dstr_assign_right(dest, src, 7);
    assert(dstreq(dest, "UVWX YZ"));

    dstr_assign_right(dest, src, 23);
    assert(dstreq(dest, " IJKL MNOP QRST UVWX YZ"));

    dstr_assign_right(dest, src, 100);
    assert(dstreq_ds(dest, src));

    dstr_assign_mid(dest, src, 5, 4);
    assert(dstreq(dest, "EFGH"));

    dstr_assign_mid(dest, src, 5, 20);
    assert(dstreq(dest, "EFGH IJKL MNOP QRST "));

    dstr_assign_mid(dest, src, 4, 1000);
    assert(dstreq(dest, " EFGH IJKL MNOP QRST UVWX YZ"));

    dstr_assign_mid(dest, src, 0, 100);
    assert(dstreq_ds(dest, src));

    dstrfree(dest);
    dstrfree(src);
}
//-------------------------------------------------

void test_substr()
{
    TRACE_FN();

    char buff[2048];
    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew(longstr);

    // verifiy that no byte was written
    size_t n = dsubstr_bl(s1, 100, DSTR_NPOS, buff, 5);
    assert(n == 0);

    n = dsubstr_bl(s1, 0, DSTR_NPOS, buff, 0);
    assert(n == 0);

    n = dsubstr_bl(s1, 0, DSTR_NPOS, buff, 1);
    assert(n == 1);
    assert(strcmp(buff, "") == 0);

    n = dsubstr_bl(s1, 0, 0, buff, 100);
    assert(n == 0);

    n = dsubstr_bl(s1, 0, DSTR_NPOS, buff, sizeof(buff));
    assert(n == strlen(longstr) + 1);
    assert(strcmp(buff, longstr) == 0);

    n = dsubstr_bl(s1, 0, DSTR_NPOS, buff, 5);
    assert(n == 5);
    assert(strcmp(buff, "Good") == 0);

    n = dsubstr_bl(s1, 15, DSTR_NPOS, buff, 10);
    assert(n == 10);
    assert(strcmp(buff, "day is Fr") == 0);

    DSTR s2 = dstrnew_substr(s1, 5, 7);
    assert( dstreq(s2, "morning") );
    dstrfree(s2);

    s2 = dstrnew_substr(s1, 0, 4);
    assert( dstreq(s2, "Good") );
    dstrfree(s2);

    s2 = dstrnew_substr(s1, 15, 10);
    assert( dstreq(s2, "day is Fri") );
    dstrfree(s2);

    dstrfree(s1);
}
//-------------------------------------------------

void test_truncate()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew(longstr);

    dstrresize(s1, 100);
    assert( dstreq(s1, longstr) );

    dstrresize(s1, 10);
    assert( strncmp(dstrdata(s1), longstr, 10) == 0);

    dstrclear(s1);
    assert( dstrlen(s1) == 0);
    assert( dstreq(s1,""));

    dstrfree(s1);
}
//-------------------------------------------------

void test_shrink()
{
    TRACE_FN();

    const char* shortstr = "Good morning";
    DSTR s1 = dstrnew_reserve(1000);

    dstrcpy(s1, shortstr);
    assert(dstrcap(s1) >= 1000);
    assert(dstreq(s1, shortstr));

    dstrshrink(s1);
    assert(dstrcap(s1) == DSTR_INITIAL_CAPACITY);
    assert(dstreq(s1, shortstr));

    dstrfree(s1);
}
//-------------------------------------------------

void test_find()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew(longstr);

    assert( dstrstr(s1, 0, "good") == DSTR_NPOS);
    assert( dstrstr(s1, 0, "Good") == 0);
    assert( dstrstr(s1, 0, "morning") == 5);
    assert( dstrstr(s1, 6, "morning") == DSTR_NPOS);
    assert( dstrstr(s1, 100, "morning") == DSTR_NPOS);
    assert( dstrstr(s1, 0, "day") == 15);
    assert( dstrstr(s1, 3, "date") == DSTR_NPOS);
    assert( dstrhas(s1, "is"));
    assert( dstrhas(s1, "morn"));
    assert( !dstrhas(s1, "XXX"));

    assert( dstrstr_i(s1, 0, "good") == 0);
    assert( dstrstr_i(s1, 1, "good") == DSTR_NPOS);
    assert( dstrstr_i(s1, 0, "GOOD") == 0);
    assert( dstrstr_i(s1, 0, "MoRnInG") == 5);
    assert( dstrstr_i(s1, 6, "MoRnInG") == DSTR_NPOS);
    assert( dstrstr_i(s1, 1000, "MoRnInG") == DSTR_NPOS);
    assert( dstrstr_i(s1, 8, "DAY") == 15);
    assert( dstrstr_i(s1, 0, "Date") == DSTR_NPOS);
    assert( dstrhas_i(s1, "IS"));
    assert( dstrhas_i(s1, "MoRn"));
    assert( !dstrhas_i(s1, "xXx"));

    assert( dstrchr(s1, 0, 'g') == 11);
    assert( dstrchr(s1, 1, 'g') == 11);
    assert( dstrchr(s1, 2, 'g') == 11);
    assert( dstrchr(s1, 11, 'g') == 11);
    assert( dstrchr(s1, 12, 'g') == DSTR_NPOS);
    assert( dstrchr(s1, 100, 'g') == DSTR_NPOS);

    assert( dstrchr(s1, 0, 'G') == 0);
    assert( dstrchr(s1, 0, 'm') == 5);
    assert( dstrchr(s1, 1, 'd') == 3);
    assert( dstrchr(s1, 0, 'X') == DSTR_NPOS);

    assert( dstrchr_i(s1, 0, 'G') == 0);
    assert( dstrchr_i(s1, 0, 'g') == 0);
    assert( dstrchr_i(s1, 0, 'm') == 5);
    assert( dstrchr_i(s1, 3, 'M') == 5);
    assert( dstrchr_i(s1, 15, 'f') == 22);
    assert( dstrchr_i(s1, 100, 'f') == DSTR_NPOS);
    assert( dstrchr_i(s1, 0, 'X') == DSTR_NPOS);

    dstrfree(s1);
}
//-------------------------------------------------

void test_rfind()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew(longstr);

    assert( dstrrstr(s1, 0, "good") == DSTR_NPOS);
    printf("%zu\n", dstrrstr(s1, 1000, "Good"));
    assert( dstrrstr(s1, 1000, "Good") == 0);
    assert( dstrrstr(s1, 1000, "morning") == 5);
    assert( dstrrstr(s1, 4, "morning") == DSTR_NPOS);
    assert( dstrrstr(s1, 100, "morning") == 5);
    assert( dstrrstr(s1, 1000, "day") == 25);
    assert( dstrrstr(s1, 3, "date") == DSTR_NPOS);

    assert( dstrrstr_i(s1, 1000, "good") == 0);
    assert( dstrrstr_i(s1, 1000, "GOOD") == 0);
    assert( dstrrstr_i(s1, 1000, "MoRnInG") == 5);
    assert( dstrrstr_i(s1, 2, "MoRnInG") == DSTR_NPOS);
    assert( dstrrstr_i(s1, 0, "MoRnInG") == DSTR_NPOS);
    assert( dstrrstr_i(s1, 1000, "DAY") == 25);
    assert( dstrrstr_i(s1, 18, "DAY") == 15);
    assert( dstrrstr_i(s1, 100, "Date") == DSTR_NPOS);

    assert( dstrrchr(s1, 1000, 'g') == 11);
    assert( dstrrchr(s1, 1000, 'y') == 27);
    assert( dstrrchr(s1, 2, 'g') == DSTR_NPOS);
    assert( dstrrchr(s1, 11, 'g') == 11);
    assert( dstrrchr(s1, 12, 'g') == 11);

    assert( dstrrchr(s1, 0, 'X') == DSTR_NPOS);

    assert( dstrrchr_i(s1, 100, 'G') == 11);
    assert( dstrrchr_i(s1, 100, 'g') == 11);
    assert( dstrrchr_i(s1, 100, 'm') == 5);
    assert( dstrrchr_i(s1, 100, 'M') == 5);
    assert( dstrrchr_i(s1, 100, 'f') == 22);
    assert( dstrrchr_i(s1, 100, 'f') == 22);
    assert( dstrrchr_i(s1, 0, 'X') == DSTR_NPOS);

    dstrcpy(s1, "");
    assert( dstrrchr_i(s1, 100, 'X') == DSTR_NPOS);
    assert( dstrrstr_i(s1, 1000, "good") == DSTR_NPOS);
    assert( dstrrchr(s1, 100, 'X') == DSTR_NPOS);
    assert( dstrrstr(s1, 1000, "good") == DSTR_NPOS);
    assert( dstrrstr(s1, 1000, "") == 0);

    dstrcpy(s1, "XX");
    assert( dstrrstr(s1, 1000, "") == dstrlen(s1));

    dstrfree(s1);
}
//-------------------------------------------------

void test_put_get()
{
    TRACE_FN();

    const char * str_lower = "abcdefghijklmnopqrstuvwxyz";
    const char* str_upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    DSTR s1 = dstrnew(str_lower);

    for (size_t n = 0; n < dstrlen(s1); ++n) {
        assert(dindex_ok(s1,n));
        char c = dstrgetc(s1, n);
        c = toupper(c);
        assert(c == str_upper[n]);
        dstrputc(s1, n, c);
    }

    assert(!dindex_ok(s1, dstrlen(s1)));
    assert(!dindex_ok(s1, DSTR_NPOS));
    assert(dstreq(s1, str_upper));

    dstrfree(s1);
}
//-------------------------------------------------

void test_put_get_safe()
{
    TRACE_FN();

    const char * str_lower = "abcdefghijklmnopqrstuvwxyz";
    const char* str_upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    DSTR s1 = dstrnew(str_lower);

    // first we must support the non-safe semantics
    //
    for (size_t n = 0; n < dstrlen(s1); ++n) {
        assert(dindex_ok(s1,n));
        char c = dstrgetc_s(s1, n);
        dstrputc_s(s1, n, toupper(c));
    }
    assert(!dindex_ok(s1, dstrlen(s1)));
    assert(!dindex_ok(s1, DSTR_NPOS));
    assert(dstreq(s1, str_upper));

    // back to lowercase
    dstrlower(s1);
    assert(dstreq(s1, str_lower));

    // Now test the 'safe' part
    //
    size_t index = dstrlen(s1) + 5;
    assert(dstrgetc_s(s1, index) == '\0');
    assert(dstrgetc_s(s1, - (long)index) == '\0');

    DSTR s2 = dstrnew_ds(s1);
    dstrputc_s(s2, index, 'A');
    assert(dstreq_ds(s1, s2));
    dstrputc_s(s2, - (long)index, 'A');
    assert(dstreq_ds(s1, s2));

    for (size_t n = 1; n <= dstrlen(s2); ++n) {
        long i = - (long)n;
        char c = dstrgetc_s(s2, i);
        assert(c == dstrgetc(s2, dstrlen(s2) - n));
        dstrputc_s(s2, i, toupper(c));
    }
    assert(dstreq(s2, str_upper));

    dstrfree(s1);
    dstrfree(s2);
}
//-------------------------------------------------

void test_getline()
{
    TRACE_FN();

    FILE* fp = fopen(__FILE__, "r");
    if (!fp) {
        fprintf(stderr, "couldn't open file %s: %s\n", __FILE__, strerror(errno));
        return;
    }

    DSTR s1 = dstrnew_reserve(100);
    int line_count = 0;

    size_t maxlen = 0;
    while (dgetline(s1, fp) != EOF) {
        ++line_count;
        if (dstrlen(s1) > maxlen)
            maxlen = dstrlen(s1);
    }
    fclose(fp);
    dstrfree(s1);

    printf("Line Count of %s is %d, maxlen = %zu\n", __FILE__, line_count, maxlen);
}
//-------------------------------------------------

void test_fgets()
{
    TRACE_FN();

    FILE* fp = fopen(__FILE__, "r");
    if (!fp) {
        fprintf(stderr, "couldn't open %s: %s\n", __FILE__, strerror(errno));
        return;
    }

    DSTR s1 = dstrnew_reserve(500);
    assert(dstrcap(s1) >= 500);

    size_t maxlen = 0;
    while (dgets(s1, fp) != EOF) {
        if (dstrlen(s1) > maxlen)
            maxlen = dstrlen(s1);
    }
    printf("dstr_fgets: maxlen = %zu\n", maxlen);

    dstrfree(s1);
    fclose(fp);
}
//-------------------------------------------------

#define TEST_ASCII_UPPER(before, after) do {                \
        DSTR s = dstrnew(before);                           \
        dstrupper(s);                                       \
        assert( dstreq(s, after) );                      \
        dstrfree(s);                                        \
    } while (0)
//-------------------------------------------------

#define TEST_ASCII_LOWER(before, after) do {                \
        DSTR s = dstrnew(before);                           \
        dstrlower(s);                                       \
        assert( dstreq(s, after) );                      \
        dstrfree(s);                                        \
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

#define TEST_REVERSE(before, after) do {                    \
        DSTR s = dstrnew(before);                           \
        dstrrev(s);                                         \
        assert( dstreq(s, after) );                      \
        dstrfree(s);                                        \
    } while (0)
//-------------------------------------------------

void test_reverse()
{
    TRACE_FN();

    TEST_REVERSE("A", "A");
    TEST_REVERSE("AB", "BA");
    TEST_REVERSE("ABC", "CBA");
    TEST_REVERSE("ABCD", "DCBA");
    TEST_REVERSE("Good Morning", "gninroM dooG");

    DSTR empty = dstrnew_empty();
    dstrrev(empty);
    dstrfree(empty);
}
//-------------------------------------------------

void test_swap()
{
    TRACE_FN();

    DSTR s1 = dstrnew(NULL);
    DSTR s2 = dstrnew("hello");
    printf("\"%s\"\n", dstrdata(s1));
    printf("\"%s\"\n", dstrdata(s2));

    dstrswap(s1, s2);
    printf("\"%s\"\n", dstrdata(s1));
    printf("\"%s\"\n", dstrdata(s2));
    assert( dstrempty(s2) );
    assert( dstreq(s1, "hello") );

    dstrfree(s1);
    dstrfree(s2);
}
//-------------------------------------------------

void test_ffo()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew(longstr);

    size_t res = dstrffo(s1, 0, " \t");
    assert(res == 4);

    res = dstrffo(s1, 5, " \t");
    assert(res == 12);

    res = dstrffo(s1, 1000, " \t");
    assert(res == DSTR_NPOS);

    DSTR rhs = dstrnew("Frdy");
    res = dstrffo_ds(s1, 0, rhs);
    assert(res == 3);

    DSTR comment = dstrnew("# comment");
    res = dstrffo(comment, 0, "#;");
    assert(res == 0);

    res = dstrffo(comment, 0, "XYZ");
    assert(res == DSTR_NPOS);

    dstrcpy(comment, "");
    assert( dstrempty(comment) );
    assert( dstrffo(comment, 0, "X") == DSTR_NPOS );

    dstrfree(s1);
    dstrfree(rhs);
    dstrfree(comment);
}
//-------------------------------------------------

void test_flo()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew(longstr);

    size_t res = dstrflo(s1, DSTR_NPOS, " \t");
    assert(res == 21);

    res = dstrflo(s1, 5, " \t");
    assert(res == 4);

    res = dstrflo(s1, 1000, "");
    assert(res == DSTR_NPOS);

    DSTR rhs = dstrnew("Frdy");
    res = dstrflo_ds(s1, DSTR_NPOS, rhs);
    assert(res == 27);

    dstrcpy(rhs, " F");
    res = dstrflo_ds(s1, DSTR_NPOS, rhs);
    assert(res == 22);

    DSTR comment = dstrnew("# comment");
    res = dstrflo(comment, 1000, "#;");
    printf("RES = %zu\n", res);
    assert(res == 0);

    TRACE_LN();

    res = dstrflo(comment, 1000, "XYZ");
    printf("RES = %zu\n", res);
    assert(res == DSTR_NPOS);

    TRACE_LN();
    dstrcpy(comment, "");
    assert( dstrflo(comment, 1000, "X") == DSTR_NPOS );

    TRACE_LN();
    dstrfree(comment);
    dstrfree(rhs);
    dstrfree(s1);
}
//-------------------------------------------------

void test_ffno()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew(longstr);

    size_t res = dstrffno(s1, 0, " \t");
    assert(res == 0);

    res = dstrffno(s1, 0, "Godm ");
    assert(res == 7);

    res = dstrffno(s1, 5, "Godm ");
    assert(res == 7);

    res = dstrffno(s1, 8, "Godm ");
    assert(res == 8);

    DSTR rhs = dstrnew("Godmr ");
    res = dstrffno_ds(s1, 0, rhs);
    assert(res == 8);

    dstrcpy(s1, "ABCACABCABCABC");
    res = dstrffno(s1, 0, "ABC");
    assert(res == DSTR_NPOS);

    res = dstrffno(s1, 0, "AB");
    assert(res == 2);

    // split a string
    DSTR s2 = dstrnew(longstr);
    size_t first, last = 0;
    while ((first = dstrffno(s2, last, " \t")) != DSTR_NPOS) {
        last = dstrffo(s2, first, " \t");
        DSTR sub = dstrnew_substr(s2, first, (last - first));
        printf("%s\n", dstrdata(sub));
        dstrfree(sub);
    }

    dstrfree(s1);
    dstrfree(rhs);
    dstrfree(s2);
}
//-------------------------------------------------

void test_flno()
{
    TRACE_FN();

    const char* longstr = "ABCDEF_123456";
    DSTR s1 = dstrnew(longstr);

    size_t res = dstrflno(s1, 100, "0123456789");
    assert(res == 6);

    res = dstrflno(s1, 5, "0123456789");
    assert(res == 5);

    res = dstrflno(s1, 5, "ABCDEF");
    assert(res == DSTR_NPOS);

    dstrfree(s1);
}
//-------------------------------------------------

void test_prefix()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew(longstr);

    assert( dstartswith(s1, "G"));
    assert( dstartswith(s1, "Good"));
    assert( dstartswith(s1, longstr));
    assert( !dstartswith(s1, "good"));
    assert( dstartswith(s1, ""));

    assert( dstartswith_i(s1, "GOOD MORNING"));
    assert( dstartswith_i(s1, "GooD"));
    assert( dstartswith_i(s1, "GooD morNiNg Today is Friday"));
    assert( !dstartswith(s1, "H"));
    assert( !dstartswith_i(s1, "H"));
    assert( dstartswith_i(s1, ""));

    dstrfree(s1);
}
//-------------------------------------------------

void test_suffix()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew(longstr);

    assert( dendswith(s1, "y"));
    assert( dendswith(s1, "day"));
    assert( !dendswith(s1, "D") );
    assert( dendswith(s1, longstr));
    assert( dendswith_i(s1, "GooD morNiNg Today is Friday"));

    assert( dendswith_i(s1, "Y"));
    assert( dendswith_i(s1, "DaY"));
    assert( !dendswith(s1, "H"));
    assert( !dendswith_i(s1, "H"));

    dstrfree(s1);
}
//-------------------------------------------------

void test_blank()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew(longstr);

    assert( !disblank(s1) );

    dstrcpy(s1, " \t");
    assert( disblank(s1) );

    dstrcpy(s1, " \t:");
    assert( !disblank(s1) );

    dstrcpy(s1, "         ");
    assert( disblank(s1) );

    dstrfree(s1);
}
//-------------------------------------------------

#define TEST_ATOI(src, result) do {                                     \
        DSTR s = dstrnew(src);                                          \
        long n = datoi(s);                                              \
        printf("Converted = %ld, Expected = %ld\n", n, result);         \
        assert( n == result );                                          \
        dstrfree(s);                                                    \
    } while (0)
//-------------------------------------------------

#define TEST_ITOS(n, result) do {                           \
        DSTR s = dstrnew_empty();                           \
        ditos(s, n);                                        \
        assert( dstreq(s, result) );                     \
        dstrfree(s);                                        \
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

    // test itos vs sprintf
    //
    DSTR d = dstrnew_empty();
    for (int i = 0; i < 10000; ++i) {
        char buf[100];
        long n = rand();
        n *= rand();
        if (n % 2) n = -n;
        sprintf(buf, "%ld", n);
        ditos(d, n);
        assert(dstreq(d, buf));
    }
    dstrfree(d);
}
//-------------------------------------------------

#define TEST_DIGIT(s, result) do {                          \
        DSTR d = dstrnew(s);                                \
        assert( disdigits(d) == result );                   \
        dstrfree(d);                                        \
    } while (0)

#define TEST_XDIGIT(s, result) do {                         \
        DSTR d = dstrnew(s);                                \
        assert( disxdigits(d) == result );                  \
        dstrfree(d);                                        \
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

void test_center()
{
    TRACE_FN();

    DSTR s1 = dstrnew("Hello");
    DSTR s2 = dstrnew_ds(s1);

    dstralign_c(s2, 30, '@');
    printf("%s\n", dstrdata(s2));

    dstrcpy_ds(s2, s1);
    dstralign_c(s2, dstrlen(s1), '@');
    printf("%s\n", dstrdata(s2));
    assert(dstreq_ds(s1, s2));

    dstrcpy_ds(s2, s1);
    dstralign_c(s2, dstrlen(s1) - 1, '@');
    printf("%s\n", dstrdata(s2));
    assert(dstreq_ds(s1, s2));

    dstrcpy_ds(s2, s1);
    dstralign_c(s2, 20, '+');
    printf("%s\n", dstrdata(s2));

    dstrcpy_ds(s2, s1);
    dstralign_c(s2, 20, ' ');
    printf("\"%s\"\n", dstrdata(s2));

    dstrfree(s2);
    dstrfree(s1);
}

void test_join()
{
    TRACE_FN();
    const char* argv[] = {"hello", "world", "good", "morning", NULL};
    size_t argc = 4;

    struct DSTR_TYPE dest;
    dstr_init_data(&dest);
    dstr_join_sz(&dest, "...", argv, argc);
    printf("LEN=%zu, %s\n", dstrlen(&dest), dstrdata(&dest));
    assert( dstreq(&dest, "hello...world...good...morning"));
    dstr_clean_data(&dest);

    DSTR ds = dstrnew_empty();

    dstr_join_sz(ds, "\t", argv, argc);
    printf("LEN=%zu, %s\n", dstrlen(ds), dstrdata(ds));
    assert( dstreq(ds, "hello\tworld\tgood\tmorning"));
    dstrclear(ds);

    dstr_join_sz(ds, NULL, argv, argc);
    printf("1. LEN=%zu, %s\n", dstrlen(ds), dstrdata(ds));
    assert(dstreq(ds, "helloworldgoodmorning"));
    dstrclear(ds);

    dstr_join_sz(ds, "", argv, argc);
    printf("2. LEN=%zu, %s\n", dstrlen(ds), dstrdata(ds));
    assert(dstreq(ds, "helloworldgoodmorning"));

    dstrfree(ds);
}
/*--------------------------------------------------------------------------*/

#define DSTRTRANS(s1, a1, a2, r) do {           \
        DSTR dd = dstrnew(s1);                  \
        dstrtrans(dd, (a1), (a2));              \
        puts(dstrdata(dd));                     \
        assert(dstreq(dd, (r)));                \
        dstrfree(dd); } while (0)

#define DSTRTRSQZ(s1, a1, a2, r) do {           \
        DSTR dd = dstrnew(s1);                  \
        dstrtr_s(dd, (a1), (a2));               \
        puts(dstrdata(dd));                     \
        assert(dstreq(dd, (r)));                \
        dstrfree(dd); } while (0)

#define DSTRSQZ(s1, a1, r) do {                 \
        DSTR dd = dstrnew(s1);                  \
        dstrsqz(dd, (a1));                      \
        puts(dstrdata(dd));                     \
        assert(dstreq(dd, (r)));                \
        dstrfree(dd); } while (0)
/*--------------------------------------------------------------------------*/

void test_translate()
{
    DSTRTRANS("Hello Sam!", "S", "P", "Hello Pam!");
    DSTRTRANS("Hi Sam!", "mSa", "eJo", "Hi Joe!");

    DSTRTRANS("hello", "aeiou", "-", "h-ll-");
    DSTRTRANS("hello", "aeiou", "AA-", "hAll-");
    DSTRTRANS("hello world today is sunday", "aeiou", NULL, "hll wrld tdy s sndy");
    DSTRTRANS("hello world today is sunday", "^aeiou", NULL, "eoooaiua");
    DSTRTRANS("HELLO", "Z-A", "a-z", "svool");
    DSTRTRANS("amzn", "a-mz-n", "A-MZ-N", "AMZN");
    DSTRTRANS("ABCDEFGHIJKLMNOPQRSTUVWXYZ",
              "A-MZ-N", "a-mz-n",
              "abcdefghijklmnopqrstuvwxyz");
    DSTRTRANS("hello", "a-mz-n", "A-MZ-N", "HELLO");
    DSTRTRANS("hello world 123", "a-mz-n", "A-MZ-N", "HELLO WORLD 123");

    DSTRTRANS("hello", "^aeiou", "-", "-e--o");
    DSTRTRANS("helloz worldz todayz is sundayz", "a-z", "A-Z", "HELLOZ WORLDZ TODAYZ IS SUNDAYZ");

    DSTRTRSQZ("aabbccddeeffgg", "", "abcd", "abcdeeffgg");
    DSTRTRSQZ("hello", "l", "r", "hero");
    DSTRTRSQZ("hello", "el", "-", "h-o");
    DSTRTRSQZ("hello", "el", "hx", "hxo");

    // multiple characters runs, are squeezed down to a single char
    // default is for all ("", or NULL) but a squeeze set can be provided
    //
    DSTRSQZ("yellow moon", "", "yelow mon");
    DSTRSQZ("yellow moon", NULL, "yelow mon");
    DSTRSQZ("Too    Many    Spaces", " ", "Too Many Spaces");
    DSTRSQZ("putters shoot balls", "m-z", "puters shot balls");
    DSTRSQZ("putters shoot balls", "l-z", "puters shot bals");
    DSTRSQZ("ppuutters shoot ballss", NULL, "puters shot bals");
    DSTRSQZ("hhhhheeeeeeeelllooooooo", "e-p", "helo");
    DSTRSQZ("hhhhheeeeeeeelllooooooo", "e-ko-q", "helllo");
    DSTRSQZ("hhhhheeeeeeeelllooooooo", "e-ko-q", "helllo");
    DSTRSQZ("hhhhheeeeeeeelllooooooo", "e-ko-q", "helllo");
    DSTRSQZ("aabbccddAABBCCDD11223344", "a-cA-C1-3", "abcddABCDD12344");
    DSTRSQZ("++,,----...../////023aabb", "a--0", "++,,-./023abb");
    DSTRSQZ("a---b===c", "-=", "a-b=c");
    DSTRSQZ("AAA111zzz", "A-Z0-9", "A1zzz");

    // ^ is "negate" (or -c, complement in tr command)
    //
    DSTRSQZ("AAAppp111zzz", "^A-Z0-9", "AAAp111z");

    // Unlike ruby, ranges go downwards too
    //
    DSTRSQZ("AAA111zzz", "Z-A9-0", "A1zzz");
}
/*--------------------------------------------------------------------------*/

#define TEST_SUCC(before, after) do {                                   \
    DSTR next = dstrnew(before);                                        \
    dstrinc(next);                                                      \
    printf("\"%s\".succ() ==> \"%s\"\n", (before), dstrdata(next));     \
    assert(dstreq(next, (after))); dstrfree(next); } while(0)

void test_succ()
{
    TRACE_FN();

    TEST_SUCC("USA", "USB");
    TEST_SUCC("", "");
    TEST_SUCC("00", "01");
    TEST_SUCC("09", "10");
    TEST_SUCC("99", "100");
    TEST_SUCC("aa", "ab");
    TEST_SUCC("az", "ba");
    TEST_SUCC("zz", "aaa");
    TEST_SUCC("AA", "AB");
    TEST_SUCC("AZ", "BA");
    TEST_SUCC("ZZ", "AAA");
    TEST_SUCC("zz99zz99", "aaa00aa00");
    TEST_SUCC("99zz99zz", "100aa00aa");
    TEST_SUCC("abcd", "abce");
    TEST_SUCC("THX1138", "THX1139");
    TEST_SUCC("<<koala>>", "<<koalb>>");
    TEST_SUCC("1999zzz", "2000aaa");
    TEST_SUCC("ZZZ9999", "AAAA0000");
    TEST_SUCC("***", "**+");
    TEST_SUCC("9", "10");
    TEST_SUCC("z", "aa");
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
    TEST_SUCC("hello!@$99", "hello!@$100");
    TEST_SUCC("hell8!@$zz", "hell8!@$aaa");
    TEST_SUCC("a/@z", "b/@a");
    TEST_SUCC("/@z", "/@aa");
    TEST_SUCC("XY.99", "XY.100");
    TEST_SUCC("XY1.99", "XY2.00");
}
//-------------------------------------------------

#define TEST_ZFILL(before, len, after) do {             \
        DSTR f = dstrnew(before);                       \
        dzfill(f, len);                                 \
        printf("\"%s\"\n", dstrdata(f));                \
        assert(dstreq(f, (after)));                     \
        dstrfree(f);                                    \
    } while (0)

void test_zfill()
{
    TRACE_FN();

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

void test_times()
{
    TRACE_FN();

    DSTR s = dstrnew("A");
    DSTR tmp1 = dstrnew_ds(s);
    dstrmult(tmp1, 5);
    assert(dstreq(tmp1, "AAAAA"));

    dstrcpy_ds(tmp1, s);
    DSTR tmp2 = dstrnew_ds(s);

    dstrmult(tmp1, 2);
    dstrmult(tmp1, 2);
    dstrmult(tmp2, 4);
    assert(dstreq_ds(tmp1, tmp2));

    dstrmult(tmp1, 0);
    assert(dstreq(tmp1, ""));

    dstrcpy_ds(tmp1, s);
    dstrmult(tmp1, 1);
    assert(dstreq_ds(tmp1, s));

    dstrcpy_cc(tmp1, '=', 50);
    dstrcpy_cc(tmp2, '=', 1);
    dstrmult(tmp2, 50);
    assert(dstreq_ds(tmp1, tmp2));

    printf("%s\nH E L L O   W O R L D ! !\n%s\n", dstrdata(tmp1), dstrdata(tmp2));

    dstrfree(s);
    dstrfree(tmp1);
    dstrfree(tmp2);
}
//-------------------------------------------------

void test_strip()
{
    TRACE_FN();

    DSTR s = dstrnew("####Hello####");
    dlstrip_c(s, '#');
    drstrip_c(s, '#');
    assert(dstreq(s, "Hello"));

    dstrcpy(s, "#####################");
    drstrip_c(s, '#');
    assert(dstrempty(s));

    dstrcpy(s, "#####################");
    dlstrip_c(s, '#');
    assert(dstrempty(s));

    dstrcpy(s, " ###Hello###");
    dlstrip_c(s, '#');
    drstrip_c(s, '#');
    assert(dstreq(s, " ###Hello"));

    dstrcpy(s, " \n \t hello\n");
    dlstrip_c(s, '\n');
    drstrip_c(s, '\n');
    assert(dstreq(s, " \n \t hello"));

    dstrcpy(s, "\n\n \t hello\n");
    dlstrip_c(s, '\n');
    drstrip_c(s, '\n');
    assert(dstreq(s, " \t hello"));

    dstrcpy(s, "www.example.com");
    dlstrip(s, "cmow.");
    drstrip(s, "cmow.");
    assert(dstreq(s, "example"));

    dstrcpy(s, " www.example.com");
    dlstrip(s, "cmow.");
    drstrip(s, "cmow.");
    assert(dstreq(s, " www.example"));

    dstrcpy(s, "www.example.com ");
    dlstrip(s, "cmow.");
    drstrip(s, "cmow.");
    assert(dstreq(s, "example.com "));

    dstrcpy(s, "Arthur: three!");
    dlstrip(s, "Arthur: ");
    assert(dstreq(s, "ee!"));

    dstrcpy(s, "Hello World \t\r\n");
    drstrip(s, " \t\n\r");
    assert(dstreq(s, "Hello World"));

    dstrfree(s);
}
//-------------------------------------------------

void test_is_identifier()
{
    TRACE_FN();

    DSTR tmp = dstrnew("MyFolder");
    assert(dstr_isidentifier(tmp));

    dstrcpy(tmp, "Demo001");
    assert(dstr_isidentifier(tmp));

    dstrcpy(tmp, "_Demo001");
    assert(dstr_isidentifier(tmp));

    dstrcpy(tmp, "4Demo001");
    assert(!dstr_isidentifier(tmp));

    dstrcpy(tmp, "2bring");
    assert(!dstr_isidentifier(tmp));

    dstrcpy(tmp, "my demo");
    assert(!dstr_isidentifier(tmp));

    dstrfree(tmp);
}
//-------------------------------------------------

#define ASSERT_PARTITION(str, what, p1, p2, p3) do {            \
    DSTR s = dstrnew(str);                                      \
    struct DSTR_PartInfo pinfo;                                 \
    dstr_partition(s, (what), &pinfo);                          \
    DSTR l = dstrnew_substr(s, pinfo.l_pos, pinfo.l_len);       \
    DSTR m = dstrnew_substr(s, pinfo.m_pos, pinfo.m_len);       \
    DSTR r = dstrnew_substr(s, pinfo.r_pos, pinfo.r_len);       \
    assert(dstreq(l, p1));                                      \
    assert(dstreq(m, p2));                                      \
    assert(dstreq(r, p3));                                      \
    dstrfree(s); dstrfree(l); dstrfree(m); dstrfree(r);         \
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

#define ASSERT_RPARTITION(str, what, p1, p2, p3) do {           \
    DSTR s = dstrnew(str);                                      \
    struct DSTR_PartInfo pinfo;                                 \
    dstr_rpartition(s, (what), &pinfo);                         \
    DSTR l = dstrnew_substr(s, pinfo.l_pos, pinfo.l_len);       \
    DSTR m = dstrnew_substr(s, pinfo.m_pos, pinfo.m_len);       \
    DSTR r = dstrnew_substr(s, pinfo.r_pos, pinfo.r_len);       \
    assert(dstreq(l, p1));                                      \
    assert(dstreq(m, p2));                                      \
    assert(dstreq(r, p3));                                      \
    dstrfree(s); dstrfree(l); dstrfree(m); dstrfree(r);         \
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

void test_align()
{
    TRACE_FN();

    DSTR s1 = dstrnew("Hello");
    DSTR s2 = dstrnew_empty();

    dstrcpy_ds(s2, s1);
    dstralign_l(s2, 30, '@');
    puts(dstrdata(s2));

    dstrcpy_ds(s2, s1);
    dstralign_r(s2, 30, '@');
    puts(dstrdata(s2));

    dstrcpy_ds(s2, s1);
    dstralign_l(s2, dstrlen(s1), '@');
    puts(dstrdata(s2));
    assert(dstreq_ds(s1, s2));

    dstrcpy_ds(s2, s1);
    dstralign_r(s2, dstrlen(s1), '@');
    puts(dstrdata(s2));
    assert(dstreq_ds(s1, s2));

    dstrcpy_ds(s2, s1);
    dstralign_l(s2, dstrlen(s1) - 1, '@');
    puts(dstrdata(s2));
    assert(dstreq_ds(s1, s2));

    dstrcpy_ds(s2, s1);
    dstralign_r(s2, dstrlen(s1) - 1, '@');
    puts(dstrdata(s2));
    assert(dstreq_ds(s1, s2));

    dstrfree(s1);
    dstrfree(s2);
}
//-------------------------------------------------

void test_replace_all()
{
    TRACE_FN();

    const char* origstr = "I love apple apple apple apple apple";
    DSTR orig = dstrnew(origstr);
    printf("ORIG:\n%s\n", dstrdata(orig));

    dstr_replace_all(orig, "apple", "@bannana@", DSTR_REPLACE_ALL);
    printf("%s\n", dstrdata(orig));

    dstrcpy(orig, origstr);
    dstr_replace_all(orig, "apple", "fig", DSTR_REPLACE_ALL);
    printf("%s\n", dstrdata(orig));

    dstrcpy(orig, origstr);
    dstr_replace_all(orig, "apple", "apple123", DSTR_REPLACE_ALL);
    printf("%s\n", dstrdata(orig));

    dstrcpy(orig, origstr);
    dstr_replace_all(orig, "apple", "@bannana@", 2);
    printf("%s\n", dstrdata(orig));

    dstrcpy(orig, origstr);
    DSTR oldstr = dstrnew("apple");
    DSTR newstr = dstrnew("@DSTR@");
    dstr_replace_all_ds(orig, oldstr, newstr, DSTR_REPLACE_ALL);
    printf("%s\n", dstrdata(orig));

    dstrcpy(orig, origstr);
    dstr_replace_all_ds(orig, oldstr, newstr, 0);
    printf("%s\n", dstrdata(orig));

    dstrfree(orig);
    dstrfree(oldstr);
    dstrfree(newstr);
}
//-------------------------------------------------

#define TEST_COUNT(str, search, expected) do {             \
        DSTR str_ = dstrnew(str);                          \
        assert(dstrcount(str_, (search)) == (expected));   \
        DSTR search_ = dstrnew(search);                    \
        assert(dstrcount_ds(str_, search_) == (expected));  \
        dstrfree(str_); dstrfree(search_);                 \
    } while (0)

#define TEST_ICOUNT(str, search, expected) do {              \
        DSTR str_ = dstrnew(str);                            \
        assert(dstrcount_i(str_, (search)) == (expected));   \
        DSTR search_ = dstrnew(search);                      \
        assert(dstrcount_di(str_, search_) == (expected));   \
        dstrfree(str_); dstrfree(search_);                   \
    } while (0)

void test_count()
{
    TRACE_FN();

    TEST_COUNT("", "", 1);
    TEST_COUNT("A", "", 2);
    TEST_COUNT("ABC", "", 4);
    TEST_COUNT("pppp", "pp", 2);
    TEST_COUNT("pppp", "ppp", 1);

    TEST_ICOUNT("", "", 1);
    TEST_ICOUNT("A", "", 2);
    TEST_ICOUNT("ABC", "", 4);
    TEST_ICOUNT("pppp", "pp", 2);
    TEST_ICOUNT("pppp", "ppp", 1);
    TEST_ICOUNT("pppp", "PpP", 1);

    TEST_COUNT("Hello World", "hell", 0);
    TEST_ICOUNT("Hello World", "hell", 1);

    TEST_COUNT("Hello World", "RLD", 0);
    TEST_ICOUNT("Hello World", "RLD", 1);
}
//-------------------------------------------------

#define TEST_EXPAND_TABS(str, width, expected)  do {    \
        DSTR str_ = dstrnew(str);                       \
        dexpandtabs(str_, (width));                     \
        assert(dstreq(str_, (expected)));               \
        dstrfree(str_);                                 \
} while (0);

void test_expandtabs()
{
    TRACE_FN();

    const char* s = "\t";
    TEST_EXPAND_TABS(s, 8, "        ");
    TEST_EXPAND_TABS(s, 1, " ");
    TEST_EXPAND_TABS(s, 2, "  ");
    DSTR tmp = dstrnew_cc(' ', 60);
    TEST_EXPAND_TABS(s, 60, dstrdata(tmp));
    dstrfree(tmp);

    s = "Hello\tWorld\tToday\tIs\tSaturday";
    TEST_EXPAND_TABS(s, 8, "Hello   World   Today   Is      Saturday");
    TEST_EXPAND_TABS(s, 7, "Hello  World  Today  Is     Saturday");
    TEST_EXPAND_TABS(s, 6, "Hello World Today Is    Saturday");
    TEST_EXPAND_TABS(s, 5, "Hello     World     Today     Is   Saturday");
    TEST_EXPAND_TABS(s, 4, "Hello   World   Today   Is  Saturday");
    TEST_EXPAND_TABS(s, 3, "Hello World Today Is Saturday");
    TEST_EXPAND_TABS(s, 2, "Hello World Today Is  Saturday");
    TEST_EXPAND_TABS(s, 1, "Hello World Today Is Saturday");
    TEST_EXPAND_TABS(s, 0, "HelloWorldTodayIsSaturday");
}
//-------------------------------------------------

#define TEST_TITLE(before, after)  do {         \
    DSTR before_ = dstrnew(before);             \
    dstrtitle(before_);                         \
    assert(dstreq(before_, after));             \
    assert(distitle(before_));                  \
    dstrfree(before_);                          \
    } while (0)

void test_title()
{
    TRACE_FN();

    TEST_TITLE("hello world today 33 is SAT", "Hello World Today 33 Is SAT");
    TEST_TITLE("Welcome to my 2nd world", "Welcome To My 2Nd World");
    TEST_TITLE("hello b2b2b2 and 3g3g3g", "Hello B2B2B2 And 3G3G3G");
    TEST_TITLE("hello b2bb2b2 a.n.d 3g3g3g", "Hello B2Bb2B2 A.N.D 3G3G3G");
    TEST_TITLE("hello b2b2b2 a_n_d 3g3g3g", "Hello B2B2B2 A_N_D 3G3G3G");
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
    test_fromfile();
    test_append();
    test_format();
    test_remove();
    test_insert();
    test_replace();
    test_replace1();
    test_trim();
    test_substr();
    test_left_mid_right();
    test_truncate();
    test_shrink();
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
    test_fgets();
    test_atoi_itos();
    test_isdigit();
    test_getline();
    test_center();
    test_join();
    test_translate();
    test_succ();
    test_zfill();
    test_times();
    test_strip();
    test_is_identifier();
    test_partition();
    test_rpartition();
    test_align();
    test_replace_all();
    test_count();
    test_expandtabs();
    test_title();
}
