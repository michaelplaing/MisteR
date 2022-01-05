#ifndef CONNACK_H
#define CONNACK_H

#include "mister/packet.h"

int mr_init_connack_pctx(packet_ctx **ppctx);
int mr_pack_connack_u8v0(packet_ctx *pctx);
int mr_unpack_connack_u8v0(packet_ctx *pctx);
int mr_free_connack_pctx(packet_ctx *pctx);

#endif /* CONNACK_H */
