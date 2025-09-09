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
#include "dstr.h"

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

    DSTR d1 = dstrnew_reserve(500);
    std::string s1;

    for (;;) {
        int r1 = dstr_fgets(d1, fp);
        in >> s1;

        if (r1 == EOF) {
            assert(in.eof());
            break;
        }

        assert(s1 == dstrdata(d1));
    }

    cout << "test_dgets: Good! all lines were equal" << endl;

    dstrfree(d1);
    fclose(fp);
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

    DSTR d1 = dstrnew_reserve(500);
    std::string s1;

    for (;;) {
        int r1 = dstr_fgetline(d1, fp);
        std::getline(in, s1);

        if (r1 == EOF) {
            assert(in.eof());
            break;
        }

        assert(s1 == dstrdata(d1));
    }

    cout << "test_getline: Good! all lines were equal" << endl;

    dstrfree(d1);
    fclose(fp);
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


int main(int argc, char* argv[])
{
    const char* fname =
        (argc > 1) ?
        argv[1] :
        __FILE__;

    cout << "Testing file: " << fname << endl;
    test_dgets(fname);
    test_getline(fname);
    test_fromfile(fname);
}
