#ifndef PACK_INTERNAL_H
#define PACK_INTERNAL_H

#include "mister/packet.h"

typedef struct mr_mdata {
    const char *name;
    const bool isprop;      // is it a property
    int (*pack_fn)(struct packet_ctx *pctx, struct mr_mdata *mdata);
    int (*unpack_fn)(struct packet_ctx *pctx, struct mr_mdata *mdata);
    Word_t value;           // can handle any mr_mdata value including pointers
    bool valloc;            // is value allocated
    size_t vlen;            // for sub-byte values, pointer values, vectors & VBIs
    bool vexists;           // value has been set
    const int link;         // end of range for VBI; byte to stuff for bit mdata
    const uint8_t id;       // for properties & bit position for flags
    const int index;        // integer position in the mdata vector
    bool ualloc;            // is u8v0 allocated
    size_t u8vlen;
    uint8_t *u8v0;          // packed value
} mr_mdata;

int mr_init_packet_context(
    packet_ctx **ppctx, const mr_mdata *MDATA_TEMPLATE, size_t mdata_count
);

int mr_free_packet_context(packet_ctx *pctx);

int mr_reset_value(packet_ctx *pctx, int index); // scalar or vector

int mr_set_scalar(packet_ctx *pctx, int index, Word_t value);

int mr_get_boolean(packet_ctx *pctx, int index, bool *pboolean);
int mr_get_u8(packet_ctx *pctx, int index, uint8_t *pu8);
int mr_get_u16(packet_ctx *pctx, int index, uint16_t *pu16);
int mr_get_u32(packet_ctx *pctx, int index, uint32_t *pu32);

int mr_set_vector(packet_ctx *pctx, int index, void *pvoid, size_t len);

int mr_get_u8v(packet_ctx *pctx, int index, uint8_t **pu8v0, size_t *plen);
int mr_get_spv(packet_ctx *pctx, int index, string_pair **pspv0, size_t *plen);

int mr_pack_mdata_u8v0(packet_ctx *pctx);
int mr_unpack_mdata_u8v0(packet_ctx *pctx);

int mr_pack_u8(packet_ctx *pctx, mr_mdata *mdata);
int mr_unpack_u8(packet_ctx *pctx, mr_mdata *mdata);

int mr_pack_u16(packet_ctx *pctx, mr_mdata *mdata);
int mr_unpack_u16(packet_ctx *pctx, mr_mdata *mdata);

int mr_pack_u32(packet_ctx *pctx, mr_mdata *mdata);

int mr_pack_str(packet_ctx *pctx, mr_mdata *mdata);

int mr_pack_VBI(packet_ctx *pctx, mr_mdata *mdata);
int mr_unpack_VBI(packet_ctx *pctx, mr_mdata *mdata);

int mr_pack_u8v(packet_ctx *pctx, mr_mdata *mdata);
int mr_unpack_u8v(packet_ctx *pctx, mr_mdata *mdata);

int mr_pack_bits(packet_ctx *pctx, mr_mdata *mdata);
int mr_unpack_bits(packet_ctx *pctx, mr_mdata *mdata);
int mr_unpack_incr1(packet_ctx *pctx, mr_mdata *mdata);

int mr_pack_prop_u8(packet_ctx *pctx, mr_mdata *mdata);

int mr_pack_prop_u16(packet_ctx *pctx, mr_mdata *mdata);

int mr_pack_prop_u32(packet_ctx *pctx, mr_mdata *mdata);
int mr_unpack_prop_u32(packet_ctx *pctx, mr_mdata *mdata);

int mr_pack_prop_u8v(packet_ctx *pctx, mr_mdata *mdata);

int mr_pack_prop_spv(packet_ctx *pctx, mr_mdata *mdata);

int mr_pack_prop_str(packet_ctx *pctx, mr_mdata *mdata);

int mr_unpack_prop_VBI(packet_ctx *pctx, mr_mdata *mdata);

int mr_unpack_props(packet_ctx *pctx, mr_mdata *mdata);

#endif // PACK_INTERNAL_H
