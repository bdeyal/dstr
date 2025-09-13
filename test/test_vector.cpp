#include <iostream>
#include <vector>

#include <dstr/dstring.hpp>

std::ostream& operator<<(std::ostream& out, const DString& ds)
{
    return out << ds.c_str();
}

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
