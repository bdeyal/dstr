/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>

#include <dstr/dstring.hpp>
//-----------------------------------------------------

int main()
{
   using namespace std;

   DString s = "  Hello,World,,,\t\tToday is Tuesday 1.1.1.1, 2.2.2.2,33.33.33";

   vector<DString> v;
   s.split(", \t", v);
   copy(v.begin(), v.end(), ostream_iterator<DString>(cout, "\n"));

   s.split(',', v);
   for (const auto& d : v) {
       cout << "\"" << d << "\"" << endl;
   }
}
