#ifndef DSTR_INTERNAL_H
#define DSTR_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

// Only the C++ wrapper ever touches this
//
extern void (*g_dstr_oom_handler)(void);

#ifdef __cplusplus
}
#endif

#endif
