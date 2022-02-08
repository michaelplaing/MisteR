#ifndef MEMORY_INTERNAL_H
#define MEMORY_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

int mr_calloc(void **ppv, size_t count, size_t sz);
int mr_malloc(void **ppv, size_t sz);
int mr_realloc(void **ppv, size_t sz);
int mr_free(void *pv);

#ifdef __cplusplus
}
#endif

#endif // MEMORY_INTERNAL_H
