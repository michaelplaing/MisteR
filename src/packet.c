// packet.c

/**
 * @file
 * @brief Common code for handling packets and data types.
 *
 * These functions are called by packet-specific modules to manipulate the packet context
 * (mr_packet_ctx/pctx) and its mr_mdata vector containing the packet values and associated metadata.
 *
 * This module deals with validating, packing and unpacking packets and is focused on handling
 * data types. The ordered field metadata, expressed as the mr_mdata vector and
 * which include data type attributes, are the responsibility of the packet-specific modules and
 * are passed to this module via the packet context.
*/

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <zlog.h>

#include "mister_internal.h"

// typedefs only for this module

typedef int (*mr_mdata_fn)(struct mr_packet_ctx *pctx, struct mr_mdata *mdata);

typedef struct mr_dtype {
    const int idx;
    const char *name;
    const mr_mdata_fn count_fn;
    const mr_mdata_fn pack_fn;
    const mr_mdata_fn unpack_fn;
    const mr_mdata_fn print_fn;
    const mr_mdata_fn validate_fn;
    const mr_mdata_fn free_fn;
} mr_dtype;

typedef int (*mr_ptype_fn)(struct mr_packet_ctx *pctx);

typedef struct mr_ptype {
    const int mqtt_packet_type;
    const char *mqtt_packet_name;
    const mr_ptype_fn ptype_fn;
} mr_ptype;

/**
 * @brief The mr_ptype vector of names and functions.
 *
 * This vector has the same order as mqtt_packet_type.
 *
 * The ptype_fn is invoked at the end of unpacking the packet
 */
static const mr_ptype PACKET_TYPE[] = {
//   mqtt_packet_type   mqtt_packet_name    ptype_fn
    {MQTT_RESERVED,     "RESERVED",         NULL},
    {MQTT_CONNECT,      "CONNECT",          mr_validate_connect_unpack},
    {MQTT_CONNACK,      "CONNACK",          mr_validate_connack_unpack},
    {MQTT_PUBLISH,      "PUBLISH",          mr_validate_publish_unpack},
    {MQTT_PUBACK,       "PUBACK",           NULL},
    {MQTT_PUBREC,       "PUBREC",           NULL},
    {MQTT_PUBREL,       "PUBREL",           NULL},
    {MQTT_PUBCOMP,      "PUBCOMP",          NULL},
    {MQTT_SUBSCRIBE,    "SUBSCRIBE",        NULL},
    {MQTT_SUBACK,       "SUBACK",           NULL},
    {MQTT_UNSUBSCRIBE,  "UNSUBSCRIBE",      NULL},
    {MQTT_UNSUBACK,     "UNSUBACK",         NULL},
    {MQTT_PINGREQ,      "PINGREQ",          NULL},
    {MQTT_PINGRESP,     "PINGRESP",         NULL},
    {MQTT_DISCONNECT,   "DISCONNECT",       NULL},
    {MQTT_AUTH,         "AUTH",             NULL}
};

static const mr_dtype DATA_TYPE[] = { // same order as mr_data_types enum
//   dtype idx              name                        count_fn            pack_fn             unpack_fn               print_fn               validate_fn         free_fn
    {MR_U8_DTYPE,           "uint8",                    NULL,               mr_pack_u8,         mr_unpack_u8,           mr_printable_scalar,   NULL,               NULL},
    {MR_U16_DTYPE,          "uint16",                   NULL,               mr_pack_u16,        mr_unpack_u16,          mr_printable_scalar,   NULL,               NULL},
    {MR_U32_DTYPE,          "uint32",                   NULL,               mr_pack_u32,        mr_unpack_u32,          mr_printable_scalar,   NULL,               NULL},
    {MR_VBI_DTYPE,          "variable byte integer",    mr_count_VBI,       mr_pack_VBI,        mr_unpack_VBI,          mr_printable_scalar,   NULL,               NULL},
    {MR_BITS_DTYPE,         "bits in uint8 bit field",  NULL,               mr_pack_bits,       mr_unpack_bits,         mr_printable_scalar,   NULL,               NULL},
    {MR_U8V_DTYPE,          "binary data - u8 vector",  mr_count_u8v,       mr_pack_u8v,        mr_unpack_u8v,          mr_printable_hexdump,  NULL,               mr_free_vector},
    {MR_PAYLOAD_DTYPE,      "Final U8V - no prefix",    mr_count_payload,   mr_pack_payload,    mr_unpack_payload,      mr_printable_hexdump,  NULL,               mr_free_vector},
    {MR_STR_DTYPE,          "utf8 prefix string",       mr_count_str,       mr_pack_u8v,        mr_unpack_u8v,          mr_printable_string,   mr_validate_str,    mr_free_vector},
    {MR_SPV_DTYPE,          "string pair vector",       mr_count_spv,       mr_pack_spv,        mr_unpack_spv,          mr_printable_spv,      mr_validate_spv,    mr_free_spv},
    {MR_TFV_DTYPE,          "topic filter vector",      mr_count_tfv,       mr_pack_tfv,        mr_unpack_tfv,          mr_printable_tfv,      mr_validate_tfv,    mr_free_tfv},
    {MR_VBIV_DTYPE,         "VBI vector",               mr_count_VBIv,      mr_pack_VBIv,       mr_unpack_VBIv,         mr_printable_VBIv,     mr_validate_VBIv,   mr_free_vector},
    {MR_BITFLD_DTYPE,       "uint8 flag",               NULL,               mr_pack_incr1,      mr_unpack_u8,           mr_printable_hexvalue, NULL,               NULL},
    {MR_PROPERTIES_DTYPE,   "properties",               NULL,               NULL,               mr_unpack_properties,   NULL,                  NULL,               NULL}
};

int mr_init_packet(mr_packet_ctx **ppctx, const mr_mdata *MDATA_TEMPLATE, size_t mdata_count) {
    mr_packet_ctx *pctx;
    if (mr_calloc((void **)&pctx, 1, sizeof(mr_packet_ctx))) return -1;
    pctx->mdata_count = mdata_count;
    mr_mdata *mdata0;
    if (mr_malloc((void **)&mdata0, mdata_count * sizeof(mr_mdata))) return -1;
    memcpy(mdata0, MDATA_TEMPLATE, mdata_count * sizeof(mr_mdata));
    pctx->mdata0 = mdata0;
    pctx->mqtt_packet_type = mdata0->value; // always the value of the 0th mdata row
    pctx->mqtt_packet_name = PACKET_TYPE[pctx->mqtt_packet_type].mqtt_packet_name;
    *ppctx = pctx;
    return 0;
}

static int mr_unpack_packet(mr_packet_ctx *pctx) {
    mr_mdata *mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        if (!mdata->propid) { // properties are handled by the MR_PROPERTIES_DTYPE unpack_fn
            if (mdata->flagid && mdata->dtype != MR_BITS_DTYPE) { // controlling flagid exists
                // printf("\nflagid::packet: %s; name: %s; pctx->flagid: %u\n", pctx->mqtt_packet_name, mdata->name, mdata->flagid);
                mr_mdata *flag_mdata = pctx->mdata0 + mdata->flagid;
                // printf("flagvalue::packet: %s; name: %s; pctx->flagid: %lu\n", pctx->mqtt_packet_name, flag_mdata->name, flag_mdata->value);
                if (flag_mdata->dtype == MR_VBI_DTYPE && pctx->u8vpos >= flag_mdata->value) break; // this value & remaining ones missing;
                if (!flag_mdata->value) continue; // skip if value is not set
            }
            // else {
            //    puts("");
            //}

            // printf("start::packet: %s; name: %s; pctx->u8vpos: %lu\n", pctx->mqtt_packet_name, mdata->name, pctx->u8vpos);
            mr_mdata_fn unpack_fn = DATA_TYPE[mdata->dtype].unpack_fn;
            if (unpack_fn && unpack_fn(pctx, mdata)) return -1;
            mr_mdata_fn validate_fn = DATA_TYPE[mdata->dtype].validate_fn;
            if (validate_fn && validate_fn(pctx, mdata)) return -1;
            // printf("finish::packet: %s; name: %s; pctx->u8vpos: %lu\n", pctx->mqtt_packet_name, mdata->name, pctx->u8vpos);
        }
    }

    mr_ptype_fn ptype_fn = PACKET_TYPE[pctx->mqtt_packet_type].ptype_fn;
    if (ptype_fn && ptype_fn(pctx)) return -1;

    if (pctx->u8vpos == pctx->u8vlen) {
        return 0;
    }
    else if (pctx->u8vpos < pctx->u8vlen) {
        dzlog_error(
            "unparsed bytes in the packet:: packet name: %s; u8vlen: %lu, u8vpos: %lu",
            pctx->mqtt_packet_name, pctx->u8vlen, pctx->u8vpos
        );

        return -1;
    }
    else { // pctx->u8vpos > pctx->u8vlen
        dzlog_error(
            "parsed beyond the packet:: packet name: %s; u8vlen: %lu, u8vpos: %lu",
            pctx->mqtt_packet_name, pctx->u8vlen, pctx->u8vpos
        );

        return -1;
    }
}

int mr_init_unpack_packet(
    mr_packet_ctx **ppctx,
    const mr_mdata *MDATA_TEMPLATE,
    const size_t mdata_count,
    const uint8_t *u8v0,
    const size_t u8vlen
) {
    if (mr_init_packet(ppctx, MDATA_TEMPLATE, mdata_count)) return -1;
    mr_packet_ctx *pctx = *ppctx;
    pctx->u8v0 = (uint8_t *)u8v0; // override const
    pctx->u8vlen = u8vlen;
    pctx->u8valloc = false;
    if (mr_unpack_packet(pctx)) return -1;
    pctx->u8v0 = NULL; // dereference - caller is responsible for freeing
    pctx->u8vlen = 0;
    return 0;
}

int mr_pack_packet(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *pu8vlen) {
    if (pctx->u8valloc && mr_free(pctx->u8v0)) return -1;
    const mr_mdata_fn vbi_count_fn = DATA_TYPE[MR_VBI_DTYPE].count_fn;
    mr_mdata *mdata = pctx->mdata0 + pctx->mdata_count - 1; // last one

    for (int i = pctx->mdata_count - 1; i > -1; mdata--, i--) { // go in reverse to calculate VBIs
        if (mdata->vexists && mdata->dtype != MR_BITS_DTYPE) {
            if (mdata->dtype == MR_VBI_DTYPE && vbi_count_fn(pctx, mdata)) return -1;
            pctx->u8vlen += mdata->u8vlen;
        }
    }

    if (mr_malloc((void **)&pctx->u8v0, pctx->u8vlen)) return -1;
    pctx->u8valloc = true;
    pctx->u8vpos = 0;

    mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        if (mdata->vexists) {
            mr_mdata_fn pack_fn = DATA_TYPE[mdata->dtype].pack_fn;
            if (pack_fn && pack_fn(pctx, mdata)) return -1; // each pack_fn increments pctx->u8vpos
        }
    }

    *pu8v0 = pctx->u8v0;
    *pu8vlen = pctx->u8vlen;
    return 0;
}

int mr_free_packet_context(mr_packet_ctx *pctx) {
    mr_mdata *mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; i++, mdata++) {
        if (DATA_TYPE[mdata->dtype].free_fn && mr_free(mdata->printable)) return -1;
        mr_mdata_fn free_fn = DATA_TYPE[mdata->dtype].free_fn;
        if (mdata->valloc && free_fn && free_fn(pctx, mdata)) return -1;
    }

    if (pctx->u8valloc & mr_free(pctx->u8v0)) return -1;
    if (mr_free(pctx->printable)) return -1;
    if (mr_free(pctx)) return -1;;
    return 0;
}

static int mr_get_scalar(mr_packet_ctx *pctx, const int idx, uintptr_t *pvalue, bool *pexists) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    *pvalue = mdata->value;
    *pexists = mdata->vexists;
    return 0;
}

int mr_set_scalar(mr_packet_ctx *pctx, const int idx, const uintptr_t value) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    if (mr_free(mdata->printable)) return -1;
    mr_mdata_fn validate_fn = DATA_TYPE[mdata->dtype].validate_fn;
    mdata->value = value;
    mdata->vexists = true; // don't update vlen or u8vlen for scalars
    if (validate_fn && validate_fn(pctx, mdata)) return -1;
    if (mdata->dtype == MR_BITS_DTYPE && mr_pack_bits_in_value(pctx, mdata)) return -1;
    return 0;
}

int mr_reset_scalar(mr_packet_ctx *pctx, const int idx) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    if (mr_free(mdata->printable)) return -1;
    mdata->value = 0;
    mdata->vexists = false;
    return 0;
}

int mr_get_boolean(mr_packet_ctx *pctx, const int idx, bool *pflag_value, bool *pexists) {
    uintptr_t value;
    mr_get_scalar(pctx, idx, &value, pexists);
    *pflag_value = (bool)value;
    return 0;
}

int mr_get_u8(mr_packet_ctx *pctx, const int idx, uint8_t *pu8, bool *pexists) {
    uintptr_t value;
    mr_get_scalar(pctx, idx, &value, pexists);
    *pu8 = (uint8_t)value;
    return 0;
}

static int mr_pack_u8(mr_packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t propid = mdata->propid;
    if (propid) pctx->u8v0[pctx->u8vpos++] = propid;
    pctx->u8v0[pctx->u8vpos++] = mdata->value;
    return 0;
}

static int mr_unpack_u8(mr_packet_ctx *pctx, mr_mdata *mdata) {
    mdata->vexists = true;
    mdata->value = pctx->u8v0[pctx->u8vpos++];
    return 0;
}

int mr_get_u16(mr_packet_ctx *pctx, const int idx, uint16_t *pu16, bool *pexists) {
    uintptr_t value;
    mr_get_scalar(pctx, idx, &value, pexists);
    *pu16 = (uint16_t)value;
    return 0;
}

static int mr_pack_u16(mr_packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t propid = mdata->propid;
    if (propid) pctx->u8v0[pctx->u8vpos++] = propid;
    uint16_t u16 = mdata->value;
    pctx->u8v0[pctx->u8vpos++] = (u16 >> 8) & 0xFF;
    pctx->u8v0[pctx->u8vpos++] = u16 & 0xFF;
    return 0;
}

static int mr_unpack_u16(mr_packet_ctx *pctx, mr_mdata *mdata) {
    mdata->vexists = true;
    uint8_t *u8v = pctx->u8v0 + pctx->u8vpos;
    mdata->value = (u8v[0] << 8) + u8v[1];
    pctx->u8vpos += 2;
    return 0;
}

int mr_get_u32(mr_packet_ctx *pctx, const int idx, uint32_t *pu32, bool *pexists) {
    uintptr_t value;
    mr_get_scalar(pctx, idx, &value, pexists);
    *pu32 = (uint32_t)value;
    return 0;
}

static int mr_pack_u32(mr_packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t propid = mdata->propid;
    if (propid) pctx->u8v0[pctx->u8vpos++] = propid;
    uint32_t u32 = mdata->value;
    pctx->u8v0[pctx->u8vpos++] = (u32 >> 24) & 0xFF;
    pctx->u8v0[pctx->u8vpos++] = (u32 >> 16) & 0xFF;
    pctx->u8v0[pctx->u8vpos++] = (u32 >> 8) & 0xFF;
    pctx->u8v0[pctx->u8vpos++] = u32 & 0xFF;
    return 0;
}

static int mr_unpack_u32(mr_packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t *u8v = pctx->u8v0 + pctx->u8vpos;
    mdata->value = (u8v[0] << 24) + (u8v[1] << 16) + (u8v[2] << 8) + u8v[3];
    mdata->vexists = true;
    pctx->u8vpos += 4;
    return 0;
}

static int mr_count_VBI(mr_packet_ctx *pctx, mr_mdata *mdata) {
    size_t cum_len = 0;
    mr_mdata *cnt_mdata;

    //  accumulate u8vlens in cum_len for the range of the VBI
    for (int i = mdata->idx + 1; i <= mdata->link; i++) {
        cnt_mdata = pctx->mdata0 + i;
        if (cnt_mdata->vexists && cnt_mdata->dtype != MR_BITS_DTYPE) cum_len += cnt_mdata->u8vlen;
    }

    mdata->value = cum_len;
    uint32_t u32 = mdata->value;
    int rc = mr_bytecount_VBI(u32);
    if (rc < 0) return rc;
    mdata->vlen = rc;
    mdata->u8vlen = rc;
    return 0;
}

static int mr_pack_VBI(mr_packet_ctx *pctx, mr_mdata *mdata) {
    uint32_t u32 = mdata->value;
    uint8_t u8v[4];
    int rc = mr_make_VBI(u32, u8v);
    if (rc < 0) return rc;
    memcpy(pctx->u8v0 + pctx->u8vpos, u8v, mdata->u8vlen);
    pctx->u8vpos += mdata->u8vlen;
    return 0;
}

static int mr_unpack_VBI(mr_packet_ctx *pctx, mr_mdata *mdata) {
    uint32_t u32;
    int rc = mr_extract_VBI(&u32, pctx->u8v0 + pctx->u8vpos);
    if (rc < 0) return rc;
    mdata->value = u32;
    mdata->vlen = rc;
    mdata->u8vlen = rc;
    mdata->vexists = true;
    pctx->u8vpos += rc;
    return 0;
 }

// index is vlen: # of bits to be (re)set
static const uint8_t BIT_MASKS[] = {
    0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};

static int mr_pack_bits_in_value(mr_packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t bitpos = mdata->u8vlen;
    mr_mdata *flags_mdata = pctx->mdata0 + mdata->link;
    uint8_t flags_u8 = flags_mdata->value;
    flags_u8 &= ~(BIT_MASKS[mdata->vlen] << bitpos); // reset
    if (mdata->value) flags_u8 |= mdata->value << bitpos; // set
    flags_mdata->value = flags_u8;
    return 0;
}

static int mr_pack_bits(mr_packet_ctx *pctx, mr_mdata *mdata) { // don't advance pctx->u8vpos
    uint8_t bitpos = mdata->u8vlen;
    uint8_t *pu8 = pctx->u8v0 + pctx->u8vpos;
    *pu8 &= ~(BIT_MASKS[mdata->vlen] << bitpos); // reset
    if (mdata->value) *pu8 |= mdata->value << bitpos; // set
    return 0;
}

static int mr_pack_incr1(mr_packet_ctx *pctx, mr_mdata *mdata) { // now advance - bits all packed
    pctx->u8vpos++;
    return 0;
}

static int mr_unpack_bits(mr_packet_ctx *pctx, mr_mdata *mdata) {   // don't advance pctx->u8vpos; unpacking
    uint8_t bitpos = mdata->u8vlen;                                 // the following flags byte will do that
    uint8_t *pu8 = pctx->u8v0 + pctx->u8vpos;
    mdata->value = *pu8 >> bitpos & BIT_MASKS[mdata->vlen];
    mdata->vexists = true;
    return 0;
}

static int mr_get_vector(mr_packet_ctx *pctx, const int idx, uintptr_t *ppvoid, size_t *plen, bool *pexists) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    *ppvoid = mdata->value; // for a vector, value is a pointer to something or NULL
    *plen = mdata->vlen;
    *pexists = mdata->vexists;
    return 0;
}

int mr_set_vector(mr_packet_ctx *pctx, const int idx, const void *pvoid, const size_t len) {
    if (mr_reset_vector(pctx, idx)) return -1; // frees printable
    mr_mdata *mdata = pctx->mdata0 + idx;
    mdata->value = (uintptr_t)pvoid;
    mdata->vexists = true;
    mdata->vlen = len;
    mr_mdata_fn count_fn = DATA_TYPE[mdata->dtype].count_fn;
    if (count_fn(pctx, mdata)) return -1; // sets u8vlen
    mr_mdata_fn validate_fn = DATA_TYPE[mdata->dtype].validate_fn;
    if (mdata->value && validate_fn && validate_fn(pctx, mdata)) return -1;
    return 0;
}

int mr_reset_vector(mr_packet_ctx *pctx, const int idx) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    if (mr_free(mdata->printable)) return -1;
    mr_mdata_fn free_fn = DATA_TYPE[mdata->dtype].free_fn;
    return free_fn(pctx, mdata);
}

static int mr_free_vector(mr_packet_ctx *pctx, mr_mdata *mdata) {
    if (mdata->valloc && mr_free((void *)mdata->value)) return -1;
    mdata->value = (uintptr_t)NULL;
    mdata->valloc = false;
    mdata->vexists = false;
    mdata->vlen = 0;
    mdata->u8vlen = 0;
    return 0;
}

int mr_get_u8v(mr_packet_ctx *pctx, const int idx, uint8_t **pu8v0, size_t *plen, bool *pexists) {
    uintptr_t pvoid;
    mr_get_vector(pctx, idx, &pvoid, plen, pexists);
    *pu8v0 = (uint8_t *)pvoid;
    return 0;
}

static int mr_count_u8v(mr_packet_ctx *pctx, mr_mdata *mdata) {
    mdata->u8vlen = 2 + mdata->vlen;
    if (mdata->propid) mdata->u8vlen++;
    return 0;
}

static int mr_count_payload(mr_packet_ctx *pctx, mr_mdata *mdata) {
    mdata->u8vlen = mdata->vlen;
    return 0;
}

static int mr_pack_u8v(mr_packet_ctx *pctx, mr_mdata *mdata) {
    bool str_flag = mdata->dtype == MR_STR_DTYPE;
    uint8_t propid = mdata->propid;
    if (propid) pctx->u8v0[pctx->u8vpos++] = propid;
    uint16_t u16 = mdata->vlen - (str_flag ? 1 : 0);
    pctx->u8v0[pctx->u8vpos++] = (u16 >> 8) & 0xFF;
    pctx->u8v0[pctx->u8vpos++] = u16 & 0xFF;
    memcpy(pctx->u8v0 + pctx->u8vpos, (uint8_t *)mdata->value, u16);
    pctx->u8vpos += u16;
    return 0;
}

static int mr_pack_payload(mr_packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->vlen) return 0; // MQTT: 0-length payload is OK
    memcpy(pctx->u8v0 + pctx->u8vpos, (uint8_t *)mdata->value, mdata->vlen);
    pctx->u8vpos += mdata->vlen;
    return 0;
}

static int mr_unpack_u8v(mr_packet_ctx *pctx, mr_mdata *mdata) {
    bool str_flag = mdata->dtype == MR_STR_DTYPE;
    uint8_t *u8v = pctx->u8v0 + pctx->u8vpos;
    size_t u8vlen = (u8v[0] << 8) + u8v[1];
    u8v += 2;
    size_t vlen = u8vlen + (str_flag ? 1 : 0);
    uint8_t *value;
    if (mr_calloc((void **)&value, vlen, 1)) return -1;
    memcpy(value, u8v, u8vlen);
    mdata->value = (uintptr_t)value;
    mdata->vlen = vlen;
    mdata->vexists = true;
    mdata->valloc = true;
    mdata->u8vlen = (mdata->propid ? 1 : 0) + 2 + u8vlen;
    pctx->u8vpos += 2 + u8vlen;
    return 0;
}

static int mr_unpack_payload(mr_packet_ctx *pctx, mr_mdata *mdata) {
    mdata->value = (uintptr_t)(pctx->u8v0 + pctx->u8vpos);
    mdata->vexists = true;
    mdata->valloc = false;
    // printf("\npayload:: pctx->u8vlen: %lu; pctx->u8vpos: %lu\n\n", pctx->u8vlen, pctx->u8vpos);
    mdata->u8vlen = mdata->vlen = pctx->u8vlen - pctx->u8vpos;
    pctx->u8vpos += mdata->u8vlen;
    return 0;
}

int mr_validate_u8v_utf8(mr_packet_ctx *pctx, const int idx) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    int err_pos = mr_utf8_validation((uint8_t *)mdata->value, mdata->vlen);

    if (err_pos) {
        dzlog_error(
            "invalid utf8:: packet: %s; name: %s; pos: %d",
            pctx->mqtt_packet_name, mdata->name, err_pos
        );

        return -1;
    }

    return 0;
}

int mr_get_VBIv(mr_packet_ctx *pctx, const int idx, uint32_t **pu32v0, size_t *plen, bool *pexists) {
    uintptr_t pvoid;
    mr_get_vector(pctx, idx, &pvoid, plen, pexists);
    *pu32v0 = (uint32_t *)pvoid;
    return 0;
}

static int mr_count_VBIv(mr_packet_ctx *pctx, mr_mdata *mdata) { // VBIv's are properties
    if (!mdata->value) {
        dzlog_error("NULL pointer: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
        return -1;
    }

    uint32_t *VBIv0 = (uint32_t *)mdata->value;

    mdata->u8vlen = 0;
    for (int i = 0; i < mdata->vlen; i++) { // propid(u8) + VBI byte count
        int bytecount = mr_bytecount_VBI(VBIv0[i]);

        if (bytecount < 0) {
            dzlog_error("VBI too big for 4 bytes: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
            return -1;
        }

        mdata->u8vlen += 1 + bytecount;
    }

    return 0;
}

static int mr_pack_VBIv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->value) {
        dzlog_error("NULL pointer: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
        return -1;
    }

    uint32_t *VBIv0 = (uint32_t *)mdata->value;

    for (int i = 0; i < mdata->vlen; i++) {
        pctx->u8v0[pctx->u8vpos++] = mdata->propid;
        int bytecount = mr_make_VBI(VBIv0[i], &pctx->u8v0[pctx->u8vpos]);

        if (bytecount < 0) {
            dzlog_error("VBI too big for 4 bytes: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
            return -1;
        }

        pctx->u8vpos += bytecount;
    }

    return 0;
}

static int mr_unpack_VBIv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t *pu8 = pctx->u8v0 + pctx->u8vpos;
    uint32_t u32;
    int bytecount = mr_extract_VBI(&u32, pu8);

    if (bytecount < 0) {
        dzlog_error("VBI too big for 4 bytes: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
        return -1;
    }

    uint32_t *VBIv0;
    if (mdata->valloc) {
        VBIv0 = (uint32_t *)mdata->value;
        if (mr_realloc((void **)&VBIv0, (mdata->vlen + 1) * sizeof(uint32_t))) return -1;
    }
    else {
        if (mr_malloc((void **)&VBIv0, sizeof(uint32_t))) return -1;
        mdata->vlen = 0; // incremented below
    }

    mdata->value = (uintptr_t)VBIv0;
    *(VBIv0 + mdata->vlen) = u32;
    mdata->vlen++;
    mdata->vexists = true;
    mdata->valloc = true;
    mdata->u8vlen += 1 + bytecount;
    pctx->u8vpos += bytecount; // propid has already been consumed
    return 0;
}

static int mr_validate_VBIv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->value) {
        dzlog_error("NULL pointer: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
        return -1;
    }

    uint32_t *VBIv0 = (uint32_t *)mdata->value;

    for (int i = 0; i < mdata->vlen; i++) {
        int bytecount = mr_bytecount_VBI(VBIv0[i]);

        if (bytecount < 0) {
            dzlog_error("VBI too big for 4 bytes: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
            return -1;
        }
    }

    return 0;
}

int mr_get_str(mr_packet_ctx *pctx, const int idx, char **pcv0, bool *pexists) {
    uintptr_t pvoid;
    size_t len;
    mr_get_vector(pctx, idx, &pvoid, &len, pexists);
    *pcv0 = (char *)pvoid;
    return 0;
}

static int mr_count_str(mr_packet_ctx *pctx, mr_mdata *mdata) {
    if (mr_count_u8v(pctx, mdata)) return -1;
    mdata->u8vlen--; // correct for vlen = strlen() + 1
    return 0;
}

static int mr_validate_str(mr_packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->value) {
        dzlog_error("NULL pointer: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
        return -1;
    }

    char *pc = (char *)mdata->value;
    int err_pos = mr_utf8_validation((uint8_t *)pc, strlen(pc)); // returns error position

    if (err_pos) {
        dzlog_error(
            "invalid utf8:: packet: %s; name: %s; value: %s, pos: %d",
            pctx->mqtt_packet_name, mdata->name, pc, err_pos
        );

        return -1;
    }

    return 0;
}

int mr_get_spv(mr_packet_ctx *pctx, const int idx, mr_string_pair **pspv0, size_t *plen, bool *pexists) {
    uintptr_t pvoid;
    mr_get_vector(pctx, idx, &pvoid, plen, pexists);
    *pspv0 = (mr_string_pair *)pvoid;
    return 0;
}

static int mr_count_spv(mr_packet_ctx *pctx, mr_mdata *mdata) { // spv's are properties
    if (!mdata->value) {
        dzlog_error("NULL pointer: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
        return -1;
    }

    mr_string_pair *spv = (mr_string_pair *)mdata->value;

    mdata->u8vlen = 0;
    for (int i = 0; i < mdata->vlen; i++) { // propid(u8) + u16 + nlen + u16 + vlen
        mdata->u8vlen += 1 + 2 + strlen(spv[i].name) + 2 + strlen(spv[i].value);
    }

    return 0;
}

static int mr_pack_spv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->value) {
        dzlog_error("NULL pointer: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
        return -1;
    }

    mr_string_pair *spv = (mr_string_pair *)mdata->value;
    uint16_t u16;

    for (int i = 0; i < mdata->vlen; i++) {
        pctx->u8v0[pctx->u8vpos++] = mdata->propid;
        // name
        u16 = strlen(spv[i].name);
        pctx->u8v0[pctx->u8vpos++] = (u16 >> 8) & 0xFF;
        pctx->u8v0[pctx->u8vpos++] = u16 & 0xFF;
        memcpy(pctx->u8v0 + pctx->u8vpos, spv[i].name, u16);
        pctx->u8vpos += u16;
        // value
        u16 = strlen(spv[i].value);
        pctx->u8v0[pctx->u8vpos++] = (u16 >> 8) & 0xFF;
        pctx->u8v0[pctx->u8vpos++] = u16 & 0xFF;
        memcpy(pctx->u8v0 + pctx->u8vpos, spv[i].value, u16);
        pctx->u8vpos += u16;
    }

    return 0;
}

static int mr_unpack_spv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t *u8v = pctx->u8v0 + pctx->u8vpos;

    size_t namelen = (u8v[0] << 8) + u8v[1];
    u8v += 2;
    char *name;
    if (mr_malloc((void **)&name, namelen + 1)) return -1;
    memcpy(name, u8v, namelen);
    u8v += namelen;
    name[namelen] = '\0';

    size_t valuelen = (u8v[0] << 8) + u8v[1];
    u8v += 2;
    char *value;
    if (mr_malloc((void **)&value, valuelen + 1)) return -1;
    memcpy(value, u8v, valuelen);
    u8v += valuelen;
    value[valuelen] = '\0';

    mr_string_pair *spv0, *psp;
    if (mdata->valloc) {
        spv0 = (mr_string_pair *)mdata->value;
        if (mr_realloc((void **)&spv0, (mdata->vlen + 1) * sizeof(mr_string_pair))) return -1;
    }
    else {
        if (mr_malloc((void **)&spv0, sizeof(mr_string_pair))) return -1;
        mdata->vlen = 0; // incremented below
    }

    mdata->value = (uintptr_t)spv0;
    psp = spv0 + mdata->vlen;
    psp->name = name;
    psp->value = value;
    mdata->vlen++;
    mdata->vexists = true;
    mdata->valloc = true;
    mdata->u8vlen += 1 + 2 + namelen + 2 + valuelen;
    pctx->u8vpos += 2 + namelen + 2 + valuelen; // propid has already been consumed
    return 0;
}

static int mr_validate_spv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->value) {
        dzlog_error("NULL pointer: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
        return -1;
    }

    mr_string_pair *spv = (mr_string_pair *)mdata->value;

    for (int i = 0; i < mdata->vlen; i++) { // mr_utf8_validation returns error position
        int err_pos = mr_utf8_validation((uint8_t *)spv[i].name, strlen(spv[i].name));

        if (err_pos) {
            dzlog_error(
                "invalid utf8: packet: %s; mr_string_pair: %d; name: %s; pos: %d",
                pctx->mqtt_packet_name, i, spv[i].name, err_pos
            );

            return -1;
        }

        err_pos = mr_utf8_validation((uint8_t *)spv[i].value, strlen(spv[i].value));

        if (err_pos) {
            dzlog_error(
                "invalid utf8: packet: %s; mr_string_pair: %d; value: %s; pos: %d",
                pctx->mqtt_packet_name, i, spv[i].value, err_pos
            );

            return -1;
        }
    }

    return 0;
}

static int mr_free_spv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    mr_string_pair *spv0 = (mr_string_pair *)mdata->value;

    if (mdata->valloc) {
        mr_string_pair *psp = spv0;
        for (int i = 0; i < mdata->vlen; psp++, i++) {
            if (mr_free(psp->name)) return -1;
            if (mr_free(psp->value)) return -1;
        }
    }

    return mr_free_vector(pctx, mdata);
}

int mr_get_tfv(mr_packet_ctx *pctx, const int idx, mr_topic_filter **ptfv0, size_t *plen, bool *pexists) {
    uintptr_t pvoid;
    mr_get_vector(pctx, idx, &pvoid, plen, pexists);
    *ptfv0 = (mr_topic_filter *)pvoid;
    return 0;
}

static int mr_count_tfv(mr_packet_ctx *pctx, mr_mdata *mdata) { // tfv's are in the payload not properties
    if (!mdata->value) {
        dzlog_error("NULL pointer: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
        return -1;
    }

    mr_topic_filter *tfv = (mr_topic_filter *)mdata->value;

    mdata->u8vlen = 0;
    for (int i = 0; i < mdata->vlen; i++) { // u16 + tflen + u8 (options)
        mdata->u8vlen += 2 + strlen(tfv[i].topic_filter) + 1;
    }

    return 0;
}

static int mr_pack_tfv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->value) {
        dzlog_error("NULL pointer: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
        return -1;
    }

    mr_topic_filter *tfv = (mr_topic_filter *)mdata->value;

    for (int i = 0; i < mdata->vlen; i++) {
        // topic filter
        uint16_t u16 = strlen(tfv[i].topic_filter);
        pctx->u8v0[pctx->u8vpos++] = (u16 >> 8) & 0xFF;
        pctx->u8v0[pctx->u8vpos++] = u16 & 0xFF;
        memcpy(pctx->u8v0 + pctx->u8vpos, tfv[i].topic_filter, u16);
        pctx->u8vpos += u16;

        // options
        uint8_t u8 = 0;
        u8 |= tfv[i].maximum_qos;
        u8 |= tfv[i].no_local << 2;
        u8 |= tfv[i].retain_as_published << 3;
        u8 |= tfv[i].retain_handling << 4;
        pctx->u8v0[pctx->u8vpos++] = u8;
    }

    return 0;
}

static int mr_unpack_tfv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t *u8v = pctx->u8v0 + pctx->u8vpos;

    size_t tflen = (u8v[0] << 8) + u8v[1];
    u8v += 2;

    char *topic_filter;
    if (mr_malloc((void **)&topic_filter, tflen + 1)) return -1;
    memcpy(topic_filter, u8v, tflen);
    topic_filter[tflen] = '\0';
    u8v += tflen;

    uint8_t maximum_qos = *u8v & BIT_MASKS[2];
    uint8_t no_local = *u8v >> 2 & BIT_MASKS[1];
    uint8_t retain_as_published = *u8v >> 3 & BIT_MASKS[1];
    uint8_t retain_handling = *u8v >> 4 & BIT_MASKS[2];
    u8v++;

    mr_topic_filter *tfv0, *ptf;
    if (mdata->valloc) {
        tfv0 = (mr_topic_filter *)mdata->value;
        if (mr_realloc((void **)&tfv0, (mdata->vlen + 1) * sizeof(mr_topic_filter))) return -1;
    }
    else {
        if (mr_malloc((void **)&tfv0, sizeof(mr_topic_filter))) return -1;
        mdata->vlen = 0; // incremented below
    }

    mdata->value = (uintptr_t)tfv0;
    ptf = tfv0 + mdata->vlen;
    ptf->topic_filter = topic_filter;
    ptf->maximum_qos = maximum_qos;
    ptf->no_local = no_local;
    ptf->retain_as_published = retain_as_published;
    ptf->retain_handling = retain_handling;

    mdata->vlen++;
    mdata->vexists = true;
    mdata->valloc = true;
    mdata->u8vlen += 2 + tflen + 1;
    pctx->u8vpos += 2 + tflen + 1;
    return 0;
}

static int mr_validate_tfv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->value) {
        dzlog_error("NULL pointer: packet: %s; name: %s", pctx->mqtt_packet_name, mdata->name);
        return -1;
    }

    mr_topic_filter *tfv = (mr_topic_filter *)mdata->value;

    for (int i = 0; i < mdata->vlen; i++) { // mr_utf8_validation returns error position
        int err_pos = mr_utf8_validation((uint8_t *)tfv[i].topic_filter, strlen(tfv[i].topic_filter));

        if (err_pos) {
            dzlog_error(
                "invalid utf8: packet: %s; mr_topic_filter: %d; topic_filter: %s; pos: %d",
                pctx->mqtt_packet_name, i, tfv[i].topic_filter, err_pos
            );

            return -1;
        }

        if (tfv[i].maximum_qos > 2) {
            dzlog_error(
                "maximum_qos out of range (0..2): packet: %s; mr_topic_filter: %d; maximum_qos: %u",
                pctx->mqtt_packet_name, i, tfv[i].maximum_qos
            );

            return -1;
        }

        if (tfv[i].retain_handling > 2) {
            dzlog_error(
                "retain_handling out of range (0..2): packet: %s; mr_topic_filter: %d; retain_handling: %u",
                pctx->mqtt_packet_name, i, tfv[i].retain_handling
            );

            return -1;
        }
    }

    return 0;
}

static int mr_free_tfv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    if (mdata->valloc) {
        mr_topic_filter *ptf = (mr_topic_filter *)mdata->value;
        for (int i = 0; i < mdata->vlen; ptf++, i++) {
            if (mr_free(ptf->topic_filter)) return -1;
        }
    }

    return mr_free_vector(pctx, mdata);
}

static int mr_unpack_properties(mr_packet_ctx *pctx, mr_mdata *mdata) {
    mdata->vexists = true;
    size_t end_pos = pctx->u8vpos + (mdata - 1)->value; // use property_length
    uint8_t *pu8, *pprop_index;
    int prop_index;
    mr_mdata *prop_mdata;
    mr_mdata_fn unpack_fn;
    mr_mdata_fn validate_fn;

    for (; pctx->u8vpos < end_pos;) {
        pu8 = pctx->u8v0 + pctx->u8vpos++;
        pprop_index = memchr((uint8_t *)mdata->value, *pu8, mdata->vlen);

        if (!pprop_index) {
            dzlog_error(
                "property id not found:: packet: %s; name: %s; propid: %d",
                pctx->mqtt_packet_name, mdata->name, *pu8
            );

            return -1;
        }

        prop_index = pprop_index - (uint8_t *)mdata->value;
        prop_mdata = mdata + prop_index + 1;

        if (
            prop_mdata->vexists &&
            prop_mdata->dtype != MR_SPV_DTYPE &&
            prop_mdata->dtype != MR_VBIV_DTYPE
        ) {
            dzlog_error(
                "duplicate property value:: packet: %s; name: %s",
                pctx->mqtt_packet_name, prop_mdata->name
            );

            return -1;
        }

        unpack_fn = DATA_TYPE[prop_mdata->dtype].unpack_fn;
        if (unpack_fn(pctx, prop_mdata)) return -1;
        validate_fn = DATA_TYPE[mdata->dtype].validate_fn;
        if (validate_fn && validate_fn(pctx, mdata)) return -1;
    }

    return 0;
}

static int mr_printable_scalar(mr_packet_ctx *pctx, mr_mdata *mdata) {
    char cv[32] = {'\0'};
    char *printable;
    sprintf(cv, "%u", (uint32_t)mdata->value);
    if (mr_calloc((void **)&printable, strlen(cv) + 1, 1)) return -1;
    strlcpy(printable, cv, 32);
    mdata->printable = printable;
    return 0;
}

static int mr_printable_hexvalue(mr_packet_ctx *pctx, mr_mdata *mdata) {
    char cv[100] = {'\0'};
    uint8_t u8 = mdata->value;
    if (mr_get_hexdump(cv, sizeof(cv), &u8, 1)) return -1;
    mr_compress_spaces_lines(cv);
    size_t slen = strlen(cv);
    char *printable;
    if (mr_calloc((void **)&printable, slen + 1, 1)) return -1;
    strlcpy(printable, cv, slen + 1);
    mdata->printable = printable;
    return 0;
}

static int mr_printable_hexdump(mr_packet_ctx *pctx, mr_mdata *mdata) {
    char cv[200] = {'\0'};
    size_t len = mdata->vlen > 32 ? 32 : mdata->vlen; // limit to 32 bytes

    if (len) {
        if (mr_get_hexdump(cv, sizeof(cv), (uint8_t *)mdata->value, len)) return -1;
        mr_compress_spaces_lines(cv); // make into a single line
    }

    size_t slen = strlen(cv);
    char *printable;
    if (mr_calloc((void **)&printable, slen + 1, 1)) return -1;
    strlcpy(printable, cv, slen + 1);
    mdata->printable = printable;
    return 0;
}

static int mr_printable_string(mr_packet_ctx *pctx, mr_mdata *mdata) {
    char *printable;
    size_t slen = strlen((char *)mdata->value);
    if (mr_calloc((void **)&printable, slen + 1, 1)) return -1;
    strlcpy(printable, (char *)mdata->value, slen + 1);
    mdata->printable = printable;
    return 0;
}

static int mr_printable_spv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    char *printable;
    size_t sz = 0;
    mr_string_pair *spv = (mr_string_pair *)mdata->value;

    for (int i = 0; i < mdata->vlen; i++) {
        sz += strlen(spv[i].name) + 1 + strlen(spv[i].value) + 1; // ':' and ';'
    }

    if (mr_calloc((void **)&printable, sz, 1)) return -1;

    char *pc = printable;
    for (int i = 0; i < mdata->vlen; i++) {
        sprintf(pc, "%s:%s;", spv[i].name, spv[i].value);
        pc += strlen(spv[i].name) + 1 + strlen(spv[i].value) + 1; // ditto
    }

    *(pc - 1) = '\0'; // overwrite trailing ';'
    mdata->printable = printable;
    return 0;
}

static int mr_printable_tfv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    char *printable;
    size_t sz = 0;
    mr_topic_filter *tfv = (mr_topic_filter *)mdata->value;

    for (int i = 0; i < mdata->vlen; i++) {
        sz += strlen(tfv[i].topic_filter) + 9; // ':' and 'x x x x;'
    }

    if (mr_calloc((void **)&printable, sz, 1)) return -1;

    char *pc = printable;
    for (int i = 0; i < mdata->vlen; i++) {
        sprintf(
            pc, "%s:%u %u %u %u;",
            tfv[i].topic_filter,
            tfv[i].maximum_qos,
            tfv[i].no_local,
            tfv[i].retain_as_published,
            tfv[i].retain_handling
        );

        pc += strlen(tfv[i].topic_filter) + 9; // ditto
    }

    *(pc - 1) = '\0'; // overwrite trailing ';'
    mdata->printable = printable;

    return 0;
}

static int mr_printable_VBIv(mr_packet_ctx *pctx, mr_mdata *mdata) {
    char *printable;
    uint32_t *VBIv0 = (uint32_t *)mdata->value;

    char cv[16];
    size_t len = 0;
    for (int i = 0; i < mdata->vlen; i++) {
        len += sprintf(cv, "%u;", VBIv0[i]);
    }

    if (mr_calloc((void **)&printable, len + 1, 1)) return -1;

    char *pc = printable;
    for (int i = 0; i < mdata->vlen; i++) {
        pc += sprintf(pc, "%u;", VBIv0[i]);
    }

    *(pc - 1) = '\0'; // overwrite trailing ';'
    mdata->printable = printable;
    return 0;
}

static const char NOT_PRINTABLE[] = "***";

/**
 * @brief Create the packet's printable metadata by creating and catenating the packet's mdata printable's.
 *
 * @details
 * Free the currently allocated packet printable and mdata printable's as we go.
 *
 * Invoke the mr_dtype::print_fn for each mr_mdata in the mr_packet_ctx::mdata0 vector. The print_fn's
 * are one of these static functions: mr_printable_scalar for an integer; mr_printable_hexdump for a u8 vector;
 * mr_printable_hexvalue for an integer bit field, typically a flag byte; mr_printable_string for a c-string,
 * and mr_printable_spv for a mr_string_pair vector.
 *
 * Catenate the mr_mdata::name's and the mr_mdata::printable's into the
 * mr_packet_ctx::printable_mdata c-string.
 *
 * @param all_flag true: list name:value pairs for all fields using '***' for the values of non-existent ones;
 * false: only list name:value pairs for fields that exist.
 * @param pcv the address of a c-string that will be the printable metadata.
 */
int mr_get_printable(mr_packet_ctx *pctx, const bool all_flag, char **pcv) {
    if (mr_free(pctx->printable)) return -1;
    const mr_mdata_fn vbi_count_fn = DATA_TYPE[MR_VBI_DTYPE].count_fn;

    // traverse in reverse to calculate VBIs since their u8vlens are variable
    mr_mdata *mdata = pctx->mdata0 + pctx->mdata_count - 1; // last one
    for (int i = pctx->mdata_count - 1; i > -1; mdata--, i--) {
        if (mdata->vexists && mdata->dtype == MR_VBI_DTYPE && vbi_count_fn(pctx, mdata)) return -1;
    }

    size_t len = 0;
    mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        // printf("packet: %s; field: %s\n", pctx->mqtt_packet_name, mdata->name);
        mr_mdata_fn print_fn = DATA_TYPE[mdata->dtype].print_fn;
        if (print_fn && mr_free(mdata->printable)) return -1;

        if (mdata->vexists && print_fn) {
            if (print_fn(pctx, mdata)) return -1;
            len += strlen(mdata->name) + 1 + strlen(mdata->printable) + 1; // ':' and '\n'
        }
    }

    char *printable;
    if (mr_calloc((void **)&printable, len, 1)) return -1;

    char *pc = printable;
    mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        if (mdata->printable) {
            sprintf(pc, "%s:%s\n", mdata->name, mdata->printable);
            pc += strlen(mdata->name) + 1 + strlen(mdata->printable) + 1; // ditto
        }
        else if (all_flag) {
            sprintf(pc, "%s:%s\n", mdata->name, NOT_PRINTABLE);
            pc += strlen(mdata->name) + 1 + strlen(NOT_PRINTABLE) + 1; // ditto
        }
        else {
            ; //noop
        }
    }

    *(pc - 1) = '\0'; // overwrite trailing '\n' to make a c-string
    *pcv = pctx->printable = printable;
    return 0;
}
