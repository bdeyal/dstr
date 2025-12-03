/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#include <stdlib.h>
#include <dstr/dstring.hpp>
#include "dstr_internal.h"

void DString::on_regex_error(int rc)
{
    DString msg;
    dstr_regex_strerror(msg.pImp(), rc);
    throw DStringError(std::move(msg));
}
/*-------------------------------------------------------------------------------*/

namespace {
struct Init_Regex_Handler {
    Init_Regex_Handler() {
        g_dstr_regex_handler = &DString::on_regex_error;
    }
};
static Init_Regex_Handler init_Regex_handler;
//-----------------------------------------------------------
}

//////////////////////////////////////////////////////////
//
//   DStringView Regex - all constant, subject unchanged
//
//////////////////////////////////////////////////////////

bool DStringView::match(DStringView pattern, size_t offset) const
{
    return dstr_regex_exact(pImp(), pattern.c_str(), offset);
}
/*-------------------------------------------------------------------------------*/

size_t DStringView::match_contains(DStringView pattern, size_t offset) const
{
    return dstr_regex_contains(pImp(), pattern.c_str(), offset);
}
/*-------------------------------------------------------------------------------*/

int DStringView::match(DStringView pattern, size_t offset,
                       DString::Match& m, const char* opts) const
{
    return dstr_regex_match(pImp(), pattern.c_str(), offset, &m, opts);
}
/*-------------------------------------------------------------------------------*/

int DStringView::match_groups(DStringView pattern, size_t offset,
                              DString::MatchVector& matches_out,
                              const char* opts) const
{
    struct MV_Wrapper : DSTR_Match_Vector {
        MV_Wrapper()  { matches = nullptr; matches_len = 0; }
        ~MV_Wrapper() { if (matches) free(matches); }
    };

    MV_Wrapper vec;
    int rc = dstr_regex_match_groups(pImp(), pattern.c_str(),
                                     offset, &vec, opts);

    DString::MatchVector tmp;
    for (size_t i = 0; i < vec.matches_len; ++i) {
        tmp.push_back(vec.matches[i]);
    }

    matches_out.swap(tmp);
    return rc;
}
/*-------------------------------------------------------------------------------*/

int DStringView::capture(DStringView pattern, size_t offset, DString& result,
                         const char* opts) const
{
    DString::Match mtch;
    int rc = dstr_regex_match(pImp(), pattern.c_str(), offset, &mtch, opts);

    if (rc > 0 && mtch.offset != DString::NPOS)
        result = substr(mtch.offset, mtch.length);

    return rc;
}
/*-------------------------------------------------------------------------------*/

int DStringView::capture(DStringView pattern,
                         size_t offset,
                         std::vector<DString>& vec,
                         const char* opts) const
{
    DString::MatchVector matches;
    int rc = match_groups(pattern, offset, matches, opts);

    std::vector<DString> tmp;
    for (const auto& m: matches) {
        if (m.offset != DString::NPOS) {
            tmp.push_back({*this, m.offset, m.length}); }
        else {
            tmp.push_back(""); } }

    vec.swap(tmp);
    return rc;
}
/*-------------------------------------------------------------------------------*/


////////////////////////////////////////////////////////////
//
//   DString Regex - For const functions reuse view() code
//
////////////////////////////////////////////////////////////

bool DString::match(DStringView pattern, size_t offset) const
{
    return view().match(pattern, offset);
}
/*-------------------------------------------------------------------------------*/

size_t DString::match_contains(DStringView pattern, size_t offset) const
{
    return view().match_contains(pattern, offset);
}
/*-------------------------------------------------------------------------------*/

DString DString::capture(DStringView pattern,
                         size_t offset,
                         const char* opts) const
{
    DString result;
    view().capture(pattern, offset, result, opts);
    return result;
}
/*-------------------------------------------------------------------------------*/

int DString::capture(DStringView pattern, size_t offset,
                     std::vector<DString>& vec, const char* opts) const
{
    return view().capture(pattern, offset, vec, opts);
}
/*-------------------------------------------------------------------------------*/

int DString::match(DStringView pattern, size_t offset, DString::Match& m,
                   const char* opts) const
{
    return view().match(pattern, offset, m, opts);
}
/*-------------------------------------------------------------------------------*/

int DString::match_groups(DStringView pattern, size_t offset,
                          DString::MatchVector& matches, const char* opts) const
{
    return view().match_groups(pattern, offset, matches, opts);
}
/*-------------------------------------------------------------------------------*/

int DString::subst_inplace(DStringView pattern, size_t offset,
                           DStringView replacement, const char* opts)
{
    return dstr_regex_substitute(pImp(), pattern.c_str(),
                                 offset, replacement.c_str(), opts);
}
/*-------------------------------------------------------------------------------*/
