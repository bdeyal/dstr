#ifndef DSTR_ALTNAMES_H
#define DSTR_ALTNAMES_H

#ifndef DSTR_H_INCLUDED
#error dstr_altnames.h should not be included directly
#endif

#if !defined(DSTR_CLEAN_NAMESPACE)

/* Creation functions. DSTR returned by value */
#define dstrnew             dstr_create
#define dstrnew_reserve     dstr_create_reserve
#define dstrnew_sz          dstr_create_sz
#define dstrnew_bl          dstr_create_bl
#define dstrnew_cc          dstr_create_cc
#define dstrnew_sprintf     dstr_create_sprintf
#define dstrnew_vsprintf    dstr_create_vsprintf
#define dstrnew_ds          dstr_create_ds
#define dstrnew_substr      dstr_create_substr

#define dstrdata            dstr_cstring
#define dstrempty           dstr_isempty
#define dstrlen             dstr_length
#define dstrcap             dstr_capacity

/*
 * Search functions. suffix *_i means ignore case
 */
#define dstrchr             dstr_find_c
#define dstrstr             dstr_find_sz
#define dstrchr_i           dstr_ifind_c
#define dstrstr_i           dstr_ifind_sz
#define dstrhas             dstr_contains_sz
#define dstrhas_i           dstr_icontains_sz
#define dendswith           dstr_suffix_sz
#define dendswith_i         dstr_isuffix_sz
#define dstartswith         dstr_prefix_sz
#define dstartswith_i       dstr_iprefix_sz
#define dindex_ok           dstr_valid_index
#define dstrblank           dstr_isblank

#define dstrffo_sz          dstr_ffo_sz
#define dstrffo_ds          dstr_ffo_ds
#define dstrffno_sz         dstr_ffno_sz
#define dstrffno_ds         dstr_ffno_ds

#define dstrcmp_sz          dstr_compare_sz
#define dstrcmp_ds          dstr_compare_ds
#define dstreq_sz           dstr_equal_sz
#define dstreq_i            dstr_iequal_sz
#define dstreq_ds           dstr_equal_ds

#define dstrchop            dstr_chop
#define dstrchomp           dstr_trim_right
#define dsubstr_bl          dstr_substr
#define dstrputc            dstr_putchar
#define dstrgetc            dstr_getchar
#define dstrputc_s          dstr_putchar_safe
#define dstrgetc_s          dstr_getchar_safe

#define dstrcpy_cc          dstr_assign_cc
#define dstrcpy_sz          dstr_assign_sz
#define dstrcpy_bl          dstr_assign_bl
#define dstrcpy_ds          dstr_assign_ds
#define dstrcpy_substr      dstr_assign_substr

#define dstrcat_sz          dstr_append_sz
#define dstrcat_cc          dstr_append_cc
#define dstrcat_c           dstr_append_c
#define dstrcat_ds          dstr_append_ds
#define dstrcat_bl          dstr_append_bl

#define dstrfree            dstr_destroy
#define dstrtrunc           dstr_truncate
#define dstrresize          dstr_resize
#define dstrerase           dstr_remove

#define dsprintf            dstr_assign_sprintf
#define dvsprintf           dstr_assign_vsprintf
#define dstrcat_sprintf     dstr_append_sprintf
#define dstrcat_vsprintf    dstr_append_vsprintf

#define dreplace_cc         dstr_replace_cc
#define dreplace_sz         dstr_replace_sz
#define dreplace_ds         dstr_replace_ds
#define dreplace_bl         dstr_replace_bl

#define dinsert_cc          dstr_insert_cc
#define dinsert_sz          dstr_insert_sz
#define dinsert_ds          dstr_insert_ds
#define dinsert_bl          dstr_insert_bl

#define dstrtrim            dstr_trim_both
#define dstrtrim_r          dstr_trim_right
#define dstrtrim_l          dstr_trim_left
#define dstrupper           dstr_ascii_upper
#define dstrlower           dstr_ascii_lower
#define dstrrev             dstr_reverse
#define dstrswap            dstr_swap
#define dgetline            dstr_fgetline
#define dgets               dstr_fgets

#define datoi               dstr_atoi
#define datol               dstr_atoi
#define datoll              dstr_atoll
#define ditoa               dstr_itoa
#define disdigits           dstr_isdigits
#define disxdigits          dstr_isxdigits
#define dstrhash            dstr_hash

/* clean namespace */
#endif

/* include guard */
#endif
