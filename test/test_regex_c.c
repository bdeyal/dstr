/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#include <assert.h>
#include <dstr/dstr.h>


#if defined(__BORLANDC__) || (defined(_MSC_VER) && (_MSC_VER <= 1200))
#define TRACE_FN() printf("%d\n", __LINE__)
#else
#define TRACE_FN() printf("\nFUNCTION: %s()\n", __FUNCTION__)
#endif


bool isIP(CDSTR p)
{
    return dre_exact(p, "(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})", 0);
}
//--------------------------------------------------------------------------------

void test_ip_address()
{
    TRACE_FN();

    DSTR s = dstrnew("1.2.3.4");
    assert(isIP(s));

    dstrcpy(s, "192.168.1.26");
    assert(isIP(s));

    dstrcpy(s, "1922.168.1.26");
    assert(!isIP(s));

    dstrcpy(s, "192.1681.1.26");
    assert(!isIP(s));

    dstrcpy(s, "192.168.1111.26");
    assert(!isIP(s));

    dstrcpy(s, "192.168.1.2611");
    assert(!isIP(s));

    dstrfree(s);
}
//--------------------------------------------------------------------------------

void test_dstr_subst()
{
    TRACE_FN();

    DSTR s = dstrnew("123");
    dre_subst(s, "[0-9]+", 0, "ABC", NULL);
    assert(dstreq(s, "ABC"));

    dstrcpy(s, "123 456");
    dre_subst(s, "([0-9]+) ([0-9]+)", 0, "$2 $1 $2", NULL);
    assert(dstreq(s, "456 123 456"));

    const char* pattern = "(([\\w\\.-]+)@([\\w\\.-]+\\.\\w+))";
    dstrcpy(s, "Emails: alice@foo.com, bob@bar.org, charlie@baz.net");
    dre_subst(s, pattern, 0, "***@$2", "/g");
    puts(dstrdata(s));

    DSTR hello = dstrnew("hello");
    dre_subst(hello, "[aeiou]", 0, "*", NULL);
    assert(dstreq(hello, "h*llo"));

    dstrcpy(hello, "hello");
    dre_subst(hello, "[aeiou]", 0, "*", "/g");
    assert(dstreq(hello, "h*ll*"));

    dstrcpy(hello, "hello");
    dre_subst(hello, "[aeiou]", 0, "", "/g");
    assert(dstreq(hello, "hll"));

    dstrcpy(hello, "hello");
    dre_subst(hello, "ell", 0, "al", NULL);
    assert(dstreq(hello, "halo"));

    dstrcpy(hello, "hello");
    dre_subst(hello, "xyzzy", 0, "*", NULL);
    assert(dstreq(hello, "hello"));

    dstrcpy(hello, "THX1138");
    dre_subst(hello, "\\d+", 0, "00", NULL);
    assert(dstreq(hello, "THX00"));

    dstrcpy(hello, "Regex methods\t\t in    C");
    dre_subst(hello, "\\s+", 0, "-", "/g");
    assert(dstreq(hello, "Regex-methods-in-C"));

    dstrcpy(hello, "hello");
    dre_subst(hello, "(?<vowel>[aeiou])", 0, "${vowel}${vowel}", "/g");
    assert(dstreq(hello, "heelloo"));

    dstrfree(hello);
    dstrfree(s);
}
//--------------------------------------------------------------------------------

void test_dstr_group_patterns()
{
    TRACE_FN();

    // Pattern with groups: (username)@(domain)
    const char* pattern = "(([\\w\\.-]+)@([\\w\\.-]+\\.\\w+))";

    DSTR subject = dstrnew("Emails: alice@foo.com, bob@bar.org, charlie@baz.net");

    // Find matches with groups starting from offset 8 ("Emails: ")
    //
    size_t offset = 8;
    DSTR mstr = dstrnew_empty();
    for (;;) {
        DSTR_Regex_Match matches[20];
        int n = dre_groups(subject, pattern, offset, matches, 20, NULL);
        if (n == 0) break;

        printf("Found %d matches:\n", n);
        for (int i = 0; i < n; ++i) {
            printf("  Offset: %zu, Length: %zu ==> ", matches[i].offset, matches[i].length);
            dstrcpy_substr(mstr, subject, matches[i].offset, matches[i].length);
            printf("\"%s\"\n", dstrdata(mstr));
        }

        offset = matches[0].offset + matches[0].length;
    }

    dstrfree(subject);
    dstrfree(mstr);
}
//--------------------------------------------------------------------------------

void test_dstr_group_extract()
{
    TRACE_FN();

    const char *pattern = "(?<date>(?<year>\\d{4})-(?<month>\\d{2})-(?<day>\\d{2}))";
    DSTR subject = dstrnew("Today is 2025-10-29 and tomorrow is 2025-10-30");

    size_t offset = 0;
    DSTR mstr = dstrnew_empty();
    for (;;) {
        DSTR_Regex_Match matches[20];
        int n = dre_groups(subject, pattern, offset, matches, 20, NULL);
        if (n == 0) break;

        printf("Found %d matches:\n", n);
        for (int i = 0; i < n; ++i) {
            printf("  Offset: %zu, Length: %zu, Name: %s ==> ",
                   matches[i].offset,
                   matches[i].length,
                   matches[i].name);

            dstrcpy_substr(mstr, subject, matches[i].offset, matches[i].length);
            printf("\"%s\"\n", dstrdata(mstr));
        }

        offset = matches[0].offset + matches[0].length;
    }

    dstrfree(mstr);
    dstrfree(subject);
}
//--------------------------------------------------------------------------------

int main()
{
    test_ip_address();
    test_dstr_subst();
    test_dstr_group_patterns();
    test_dstr_group_extract();
}
//--------------------------------------------------------------------------------
