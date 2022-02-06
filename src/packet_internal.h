#ifndef PACKET_INTERNAL_H
#define PACKET_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/// a type that can be cast to/from a pointer or an int up to uint32_t
typedef unsigned long mr_mvalue_t;

typedef struct mr_mdata {
    const char *name;       ///< field name from spec
    const int dtype;        ///< data type
    const uint8_t bitpos;   ///< bit position
    mr_mvalue_t value;      ///< for any mdata scalar value or pointer
    bool valloc;            ///< value allocated?
    size_t vlen;            ///< integer byte size OR sub-byte # of bits OR vector byte length
    size_t u8vlen;          ///< byte count of packed value as a uint8_t vector
    bool vexists;           ///< value set?
    const int link;         ///< end of range for VBI; byte to stuff for sub-byte scalar
    const int propid;       ///< property id if any
    const int flagid;       ///< controlling flag id if any
    const int idx;          ///< offset of the mdata in the packet's mdata0 vector
    char *printable;        ///< c-string printable value
} mr_mdata;

typedef int (*mr_mdata_fn)(struct mr_packet_ctx *pctx, struct mr_mdata *mdata);

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
    MR_PROPERTIES_DTYPE
};

typedef int (*mr_ptype_fn)(struct mr_packet_ctx *pctx);

typedef struct mr_ptype {
    const int mqtt_packet_type;
    const char *mqtt_packet_name;
    const mr_ptype_fn ptype_fn;
} mr_ptype;

int mr_init_packet(
    mr_packet_ctx **ppctx, const mr_mdata *MDATA_TEMPLATE, const size_t mdata_count
);

static int mr_unpack_packet(mr_packet_ctx *pctx);

int mr_init_unpack_packet(
    mr_packet_ctx **ppctx,
    const mr_mdata *MDATA_TEMPLATE,
    const size_t mdata_count,
    const uint8_t *u8v0,
    const size_t ulen
);

int mr_pack_packet(mr_packet_ctx *pctx);
int mr_free_packet_context(mr_packet_ctx *pctx);

int mr_validate_utf8_values(mr_packet_ctx *pctx);

static int mr_get_scalar(mr_packet_ctx *pctx, const int idx, mr_mvalue_t *pvalue, bool *pexists);
int mr_set_scalar(mr_packet_ctx *pctx, const int idx, const mr_mvalue_t value);
int mr_reset_scalar(mr_packet_ctx *pctx, const int idx);

int mr_get_boolean(mr_packet_ctx *pctx, const int idx, bool *pboolean, bool *pexists);

int mr_get_u8(mr_packet_ctx *pctx, const int idx, uint8_t *pu8, bool *pexists);
static int mr_pack_u8(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u8(mr_packet_ctx *pctx, mr_mdata *mdata);

int mr_get_u16(mr_packet_ctx *pctx, const int idx, uint16_t *pu16, bool *pexists);
static int mr_pack_u16(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u16(mr_packet_ctx *pctx, mr_mdata *mdata);

int mr_get_u32(mr_packet_ctx *pctx, const int idx, uint32_t *pu32, bool *pexists);
static int mr_pack_u32(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u32(mr_packet_ctx *pctx, mr_mdata *mdata);

static int mr_count_VBI(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_VBI(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_VBI(mr_packet_ctx *pctx, mr_mdata *mdata);

static int mr_pack_bits_in_value(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_bits(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_incr1(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_bits(mr_packet_ctx *pctx, mr_mdata *mdata);

static int mr_get_vector(mr_packet_ctx *pctx, const int idx, mr_mvalue_t *ppvoid, size_t *plen, bool *pexists);
int mr_set_vector(mr_packet_ctx *pctx, const int idx, const void *pvoid, const size_t len);
int mr_reset_vector(mr_packet_ctx *pctx, const int idx);
static int mr_free_vector(mr_packet_ctx *pctx, mr_mdata *mdata);

int mr_get_u8v(mr_packet_ctx *pctx, const int idx, uint8_t **pu8v0, size_t *plen, bool *pexists);
static int mr_count_u8v(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_u8v(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u8v(mr_packet_ctx *pctx, mr_mdata *mdata);
int mr_validate_u8vutf8(mr_packet_ctx *pctx, const int idx);

int mr_get_str(mr_packet_ctx *pctx, const int idx, char **pcv0, bool *pexists);
static int mr_count_str(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_validate_str(mr_packet_ctx *pctx, mr_mdata *mdata);

int mr_get_spv(mr_packet_ctx *pctx, const int idx, mr_string_pair **pspv0, size_t *plen, bool *pexists);
static int mr_count_spv(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_spv(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_spv(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_validate_spv(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_free_spv(mr_packet_ctx *pctx, mr_mdata *mdata);

static int mr_unpack_properties(mr_packet_ctx *pctx, mr_mdata *mdata);

static int mr_output_scalar(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_output_hexvalue(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_output_hexdump(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_output_string(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_output_spv(mr_packet_ctx *pctx, mr_mdata *mdata);
int mr_printable_mdata(mr_packet_ctx *pctx, const bool all_flag);


#ifdef __cplusplus
}
#endif

#endif // PACKET_INTERNAL_H
