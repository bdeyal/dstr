/*
 * Copyright (c) 2025 Eyal Ben-David
 *
 * This file is part of DString C and C++ dynamic string library,
 * distributed under the GNU GPL v3.0. See LICENSE file for full GPL-3.0 license text.
 */
#include <stdlib.h>
#include <dstr/dstring.hpp>
#include "dstr_internal.h"

#if !defined(NO_DSTRING_REGEX)

void DString::on_regex_error(int rc)
{
    DString msg;
    dstr_regex_strerror(msg.pImp(), rc);
    throw DStringError(std::move(msg));
}
/*-------------------------------------------------------------------------------*/

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
    int rc = dstr_regex_match(pImp(), pattern.c_str(), offset, &m, opts);

    if (rc > REGEX_COMPILE_ERROR_BASE) {
        DString::on_regex_error(rc);  }

    if (rc < 0) {
        DString::on_regex_error(rc);  }

    return rc;
}
/*-------------------------------------------------------------------------------*/

int DStringView::match_groups(DStringView pattern, size_t offset,
                              DString::MatchVector& vec,
                              const char* opts) const
{
    int rc = dstr_regex_match_groups(pImp(), pattern.c_str(),
                                     offset, vec.self(), opts);

    if (rc > REGEX_COMPILE_ERROR_BASE) {
        DString::on_regex_error(rc); }

    if (rc < 0) {
        DString::on_regex_error(rc); }

    return rc;
}
/*-------------------------------------------------------------------------------*/

int DStringView::capture(DStringView pattern, size_t offset, DString& result,
                         const char* opts) const
{
    DString::Match mtch;
    int rc = match(pattern, offset, mtch, opts);

    if (rc > 0 && mtch.offset != DString::NPOS) {
        result = substr(mtch.offset, mtch.length); }

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

int DStringView::re_split(DStringView pattern, size_t offset,
                          std::vector<DString>& strings,
                          const char* options) const
{
    std::vector<DString> tmp;

    // Spliting on empty pattern = split on each char
    //
    if (pattern.size() == 0) {
        for (char ch : *this) {
            tmp.push_back(DString(ch, 1)); }
        tmp.swap(strings);
        return (int) strings.size(); }

    DString::MatchVector matches;
    int rc;
    while ((rc = match_groups(pattern, offset, matches, options)) > 0) {
        tmp.push_back({*this, offset, matches[0].offset - offset});

        // If have sub groups include them
        //
        for (size_t i = 1; i < matches.size(); ++i) {
            const auto& m = matches[i];
            if (m.offset != DString::NPOS) {
                tmp.push_back({*this, m.offset, m.length}); }
            else {
                tmp.push_back(""); } }

        offset = (matches[0].offset + matches[0].length); }

    tmp.push_back({*this, offset, size() - offset});

    tmp.swap(strings);
    return rc;
}

////////////////////////////////////////////////////////////
//
//   DString Regex - For const functions reuse view() code
//
////////////////////////////////////////////////////////////

int DString::subst_inplace(DStringView pattern, size_t offset,
                           DStringView replacement, const char* opts)
{
    int rc =  dstr_regex_substitute(pImp(), pattern.c_str(),
                                    offset, replacement.c_str(), opts);

    if (rc > REGEX_COMPILE_ERROR_BASE || rc < 0) {
        DString::on_regex_error(rc); }

    return rc;
}
/*-------------------------------------------------------------------------------*/

#endif
