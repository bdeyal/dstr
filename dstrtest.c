#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include "dstr.h"

/*
 *   garbage collector / memory checker
 */
#ifndef _WIN32
#ifdef GC_DEBUG
#include <gc/gc.h>
#define malloc   GC_MALLOC
#define free     GC_FREE
#define realloc  GC_REALLOC
#endif
#endif

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

    DSTR s1 = dstrnew_sz("hello");
    TRACE_DS(s1);
    assert(dstrlen(s1) == strlen("hello"));
    assert(strcmp(dstrdata(s1), "hello") == 0);

    DSTR s2 = dstrnew();
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
    DSTR s1 = dstrnew_sz("hello");
    DSTR s2 = dstrnew_sz("hello");
    DSTR s3 = dstrnew_sz("HELLO");
    DSTR s4 = dstrnew_sz("Hello World");
    DSTR s5 = dstrnew();

    assert(dstreq_ds(s1, s2));
    assert(dstreq_sz(s1, "hello"));
    assert(!dstreq_ds(s1, s3));
    assert(!dstreq_sz(s1, "ABC"));
    assert(!dstreq_ds(s1, s4));

    assert(dstrcmp_ds(s1, s2) == 0);
    assert(dstrcmp_sz(s1, "hello") == 0);
    assert(dstrcmp_ds(s1, s3) != 0);
    assert(dstrcmp_sz(s1, "ABC") != 0);

    assert(dstrcmp_sz(s5, NULL) == 0);
    assert(dstrcmp_sz(s1, NULL) > 0);

    assert(dstreq_sz(s5, NULL));
    assert(!dstreq_sz(s1, NULL));

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

    DSTR s1 = dstrnew();
    DSTR s2 = dstrnew_bl(NULL, 0);
    DSTR s3 = dstrnew_reserve(0);
    DSTR s4 = dstrnew_reserve(100);
    DSTR s5 = dstrnew_sz(NULL);
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
    assert( dstreq_sz(s1, "AAAAA") );

    DSTR s2 = dstrnew_cc('B', 1);
    assert( dstrlen(s2) == 1 );
    assert( dstreq_sz(s2, "B") );
    assert( dstrgetc(s2, 0) == 'B' );

    DSTR s3 = dstrnew_cc('\0', 5);
    assert( dstrlen(s3) == 0 );
    assert( dstreq_sz(s3, "") );

    dstrfree(s1);
    dstrfree(s2);
    dstrfree(s3);
}
//-------------------------------------------------

void test_copy_ctor()
{
    TRACE_FN();
    DSTR s1 = dstrnew_sz("hello");
    DSTR s2 = dstrnew_ds(s1);

    assert(dstreq_ds(s1, s2));
    assert(dstreq_sz(s2, "hello"));

    DSTR s3 = dstrnew();
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
    assert(dstreq_sz(s1, "ABCDE"));

    DSTR s2 = dstrnew_bl("ABCDEFGH", 0);
    assert( dstrlen(s2) == 0);
    assert( dstreq_sz(s2,""));

    // Test assign_buff too
    //
    DSTR s3 = dstrnew();
    dstrcpy_bl(s3, "ABCDEFGH", 5);
    assert(dstrlen(s3) == 5);
    assert(dstreq_sz(s3, "ABCDE"));

    DSTR s4 = dstrnew();
    dstrcpy_bl(s4, "ABCDEFGH", 0);
    assert(dstrlen(s4) == 0);
    assert(dstreq_sz(s4, ""));

    // failed
    DSTR s5 = dstrnew_bl("", 10);
    assert( dstreq_sz(s5, "") );
    assert(dstrlen(s5) == 0);

    dstrfree(s1);
    dstrfree(s2);
    dstrfree(s3);
    dstrfree(s4);
    dstrfree(s5);
}
//-------------------------------------------------

void test_assign()
{
    TRACE_FN();

    DSTR s1 = dstrnew();
    dstrcpy_sz(s1, "hello");
    assert( dstreq_sz(s1, "hello"));
    assert( dstrlen(s1) == strlen("hello"));

    DSTR s2 = dstrnew();
    dstrcpy_ds(s2, s1);
    assert( dstreq_ds(s1, s2));
    assert( dstreq_sz(s2, "hello"));
    assert( dstrlen(s2) == strlen("hello"));

    dstrcpy_cc(s2, 'C', 8);
    assert( dstrlen(s2) == 8);
    assert( dstreq_sz(s2, "CCCCCCCC"));
    assert( strcmp(dstrdata(s2), "CCCCCCCC") == 0);

    dstrcpy_sz(s2, NULL);
    assert(dstrlen(s2) == 0);

    DSTR s3 = dstrnew_sz("hello");
    DSTR s4 = dstrnew();
    dstrcpy_ds(s3, s4);
    assert( dstreq_ds(s3, s4) );
    assert( dstrlen(s3) == 0 );

    DSTR s5 = dstrnew_sz("hello");
    dstrcpy_sz(s5, "world");
    assert( dstreq_sz(s5, "world") );
    assert( dstrlen(s5) == strlen("world") );

    DSTR s6 = dstrnew_reserve(100);
    dstrcpy_substr(s6, s5, 0, DSTR_NPOS);
    assert( dstreq_ds(s5, s6));

    dstrcpy_substr(s6, s5, 0, 2);
    assert( dstreq_sz(s6, "wo"));

    dstrcpy_substr(s6, s5, 1, 3);
    assert( dstreq_sz(s6, "orl"));

    dstrcpy_substr(s6, s5, 3, 2);
    assert( dstreq_sz(s6, "ld"));

    dstrcpy_substr(s6, s5, 3, 100);
    assert( dstreq_sz(s6, "ld"));

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

    s1 = dstrnew_sz("hello");
    s2 = dstrnew_sz("world");
    s3 = dstrnew_ds(s1);
    dstrcat_ds(s3, s2);

    assert(dstreq_sz(s3, "helloworld"));
    assert(dstrlen(s3) == dstrlen(s2) + dstrlen(s1));

    DSTR s4 = dstrnew();
    dstrcpy_ds(s4, s1);
    dstrcat_sz(s4, "world");
    assert( dstreq_sz(s4, "helloworld") );

    dstrcat_sz(s4, NULL);
    assert( dstreq_sz(s4, "helloworld") );

    dstrcat_sz(s4, "");
    assert( dstreq_sz(s4, "helloworld") );

    DSTR s5 = dstrnew();
    dstrcat_cc(s5, 'h', 1);
    assert( dstreq_sz(s5, "h") );

    dstrcat_c(s5, 'e');
    dstrcat_cc(s5, 'l', 2);
    dstrcat_cc(s5, 'o', 1);
    assert( dstreq_sz(s5, "hello"));

    dstrcat_c(s5, '\0');
    assert( dstreq_sz(s5, "hello"));

    dstrcat_bl(s5, "_ABC\0EFG", 10);
    assert( dstreq_sz(s5, "hello_ABC"));

    dstrcat_bl(s5, NULL, 10);
    assert( dstreq_sz(s5, "hello_ABC"));

    dstrcat_bl(s5, "Hi", 0);
    assert( dstreq_sz(s5, "hello_ABC"));

    dstrcat_bl(s5, "_DEF", 3);
    assert( dstreq_sz(s5, "hello_ABC_DE"));

    dstrcat_bl(s5, "FGH", 1);
    assert( dstreq_sz(s5, "hello_ABC_DEF"));

    dstrcat_ds(s5, s5);
    assert( dstreq_sz(s5, "hello_ABC_DEFhello_ABC_DEF"));

    int step = 1000;
    int limit = 10 * step;
    for (int i = 0; i < limit; ++i) {
        if ((i % step) == 0) {
            printf("step: %d LEN=%zu, CAP=%zu\n", i, dstrlen(s5), dstrcap(s5));
        }
        dstrcat_c(s5, 'X');
    }

    dstrtrunc(s5);

    for (int i = 0; i < limit; ++i) {
        if ((i % step) == 0) {
            printf("step: %d LEN=%zu, CAP=%zu\n", i, dstrlen(s5), dstrcap(s5));
        }
        dstrcat_sz(s5, "hello");
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
    assert( dstreq_sz(res1, res2) );
    TRACE_LN();

    // simple double
    //
    dstrtrunc(res1);
    dsprintf(res1, "%g", 100.0);
    sprintf(res2, "%g", 100.0);
    assert( dstreq_sz(res1, res2) );
    TRACE_LN();

    // hex
    //
    dsprintf(res1, "%08X", 100);
    sprintf(res2, "%08X", 100);
    assert( dstreq_sz(res1, res2) );
    TRACE_LN();

    // long string
    //
    char buffer[2048];
    memset(buffer, 'a', sizeof buffer);
    buffer[sizeof(buffer) - 1] = '\0';
    TRACE_LN();

    dsprintf(res1, "%s", buffer);
    sprintf(res2, "%s", buffer);
    assert( dstreq_sz(res1, res2) );
    TRACE_LN();

    // Complicated format
    //
    dsprintf(res1, "%0X %g %10s %10d %-20s", 100, 4.55, "Hello World", 10, "A_String");
    sprintf(res2, "%0X %g %10s %10d %-20s", 100, 4.55, "Hello World", 10, "A_String");
    assert( dstreq_sz(res1, res2) );
    TRACE_LN();

    dsprintf(res1, "%s", "Hello ");
    dstrcat_sprintf(res1, "%s", "World");
    sprintf(res2, "%s", "Hello World");
    assert(dstreq_sz(res1, res2 ));
    TRACE_LN();

    dstrfree(res1);
    TRACE_LN();
}
//-------------------------------------------------

#define TEST_TRIM_LEFT(before, after) do {                  \
        DSTR s = dstrnew_sz(before);                        \
        dstrtrim_l(s);                                      \
        assert( dstreq_sz(s, after) );                      \
        dstrfree(s);                                        \
    } while (0)
//-------------------------------------------------

#define TEST_TRIM_RIGHT(before, after) do {                 \
        DSTR s = dstrnew_sz(before);                        \
        dstrtrim_r(s);                                      \
        assert( dstreq_sz(s, after) );                      \
        dstrfree(s);                                        \
    } while (0)
//-------------------------------------------------

#define TEST_TRIM_BOTH(before, after) do {                  \
        DSTR s = dstrnew_sz(before);                        \
        dstrtrim(s);                                        \
        assert( dstreq_sz(s, after) );                      \
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
    DSTR s1 = dstrnew();
    dstrcpy_sz(s1, hello);
    assert( dstreq_sz(s1, hello));

    // erase at loc 1000 -> NOOP
    dstrerase(s1, 1000, 1);
    assert( dstreq_sz(s1, hello));

    dstrerase(s1, 0, 1);
    assert( dstreq_sz(s1, "hellXXoXXX"));

    dstrerase(s1, 4, 2);
    assert( dstreq_sz(s1, "helloXXX"));

    dstrerase(s1, 5, 3);
    assert( dstreq_sz(s1, "hello"));

    dstrerase(s1, 4, 100);
    assert( dstreq_sz(s1, "hell") );

    dstrerase(s1, 2, 0);
    assert( dstreq_sz(s1, "hell") );

    dstrerase(s1, 0, DSTR_NPOS);
    assert( dstreq_sz(s1, ""));

    dstrfree(s1);
}
//-------------------------------------------------

void test_insert()
{
    TRACE_FN();

    const char* hello = "hello";
    DSTR s1 = dstrnew();
    dstrcpy_sz(s1, hello);
    assert( dstreq_sz(s1, hello));

    dinsert_cc(s1, 1000, 'X', 1);
    assert( dstreq_sz(s1, "helloX"));

    dinsert_sz(s1, 4, "XX");
    assert( dstreq_sz(s1, "hellXXoX"));

    dinsert_sz(s1, 4, "");
    assert( dstreq_sz(s1, "hellXXoX"));

    dinsert_sz(s1, 4, NULL);
    assert( dstreq_sz(s1, "hellXXoX"));

    dinsert_bl(s1, 0, "XXX", DSTR_NPOS);
    assert( dstreq_sz(s1, "XXXhellXXoX"));

    dinsert_bl(s1, 0, NULL, DSTR_NPOS);
    assert( dstreq_sz(s1, "XXXhellXXoX"));

    dinsert_bl(s1, 0, "XXX", 0);
    assert( dstreq_sz(s1, "XXXhellXXoX"));

    dinsert_cc(s1, 0, 'A', 3);
    assert( dstreq_sz(s1, "AAAXXXhellXXoX"));

    dinsert_cc(s1, 1000, 'B', 2);
    assert( dstreq_sz(s1, "AAAXXXhellXXoXBB"));

    dstrcpy_sz(s1, "ABCDE");
    dinsert_cc(s1, 3, 'Z', 5);
    assert( dstreq_sz(s1, "ABCZZZZZDE"));

    DSTR s2 = dstrnew_cc('X', 2);
    dinsert_ds(s1, 2, s2);
    assert( dstreq_sz(s1, "ABXXCZZZZZDE"));

    dstrcpy_sz(s2, "0123456789");
    dinsert_sz(s2, dstrlen(s2), "ABCDE");
    assert( dstreq_sz(s2, "0123456789ABCDE"));

    dstrfree(s1);
    dstrfree(s2);
}
//-------------------------------------------------

void test_replace1()
{
    TRACE_FN();

    DSTR s1 = dstrnew_sz("hell@@@@@oXXX");
    DSTR sn = dstrnew_sz("hello");
    dreplace_ds(s1, 10, 5, sn);
    assert( dstreq_sz(s1, "hell@@@@@ohello"));

    dstrfree(s1);
    dstrfree(sn);
}
//-------------------------------------------------

void test_replace()
{
    TRACE_FN();

    const char* hello = "XhellXXoXXX";
    DSTR s1 = dstrnew();
    dstrcpy_sz(s1, hello);
    assert( dstreq_sz(s1, hello));

    dreplace_sz(s1, 0, 1, "");
    assert( dstreq_sz(s1, "hellXXoXXX"));

    dreplace_sz(s1, 4, 2, "@@@@@");
    assert( dstreq_sz(s1, "hell@@@@@oXXX"));

    DSTR sn = dstrnew_sz("hello");
    dreplace_ds(s1, 10, 5, sn);
    assert( dstreq_sz(s1, "hell@@@@@ohello"));

    dreplace_sz(s1, 4, 5, NULL);
    assert( dstreq_sz(s1, "hellohello"));


    dreplace_cc(s1, 5, 5, '\0', 1);
    assert( dstreq_sz(s1, "hello") );

    dreplace_sz(s1, 0, DSTR_NPOS, "world");
    assert( dstreq_sz(s1, "world"));

    DSTR s2 = dstrnew_ds(s1);
    dreplace_bl(s1, 0, DSTR_NPOS, NULL, 100);
    assert( dstreq_sz(s1, ""));

    dreplace_cc(s2, 2, 8,'\0', 100);
    assert( dstreq_sz(s2, "wo"));

    dreplace_cc(s2, 0, 2,'\0', 100);
    assert( dstreq_sz(s2, ""));

    dstrfree(sn);
    dstrfree(s1);
    dstrfree(s2);
}
//-------------------------------------------------

void test_substr()
{
    TRACE_FN();

    char buff[2048];
    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew_sz(longstr);

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
    assert( dstreq_sz(s2, "morning") );
    dstrfree(s2);

    s2 = dstrnew_substr(s1, 0, 4);
    assert( dstreq_sz(s2, "Good") );
    dstrfree(s2);

    s2 = dstrnew_substr(s1, 15, 10);
    assert( dstreq_sz(s2, "day is Fri") );
    dstrfree(s2);

    dstrfree(s1);
}
//-------------------------------------------------

void test_truncate()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew_sz(longstr);

    dstrresize(s1, 100);
    assert( dstreq_sz(s1, longstr) );

    dstrresize(s1, 10);
    assert( strncmp(dstrdata(s1), longstr, 10) == 0);

    dstrtrunc(s1);
    assert( dstrlen(s1) == 0);
    assert( dstreq_sz(s1,""));

    dstrfree(s1);
}
//-------------------------------------------------

void test_find()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew_sz(longstr);

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

void test_put_get()
{
    TRACE_FN();

    const char * str_lower = "abcdefghijklmnopqrstuvwxyz";
    const char* str_upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    DSTR s1 = dstrnew_sz(str_lower);

    for (size_t n = 0; n < dstrlen(s1); ++n) {
        assert(dindex_ok(s1,n));
        char c = dstrgetc(s1, n);
        c = toupper(c);
        assert(c == str_upper[n]);
        dstrputc(s1, n, c);
    }

    assert(!dindex_ok(s1, dstrlen(s1)));
    assert(!dindex_ok(s1, DSTR_NPOS));
    assert(dstreq_sz(s1, str_upper));

    dstrfree(s1);
}
//-------------------------------------------------

void test_put_get_safe()
{
    TRACE_FN();

    const char * str_lower = "abcdefghijklmnopqrstuvwxyz";
    const char* str_upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    DSTR s1 = dstrnew_sz(str_lower);

    // first we must support the non-safe semantics
    //
    for (size_t n = 0; n < dstrlen(s1); ++n) {
        assert(dindex_ok(s1,n));
        char c = dstrgetc_s(s1, n);
        dstrputc_s(s1, n, toupper(c));
    }
    assert(!dindex_ok(s1, dstrlen(s1)));
    assert(!dindex_ok(s1, DSTR_NPOS));
    assert(dstreq_sz(s1, str_upper));

    // back to lowercase
    dstrlower(s1);
    assert(dstreq_sz(s1, str_lower));

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
    assert(dstreq_sz(s2, str_upper));

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

    while (dgetline(s1, fp) != EOF) {
        ++line_count;
        printf("%s\n", dstrdata(s1));
    }
    fclose(fp);
    dstrfree(s1);

    printf("Line Count of %s is %d\n", __FILE__, line_count);

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

    while (dgets(s1, fp) != EOF)
        printf("\t%s\n", dstrdata(s1));

    dstrfree(s1);
    fclose(fp);
}
//-------------------------------------------------

#define TEST_ASCII_UPPER(before, after) do {                \
        DSTR s = dstrnew_sz(before);                        \
        dstrupper(s);                                       \
        assert( dstreq_sz(s, after) );                      \
        dstrfree(s);                                        \
    } while (0)
//-------------------------------------------------

#define TEST_ASCII_LOWER(before, after) do {                \
        DSTR s = dstrnew_sz(before);                        \
        dstrlower(s);                                       \
        assert( dstreq_sz(s, after) );                      \
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
        DSTR s = dstrnew_sz(before);                        \
        dstrrev(s);                                         \
        assert( dstreq_sz(s, after) );                      \
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
}
//-------------------------------------------------

void test_swap()
{
    TRACE_FN();

    DSTR s1 = dstrnew_sz(NULL);
    DSTR s2 = dstrnew_sz("hello");

    dstrswap(s1, s2);
    assert( dstrempty(s2) );
    assert( dstreq_sz(s1, "hello") );

    dstrfree(s1);
    dstrfree(s2);
}
//-------------------------------------------------

void test_ffo()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew_sz(longstr);

    size_t res = dstrffo_sz(s1, 0, " \t");
    assert(res == 4);

    res = dstrffo_sz(s1, 5, " \t");
    assert(res == 12);

    res = dstrffo_sz(s1, 1000, " \t");
    assert(res == DSTR_NPOS);

    DSTR rhs = dstrnew_sz("Frdy");
    res = dstrffo_ds(s1, 0, rhs);
    assert(res == 3);

    DSTR comment = dstrnew_sz("# comment");
    res = dstrffo_sz(comment, 0, "#;");
    assert(res == 0);

    res = dstrffo_sz(comment, 0, "XYZ");
    assert(res == DSTR_NPOS);

    dstrcpy_sz(comment, "");
    assert( dstrempty(comment) );
    assert( dstrffo_sz(comment, 0, "X") == DSTR_NPOS );

    dstrfree(s1);
    dstrfree(rhs);
    dstrfree(comment);
}
//-------------------------------------------------

void test_ffno()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew_sz(longstr);

    size_t res = dstrffno_sz(s1, 0, " \t");
    assert(res == 0);

    res = dstrffno_sz(s1, 0, "Godm ");
    assert(res == 7);

    res = dstrffno_sz(s1, 5, "Godm ");
    assert(res == 7);

    res = dstrffno_sz(s1, 8, "Godm ");
    assert(res == 8);

    DSTR rhs = dstrnew_sz("Godmr ");
    res = dstrffno_ds(s1, 0, rhs);
    assert(res == 8);

    dstrcpy_sz(s1, "ABCACABCABCABC");
    res = dstrffno_sz(s1, 0, "ABC");
    assert(res == DSTR_NPOS);

    res = dstrffno_sz(s1, 0, "AB");
    assert(res == 2);

    // split a string
    DSTR s2 = dstrnew_sz(longstr);
    size_t first, last = 0;
    while ((first = dstrffno_sz(s2, last, " \t")) != DSTR_NPOS) {
        last = dstrffo_sz(s2, first, " \t");
        DSTR sub = dstrnew_substr(s2, first, (last - first));
        printf("%s\n", dstrdata(sub));
        dstrfree(sub);
    }

    dstrfree(s1);
    dstrfree(rhs);
    dstrfree(s2);
}
//-------------------------------------------------

void test_prefix()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew_sz(longstr);

    assert( dstartswith(s1, "G"));
    assert( dstartswith(s1, "Good"));
    assert( dstartswith(s1, longstr));
    assert( !dstartswith(s1, "good"));

    assert( dstartswith_i(s1, "GOOD MORNING"));
    assert( dstartswith_i(s1, "GooD"));
    assert( dstartswith_i(s1, "GooD morNiNg Today is Friday"));
    assert( !dstartswith(s1, "H"));
    assert( !dstartswith_i(s1, "H"));

    dstrfree(s1);
}
//-------------------------------------------------

void test_suffix()
{
    TRACE_FN();

    const char* longstr = "Good morning today is Friday";
    DSTR s1 = dstrnew_sz(longstr);

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
    DSTR s1 = dstrnew_sz(longstr);

    assert( !dstrblank(s1) );

    dstrcpy_sz(s1, " \t");
    assert( dstrblank(s1) );

    dstrcpy_sz(s1, " \t:");
    assert( !dstrblank(s1) );

    dstrcpy_sz(s1, "         ");
    assert( dstrblank(s1) );

    dstrfree(s1);
}
//-------------------------------------------------

#define TEST_ATOI(src, result) do {                                     \
        DSTR s = dstrnew_sz(src);                                       \
        long n = datoi(s);                                              \
        printf("Converted = %ld, Expected = %ld\n", n, result);         \
        assert( n == result );                                          \
        dstrfree(s);                                                    \
    } while (0)
//-------------------------------------------------

#define TEST_ITOA(n, result) do {                           \
        DSTR s = dstrnew();                                 \
        ditoa(s, n);                                        \
        assert( dstreq_sz(s, result) );                     \
        dstrfree(s);                                        \
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
        DSTR d = dstrnew_sz(s);                             \
        assert( disdigits(d) == result );                   \
        dstrfree(d);                                        \
    } while (0)

#define TEST_XDIGIT(s, result) do {                         \
        DSTR d = dstrnew_sz(s);                             \
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

void test_shared_dstr()
{
    TRACE_FN();

    DSTR s1 = dstrnew_sz("Hello");
    DSTR s2 = s1;
    assert( dstrdata(s1) == dstrdata(s2) );

    dstrresize(s1, 1024);
    assert( dstrdata(s1) == dstrdata(s2) );
}
//-------------------------------------------------

void test_move()
{
    TRACE_FN();

    DSTR s1 =   dstrnew_sprintf("%d", 100);
    TRACE_LN();

    for (int i=0; i < 80; ++i) {
        dstrcat_c(s1, ' ' + i);
    }
    TRACE_LN();

    char* p = dstrmove(s1);
    printf("%s\n", p);
    TRACE_LN();
    free(p);
    TRACE_LN();
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
    test_fromfile();
    test_append();
    test_move();
    test_format();
    test_remove();
    test_insert();
    test_replace();
    test_replace1();
    test_trim();
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
    test_fgets();
    test_atoi_itoa();
    test_isdigit();
    //test_shared_dstr();
    test_move();

    // last test
    test_getline();
}
