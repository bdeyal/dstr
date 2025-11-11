/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#include <iostream>
#include <map>

#include <dstr/dstring.hpp>

int main()
{
    using namespace std;

    std::map<DString, int> dict;

    dict["One"] = 1;
    dict["Two"] = 2;
    dict["Three"] = 3;
    dict["Four"] = 4;
    dict["Five"] = 5;

    for (const auto& i : dict) {
        cout << i.first << " = " << i.second << endl;
    }
}
