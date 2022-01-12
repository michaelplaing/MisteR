#ifndef PACK_INTERNAL_H
#define PACK_INTERNAL_H

#include "mister/packet.h"

typedef struct mr_mdata {
    const char *name;
    const int dtype;        // data type
    const uint8_t bp;       // bit position
    Word_t value;           // for any mdata value including pointers
    bool valloc;            // is value allocated
    size_t vlen;            // for sub-byte values, pointer values, vectors & VBIs
    bool vexists;           // value has been set
    const int link;         // end of range for VBI; byte to stuff for bit mdata
    const int propid;       // property id
    const int flagid;       // flag id
    const int idx;          // integer position in the mdata vector
    bool ualloc;            // is u8v0 allocated
    size_t u8vlen;
    uint8_t *u8v0;          // packed value
    bool pvalloc;           // is pvalue allocated?
    char *pvalue;           // stringified value
} mr_mdata;

typedef int (*mr_mdata_fn)(struct packet_ctx *pctx, struct mr_mdata *mdata);

typedef struct mr_dtype {
    const int idx;
    const char *name;
    const mr_mdata_fn count_fn;
    const mr_mdata_fn pack_fn;
    const mr_mdata_fn unpack_fn;
    const mr_mdata_fn output_fn;
    const mr_mdata_fn validate_fn;
    const mr_mdata_fn free_fn;
} mr_dtype;

enum mr_dtypes {
    MR_U8_DTYPE,
    MR_U16_DTYPE,
    MR_U32_DTYPE,
    MR_VBI_DTYPE,
    MR_BITS_DTYPE,
    MR_U8V_DTYPE,
    MR_STR_DTYPE,
    MR_SPV_DTYPE,
    MR_FLAGS_DTYPE,
    MR_PROPS_DTYPE
};

int mr_init_unpack_packet(
    packet_ctx **ppctx,
    const mr_mdata *MDATA_TEMPLATE, size_t mdata_count,
    uint8_t *u8v0, size_t ulen
);

int mr_init_packet_context(
    packet_ctx **ppctx, const mr_mdata *MDATA_TEMPLATE, size_t mdata_count
);

static int mr_unpack_packet(packet_ctx *pctx);
int mr_pack_packet(packet_ctx *pctx);

int mr_validate_u8vutf8(packet_ctx *pctx, int idx);

int mr_free_packet_context(packet_ctx *pctx);

static int mr_output_scalar(packet_ctx *pctx, mr_mdata *mdata);
static int mr_output_hexdump(packet_ctx *pctx, mr_mdata *mdata);
static int mr_output_string(packet_ctx *pctx, mr_mdata *mdata);
static int mr_output_spv(packet_ctx *pctx, mr_mdata *mdata);
int mr_mdata_dump(packet_ctx *pctx);

int mr_reset_scalar(packet_ctx *pctx, int idx);
int mr_reset_vector(packet_ctx *pctx, int idx);

int mr_set_scalar(packet_ctx *pctx, int idx, Word_t value);

static int mr_get_scalar(packet_ctx *pctx, int idx, Word_t *pvalue, bool *pexists);
int mr_get_boolean(packet_ctx *pctx, int idx, bool *pboolean, bool *pexists);
int mr_get_u8(packet_ctx *pctx, int idx, uint8_t *pu8, bool *pexists);
int mr_get_u16(packet_ctx *pctx, int idx, uint16_t *pu16, bool *pexists);
int mr_get_u32(packet_ctx *pctx, int idx, uint32_t *pu32, bool *pexists);

int mr_set_vector(packet_ctx *pctx, int idx, void *pvoid, size_t len);

static int mr_get_vector(packet_ctx *pctx, int idx, Word_t *ppvoid, size_t *plen, bool *pexists);
int mr_get_str(packet_ctx *pctx, int idx, char **pcv0, bool *pexists);
int mr_get_u8v(packet_ctx *pctx, int idx, uint8_t **pu8v0, size_t *plen, bool *pexists);
int mr_get_spv(packet_ctx *pctx, int idx, string_pair **pspv0, size_t *plen, bool *pexists);

static int mr_free_vector(packet_ctx *pctx, mr_mdata *mdata);

static int mr_count_u8(packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_u8(packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u8(packet_ctx *pctx, mr_mdata *mdata);

static int mr_count_u16(packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_u16(packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u16(packet_ctx *pctx, mr_mdata *mdata);

static int mr_count_u32(packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_u32(packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u32(packet_ctx *pctx, mr_mdata *mdata);

static int mr_count_str(packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_str(packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_str(packet_ctx *pctx, mr_mdata *mdata);
static int mr_validate_str(packet_ctx *pctx, mr_mdata *mdata);

static int mr_count_VBI(packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_VBI(packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_VBI(packet_ctx *pctx, mr_mdata *mdata);

static int mr_count_u8v(packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_u8v(packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u8v(packet_ctx *pctx, mr_mdata *mdata);

static int mr_pack_bits(packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_incr1(packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_bits(packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_incr1(packet_ctx *pctx, mr_mdata *mdata);

static int mr_count_spv(packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_spv(packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_spv(packet_ctx *pctx, mr_mdata *mdata);
static int mr_validate_spv(packet_ctx *pctx, mr_mdata *mdata);
static int mr_free_spv(packet_ctx *pctx, mr_mdata *mdata);

static int mr_unpack_props(packet_ctx *pctx, mr_mdata *mdata);

static int mr_output_scalar(packet_ctx *pctx, mr_mdata *mdata);
static int mr_output_hexdump(packet_ctx *pctx, mr_mdata *mdata);
static int mr_output_string(packet_ctx *pctx, mr_mdata *mdata);
static int mr_output_spv(packet_ctx *pctx, mr_mdata *mdata);
#endif // PACK_INTERNAL_H
