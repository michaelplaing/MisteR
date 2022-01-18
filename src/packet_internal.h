#ifndef PACKET_INTERNAL_H
#define PACKET_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

// a type that can be cast to/from a pointer or an int up to uint32_t
typedef unsigned long mvalue_t;

struct mr_mdata;

typedef struct mr_mdata {
    const char *name;
    const int dtype;        // data type
    const uint8_t bpos;       // bit position
    mvalue_t value;         // for any mdata value including pointers
    bool valloc;            // is value allocated
    size_t vlen;            // byte size of an integer scalar, # of bits for a sub-byte scalar,
                            // length of a vector - includes trailing '/0' for a string, i.e. strlen() + 1
    size_t u8vlen;          // byte count of packed value as a uint8_t vector
    bool vexists;           // value has been set
    const int link;         // end of range for VBI; byte to stuff for sub-byte scalar
    const int propid;       // property id if any
    const int flagid;       // flag id if any
    const int idx;          // integer position in the mdata vector
    bool ovalloc;           // is ovalue allocated?
    char *ovalue;           // stringified printable output value
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

int mr_init_packet(
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

int mr_set_scalar(packet_ctx *pctx, int idx, mvalue_t value);

static int mr_get_scalar(packet_ctx *pctx, int idx, mvalue_t *pvalue, bool *pexists);
int mr_get_boolean(packet_ctx *pctx, int idx, bool *pboolean, bool *pexists);
int mr_get_u8(packet_ctx *pctx, int idx, uint8_t *pu8, bool *pexists);
int mr_get_u16(packet_ctx *pctx, int idx, uint16_t *pu16, bool *pexists);
int mr_get_u32(packet_ctx *pctx, int idx, uint32_t *pu32, bool *pexists);

int mr_set_vector(packet_ctx *pctx, int idx, void *pvoid, size_t len);

static int mr_get_vector(packet_ctx *pctx, int idx, mvalue_t *ppvoid, size_t *plen, bool *pexists);
int mr_get_str(packet_ctx *pctx, int idx, char **pcv0, bool *pexists);
int mr_get_u8v(packet_ctx *pctx, int idx, uint8_t **pu8v0, size_t *plen, bool *pexists);
int mr_get_spv(packet_ctx *pctx, int idx, string_pair **pspv0, size_t *plen, bool *pexists);

static int mr_free_vector(packet_ctx *pctx, mr_mdata *mdata);

static int mr_pack_u8(packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u8(packet_ctx *pctx, mr_mdata *mdata);

static int mr_pack_u16(packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u16(packet_ctx *pctx, mr_mdata *mdata);

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

#ifdef __cplusplus
}
#endif

#endif // PACKET_INTERNAL_H
