#ifndef CONNACK_H
#define CONNACK_H

#include "mister/packet.h"

int mr_init_connack_pctx(packet_ctx **ppctx);
int mr_pack_connack_u8v0(packet_ctx *pctx);
int mr_unpack_connack_u8v0(packet_ctx *pctx);
int mr_free_connack_pctx(packet_ctx *pctx);

typedef struct mr_connack_values { // may or may not be useful
    const uint8_t packet_type;
    uint32_t remaining_length;
} mr_connack_values;

#endif /* CONNACK_H */
