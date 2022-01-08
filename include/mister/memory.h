#ifndef MEMORY_H
#define MEMORY_H

int mr_calloc(void **ppv, size_t count, size_t sz);
int mr_malloc(void **ppv, size_t sz);
int mr_realloc(void **ppv, size_t sz);
int mr_free(void *pv);

#endif // MEMORY_H