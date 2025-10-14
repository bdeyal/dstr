#include <iostream>
#include <vector>
#include <list>

#include <dstr/dstring.hpp>

int main()
{
    using namespace std;

    std::vector<DString> v;

    v.push_back("Hello");
    v.push_back("World");
    v.push_back("Today");
    v.push_back("Is");
    v.push_back("SAT");
    for (const auto& i : v) {
        cout << i << ' ';
    }
    cout << endl;

    std::list<DString> l;

    l.push_back("Hello");
    l.push_back("World");
    l.push_back("Today");
    l.push_back("Is");
    l.push_back("SAT");
    for (const auto& i : l) {
        cout << i << ' ';
    }
    cout << endl;
}
