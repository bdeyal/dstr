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

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <dstr/dstring.hpp>

using namespace std;

// Compare to 'dstr_fgets' to 'cin >> str' must be equal
//
void test_dgets(const char* fname)
{
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
    }
    fclose(fp);

    cout << __func__ << ": Good! all \'words\' were equal" << endl;
}
//-------------------------------------------------

// Compare to 'dstr_fgets' to 'cin >> str' must be equal
//
void test_dgets_2(const char* fname)
{
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

    DString s1;
    DString s2;

    for (;;) {
        int r1 = s1.fgets(fp);
        in >> s2;

        if (r1 == EOF) {
            assert(in.eof());
            break;
        }

        assert(s1 == s2);
    }
    fclose(fp);

    cout << __func__ << ": Good! all lines were equal" << endl;
}
//-------------------------------------------------

// Compare to 'dstr_getline' to 'std::getline(in, str)' must be equal
//
void test_getline(const char* fname)
{
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
    }
    fclose(fp);

    cout << __func__ << ": Good! all lines were equal" << endl;
}
//-------------------------------------------------

void test_getline_2(const char* fname)
{
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

    DString s1;
    DString s2;
    size_t maxline = 0;

    for (;;) {
        int r1 = s1.fgetline(fp);
        io_getline(in, s2);

        if (r1 == EOF) {
            assert(in.eof());
            break;
        }

        if (s1 != s2) {
            cout << "\"" << s1 << "\"" << endl;
            cout << "\"" << s2 << "\"" << endl;
        }
        assert(s1 == s2);

        if (s2.size() > maxline)
            maxline = s2.size();
    }
    fclose(fp);

    cout << __func__ << ": Good! all lines were equal. Max len = " << maxline << endl;
}
//-------------------------------------------------


// Compare to 'dstr_getline' to 'std::getline(in, str)' must be equal
//
void test_fromfile(const char* fname)
{
    DSTR d1 = dstrnew_slurp(fname);
    if (!d1) {
        perror(fname);
        exit(1);
    }

    ifstream in(fname);
    if (!in) {
        cerr << "couldn't open "<< fname << ": " << strerror(errno) << endl;
        exit(1);
    }

    ostringstream out;
    out << in.rdbuf();

    std::string s1 = out.str();

    assert(s1 == dstrdata(d1));

    cout << "test_fromfile: Good! file content equal" << endl;

    dstrfree(d1);
}
//-------------------------------------------------

void test_insert_operator(const char* fname)
{
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
            break;
        }

        ostringstream out1;
        ostringstream out2;
        out1 << d1;
        out2 << s1;
        assert(out1.str() == out2.str());
    }
    fclose(fp);

    cout << __func__ << ": Good! operator<< equal for DString and std::string" << endl;
}
//-------------------------------------------------


int main(int argc, char* argv[])
{
    const char* fname =
        (argc > 1) ?
        argv[1] :
        __FILE__;

    cout << "Testing file: " << fname << endl;
    test_dgets(fname);
    test_dgets_2(fname);
    test_getline(fname);
    test_getline_2(fname);
    test_fromfile(fname);
    test_insert_operator(fname);

    cout << endl;
}
