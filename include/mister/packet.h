#ifndef PACK_H
#define PACK_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "mister.h"

// expect either 32-bit or 64-bit words
// to be cast as pointer or whatever fits
typedef unsigned long Word_t;

struct mr_mdata;

typedef struct packet_ctx {
    uint8_t mqtt_packet_type;
    char *mqtt_packet_name;
    char *mdata_dump;
    uint8_t *u8v0;
    bool ualloc;
    size_t len;
    size_t pos;
    struct mr_mdata *mdata0;
    size_t mdata_count;
} packet_ctx;

typedef struct string_pair {
    char *name;
    char *value;
} string_pair;

int mr_print_existing_mdata(packet_ctx *pctx);

#endif // PACK_H
