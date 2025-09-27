#include <iostream>
#include <vector>

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
}
