#include <algorithm>
#include <iostream>
#include <cassert>
#include <cctype>

#include <dstr/dstring.hpp>

using namespace std;

void test_back_inserter()
{
    DString s1("Hello World");
    DString s2;
    assert(s1 != s2);

    std::copy(s1.begin(), s1.end(), back_inserter<DString>(s2));
    assert(s1 == s2);

    s1 = "Hello World today is SAT deep at night";
    s2 = "";
    std::copy(s1.begin(), s1.end(), back_inserter<DString>(s2));
    assert(s1 == s2);
}
//--------------------------------------------------------------

void test_for()
{
    DString s1("Hello World");

    for (auto c : s1) {
        cout << c << ", ";
    }
    cout << endl;

}
//--------------------------------------------------------------

void test_sort()
{
    DString s1("Hello World");
    std::sort(s1.begin(), s1.end());
    cout << s1.c_str() << endl;
}
//--------------------------------------------------------------

void test_transform()
{
    DString s1("Hello World");
    DString s2;

    transform(s1.begin(),
              s1.end(),
              std::back_inserter<DString>(s2),
              [] (char c) { return toupper(c); });

    cout << s2.c_str() << endl;
    assert(s2 == "HELLO WORLD");
}
//--------------------------------------------------------------

void test_reverse()
{
    DString s1("Hello World Today is SAT deep at night");
    DString s2(s1);
    assert(s1 == s2);

    std::reverse(s1.begin(), s1.end());
    s2.reverse();

    cout << s1.c_str() << endl;
    cout << s2.c_str() << endl;
    assert(s1 == s2);
}
//--------------------------------------------------------------

void test_permutations()
{
    DString s1("ABCD");

    do {
        cout << s1.c_str() << endl;
    } while (next_permutation(s1.begin(), s1.end()));
}
//--------------------------------------------------------------

void test_reverse_iterator()
{
    DString s1("Hello World today is SAT deep at night");

    auto first = reverse_iterator<DString::iterator>(s1.end());
    auto last  = reverse_iterator<DString::iterator>(s1.begin());

    while (first != last) {
        cout.put(*first++);
    }
    cout.put('\n');
}
//--------------------------------------------------------------

int main()
{
    test_back_inserter();
    test_for();
    test_sort();
    test_transform();
    test_reverse();
    test_permutations();
    test_reverse_iterator();
}
