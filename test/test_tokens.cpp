#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>

#include <dstr/dstring.hpp>

std::ostream& operator<<(std::ostream& out, const DString& ds)
{
    return out << ds.c_str();
}

void string_tokens(const DString& s, const DString& sep, std::vector<DString>& result)
{
    using namespace std;

	vector<DString> tmp;

    // Find the first location which does not belong to
    // the separator characters
    //
    size_t first = s.ffno(sep, 0);

    // Check if we are at the end
    //
    while (first != DString::NPOS)
    {
        // Find the first location (> first) with a character
        // that belongs to the separator group
        //
        size_t last = s.ffo(sep, first);

        // Create a substring to print
        //
        DString token = DString(s, first, last - first);
        tmp.push_back(token);

        // Prepare for next iteration.
        // Again find the first char not in separator but now
        // not from the start
        //
        first = s.ffno(sep, last);
    }

	result.swap(tmp);
}
//-----------------------------------------------------

int main()
{
   using namespace std;

   DString s = "  Hello,World,,,\t\tToday is Tuesday 1.1.1.1, 2.2.2.2,33.33.33";

   vector<DString> v;
   string_tokens(s, ", \t", v);

   copy(v.begin(), v.end(), ostream_iterator<DString>(cout, "\n"));
}
