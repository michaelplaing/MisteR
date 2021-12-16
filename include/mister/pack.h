#ifndef PACK_H
#define PACK_H

#ifndef _PVOID_T
#define _PVOID_T
typedef void *   Pvoid_t;
typedef void ** PPvoid_t;
#endif

#ifndef _WORD_T
#define _WORD_T
typedef unsigned long    Word_t, * PWord_t;  // expect 32-bit or 64-bit words.
#endif

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
