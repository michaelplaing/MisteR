#ifndef PACK_H
#define PACK_H

#include "mister.h"

// expect either 32-bit or 64-bit words
// to be cast as pointer or whatever fits
typedef unsigned long Word_t;

struct mr_mdata;

typedef struct packet_ctx {
    uint8_t mqtt_packet_type;
    uint8_t *u8v0;
    bool isalloc;
    size_t len;
    size_t pos;
    size_t limit; // pos + property_length
    struct mr_mdata *mdata0;
    size_t mdata_count;
} packet_ctx;

typedef struct string_pair {
    uint8_t *name;
    uint8_t *value;
} string_pair;

#endif // PACK_H
