#ifndef MEMORY_INTERNAL_H
#define MEMORY_INTERNAL_H

int mr_calloc(void **ppv, size_t count, size_t size);
int mr_malloc(void **ppv, size_t len);
int mr_realloc(void **ppv, size_t ulen);
int mr_reallocf(void **ppv, size_t ulen);

#endif // MEMORY_INTERNAL_H