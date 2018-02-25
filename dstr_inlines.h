/*
 * dstr_inlines.h
 *
 */
#ifndef DSTR_INLINES_H_
#define DSTR_INLINES_H_

#ifndef DSTR_H_INCLUDED
#error dstr_inlines.h should not be included directly
#endif


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

static __inline size_t dstr_length(const DSTR p)
{
    return DLEN(p);
}
/*-------------------------------------------------------------------------------*/

static __inline size_t dstr_capacity(const DSTR p)
{
    return DCAP(p);
}
/*-------------------------------------------------------------------------------*/

static __inline int dstr_isempty(const DSTR p)
{
    return (DLEN(p) == 0);
}
/*-------------------------------------------------------------------------------*/

static __inline const char* dstr_cstring(const DSTR p)
{
    return DBUF(p);
}
/*-------------------------------------------------------------------------------*/

static __inline int dstr_valid_index(const DSTR p, size_t pos)
{
    return (pos < DLEN(p));
}
/*-------------------------------------------------------------------------------*/

static __inline char dstr_getchar(const DSTR p, size_t pos)
{
    return DVAL(p, pos);
}
/*-------------------------------------------------------------------------------*/

static __inline void dstr_putchar(DSTR p, size_t pos, char c)
{
    DVAL(p, pos) = c;
}
/*-------------------------------------------------------------------------------*/

static __inline char dstr_getchar_safe(const DSTR p, long pos)
{
    if (pos < 0)
        pos += (long) DLEN(p);

    return dstr_valid_index(p, pos) ? DVAL(p, pos) : (char)'\0';
}
/*-------------------------------------------------------------------------------*/

static __inline void dstr_putchar_safe(DSTR p, long pos, char c)
{
    if (pos < 0)
        pos += (long) DLEN(p);

    if (dstr_valid_index(p, pos))
        DVAL(p, pos) = c;
}
/*-------------------------------------------------------------------------------*/

static __inline void dstr_chop(DSTR p)
{
    if (DLEN(p) > 0) {
        DLEN(p) -= 1;
        DVAL(p, DLEN(p)) = '\0';
    }
}
/*-------------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif


#endif /* DSTR_INLINES_H_ */
