/*
 * dstr_impl.h
 *
 */
#ifndef DSTR_IMPL_H_
#define DSTR_IMPL_H_

#ifndef DSTR_H_INCLUDED
#error dstr_impl.h should not be included directly
#endif

typedef struct DSTR_IMP
{
    size_t length;
    size_t capacity;
    char*  data;
} DSTR_IMP;
/*--------------------------------------------------------------------------*/

/* allocation size at creation */
#define DSTR_INITIAL_CAPACITY (32UL)

#define BASE(p)        (p)
#define DBUF(p)        BASE(p)->data
#define DLEN(p)        BASE(p)->length
#define DCAP(p)        BASE(p)->capacity
#define DVAL(p, i)     DBUF(p)[(i)]
/*--------------------------------------------------------------------------*/

#define dstr_assert_valid(p) do {                      \
    assert((p) != NULL);                               \
    assert(DBUF(p) != NULL);                           \
    assert(DCAP(p) > 0);                               \
    assert(DLEN(p) < DCAP(p));                         \
    assert(DVAL(p, DLEN(p)) == '\0');                  \
    assert(DLEN(p) == strlen(DBUF(p)));                \
    assert((DCAP(p) % DSTR_INITIAL_CAPACITY) == 0);    \
} while(0)
/*--------------------------------------------------------------------------*/


#endif /* DSTR_IMPL_H_ */
