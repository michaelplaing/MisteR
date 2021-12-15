#ifndef PACK_H
#define PACK_H

#include <Judy.h>

struct mr_mdata;

typedef struct pack_ctx {
    uint8_t *buf;
    bool isalloc;
    size_t len;
    struct mr_mdata *mdata0;
    size_t mdata_count;
    Pvoid_t PJSLArray;
} pack_ctx;

typedef struct string_pair {
    uint8_t *name;
    uint8_t *value;
} string_pair;

#endif /* PACK_H */
