#ifndef DSTR_INTERNAL_H
#define DSTR_INTERNAL_H

#define REGEX_COMPILE_ERROR_BASE 10000

// This function is not part of public C API but exported since it is
// needed in the C++ wrapper.  we must force C name and linkage
//
#ifdef __cplusplus
extern "C"  {
#endif

int dstr_grow_ctor(DSTR p, size_t len);
void dstr_out_of_memory(void);

#ifdef __cplusplus
}
#endif


#endif
