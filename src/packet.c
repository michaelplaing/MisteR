/* packet.c */

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mister/mister.h"
#include "mister/mrzlog.h"
#include "util_internal.h"
#include "packet_internal.h"
#include "connect_will_internal.h"

static char *_PTYPE_NAME[] = { // same order as mqtt_packet_type
    "", // Note: enum high nibbles are 1-based, hence dummy str so we can use a 1-based index
    "CONNECT",
    "CONNACK",
    "PUBLISH",
    "PUBACK",
    "PUBREC",
    "PUBREL",
    "PUBCOMP",
    "SUBSCRIBE",
    "SUBACK",
    "UNSUBSCRIBE",
    "UNSUBACK",
    "PINGREQ",
    "PINGRESP",
    "DISCONNECT",
    "AUTH"
};

static const mr_dtype _DTYPE[] = { // same order as mr_dtypes enum
//   dtype idx          name                        count_fn        pack_fn             unpack_fn           output_fn           validate_fn         free_fn
    {MR_U8_DTYPE,       "uint8",                    NULL,           mr_pack_u8,         mr_unpack_u8,       mr_output_scalar,   NULL,               NULL},
    {MR_U16_DTYPE,      "uint16",                   NULL,           mr_pack_u16,        mr_unpack_u16,      mr_output_scalar,   NULL,               NULL},
    {MR_U32_DTYPE,      "uint32",                   NULL,           mr_pack_u32,        mr_unpack_u32,      mr_output_scalar,   NULL,               NULL},
    {MR_VBI_DTYPE,      "variable byte int",        mr_count_VBI,   mr_pack_VBI,        mr_unpack_VBI,      mr_output_scalar,   NULL,               NULL},
    {MR_BITS_DTYPE,     "bits in uint8 flag",       NULL,           mr_pack_bits,       mr_unpack_bits,     mr_output_scalar,   NULL,               NULL},
    {MR_U8V_DTYPE,      "binary data - uint8 vec",  mr_count_u8v,   mr_pack_u8v,        mr_unpack_u8v,      mr_output_hexdump,  NULL,               mr_free_vector},
    {MR_STR_DTYPE,      "utf8 prefix string",       mr_count_str,   mr_pack_str,        mr_unpack_str,      mr_output_string,   mr_validate_str,    mr_free_vector},
    {MR_SPV_DTYPE,      "string pair vector",       mr_count_spv,   mr_pack_spv,        mr_unpack_spv,      mr_output_spv,      mr_validate_spv,    mr_free_spv},
    {MR_FLAGS_DTYPE,    "uint8 flag",               NULL,           mr_pack_incr1,      mr_unpack_u8,       mr_output_hdvalue,  NULL,               NULL},
    {MR_PROPS_DTYPE,    "properties",               NULL,           NULL,               mr_unpack_props,    NULL,               NULL,               NULL}
};

static int mr_output_scalar(packet_ctx *pctx, mr_mdata *mdata) {
    char *ovalue;
    if (!asprintf(&ovalue, "%u", (uint32_t)mdata->value)) return -1;
    mdata->ovalloc = true;
    mdata->ovalue = ovalue;
    return 0;
}

#define CVSZ 200
static int mr_output_hexdump(packet_ctx *pctx, mr_mdata *mdata) {
    char cv[CVSZ];
    size_t len = mdata->vlen > 32 ? 32 : mdata->vlen; // limit to 32 bytes
    if (mr_get_hexdump(cv, sizeof(cv), (uint8_t *)mdata->value, len)) return -1;
    mr_compress_spaces_lines(cv); // make into a single line
    char *ovalue;
    if (mr_malloc((void **)&ovalue, strlen(cv) + 1)) return -1;
    strlcpy(ovalue, cv, CVSZ);
    mdata->ovalue = ovalue;
    mdata->ovalloc = true;
    return 0;
}

static int mr_output_hdvalue(packet_ctx *pctx, mr_mdata *mdata) {
    char cv[CVSZ];
    uint8_t u8v[4];
    uint32_t u32 = mdata->value;
    u8v[0] = (u32 >> 24) & 0xFF;
    u8v[1] = (u32 >> 16) & 0xFF;
    u8v[2] = (u32 >> 8) & 0xFF;
    u8v[3] = u32 & 0xFF;
    if (mr_get_hexdump(cv, sizeof(cv), u8v, 4)) return -1;
    mr_compress_spaces_lines(cv); // make into a single line
    char *ovalue;
    if (mr_malloc((void **)&ovalue, strlen(cv) + 1)) return -1;
    strlcpy(ovalue, cv, CVSZ);
    mdata->ovalue = ovalue;
    mdata->ovalloc = true;
    return 0;
}

static int mr_output_string(packet_ctx *pctx, mr_mdata *mdata) {
    char *ovalue;
    size_t slen = strlen((char *)mdata->value);
    if (mr_malloc((void **)&ovalue, slen + 1)) return -1;
    strlcpy(ovalue, (char *)mdata->value, mdata->vlen);
    ovalue[slen] = '\0';
    mdata->ovalue = ovalue;
    mdata->ovalloc = true;
    return 0;
}

static int mr_output_spv(packet_ctx *pctx, mr_mdata *mdata) {
    char *ovalue;
    size_t sz = 0;
    string_pair *spv = (string_pair *)mdata->value;

    for (int i = 0; i < mdata->vlen; i++) {
        sz += strlen(spv[i].name) + 1 + strlen(spv[i].value) + 1; // ':' and ';'
    }

    if (mr_malloc((void **)&ovalue, sz)) return -1;

    char *pc = ovalue;
    for (int i = 0; i < mdata->vlen; i++) {
        sprintf(pc, "%s:%s;", spv[i].name, spv[i].value);
        pc += strlen(spv[i].name) + 1 + strlen(spv[i].value) + 1; // ditto
    }

    *(pc - 1) = '\0'; // overwrite trailing ';'
    mdata->ovalue = ovalue;
    mdata->ovalloc = true;
    return 0;
}

int mr_mdata_dump(packet_ctx *pctx) {
    const mr_mdata_fn vbi_count_fn = _DTYPE[MR_VBI_DTYPE].count_fn;

    // traverse in reverse to calculate VBIs since they have variable size
    mr_mdata *mdata = pctx->mdata0 + pctx->mdata_count - 1; // last one

    for (int i = pctx->mdata_count - 1; i > -1; mdata--, i--) {
        if (mdata->vexists && mdata->dtype == MR_VBI_DTYPE && vbi_count_fn(pctx, mdata)) return -1;
    }

    size_t len = 0;
    mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        if (mdata->vexists) {
            mr_mdata_fn output_fn = _DTYPE[mdata->dtype].output_fn;

            if (output_fn) {
                if (output_fn(pctx, mdata)) return -1;
                len += strlen(mdata->name) + 1 + strlen(mdata->ovalue) + 1; // ':' and '\n'
            }
        }
    }

    char *mdata_dump;
    if (mr_malloc((void **)&mdata_dump, len)) return -1;

    char *pc = mdata_dump;
    mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        if (mdata->ovalue) {
            sprintf(pc, "%s:%s\n", mdata->name, mdata->ovalue);
            pc += strlen(mdata->name) + 1 + strlen(mdata->ovalue) + 1; // ditto
        }
    }

    *(pc - 1) = '\0'; // overwrite trailing '\n'
    pctx->mdata_dump = mdata_dump;
    return 0;
}

int mr_validate_u8vutf8(packet_ctx *pctx, int idx) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    int err_pos = utf8val((uint8_t *)mdata->value, mdata->vlen); // returns error position

    if (err_pos) {
        dzlog_error(
            "invalid utf8:: packet: %s; name: %s; pos: %d",
            pctx->mqtt_packet_name, mdata->name, err_pos
        );

        return -1;
    }

    return 0;
}

int mr_set_scalar(packet_ctx *pctx, int idx, mvalue_t value) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    mr_mdata_fn validate_fn = _DTYPE[mdata->dtype].validate_fn;
    mdata->value = value;
    mdata->vexists = true; // don't update vlen or u8vlen for scalars
    if (validate_fn && validate_fn(pctx, mdata)) return -1;
    if (mdata->dtype == MR_BITS_DTYPE && mr_pack_bits_in_value(pctx, mdata)) return -1;
    return 0;
}

static int mr_get_scalar(packet_ctx *pctx, int idx, mvalue_t *pvalue, bool *pexists) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    *pvalue = mdata->value;
    *pexists = mdata->vexists;
    return 0;
}

int mr_get_boolean(packet_ctx *pctx, int idx, bool *pboolean, bool *pexists) {
    mvalue_t value;
    mr_get_scalar(pctx, idx, &value, pexists);
    *pboolean = (bool)value;
    return 0;
}

int mr_get_u8(packet_ctx *pctx, int idx, uint8_t *pu8, bool *pexists) {
    mvalue_t value;
    mr_get_scalar(pctx, idx, &value, pexists);
    *pu8 = (uint8_t)value;
    return 0;
}

int mr_get_u16(packet_ctx *pctx, int idx, uint16_t *pu16, bool *pexists) {
    mvalue_t value;
    mr_get_scalar(pctx, idx, &value, pexists);
    *pu16 = (uint16_t)value;
    return 0;
}

int mr_get_u32(packet_ctx *pctx, int idx, uint32_t *pu32, bool *pexists) {
    mvalue_t value;
    mr_get_scalar(pctx, idx, &value, pexists);
    *pu32 = (uint32_t)value;
    return 0;
}

int mr_set_vector(packet_ctx *pctx, int idx, void *pvoid, size_t len) {
    if (mr_reset_vector(pctx, idx)) return -1;
    mr_mdata *mdata = pctx->mdata0 + idx;
    mdata->value = (mvalue_t)pvoid;
    mdata->vexists = mdata->value ? true : false;
    mdata->vlen = mdata->value ? len : 0;
    mr_mdata_fn count_fn = _DTYPE[mdata->dtype].count_fn;
    if (count_fn(pctx, mdata)) return -1; // sets u8vlen

    mr_mdata_fn validate_fn = _DTYPE[mdata->dtype].validate_fn;
    if (mdata->value && validate_fn && validate_fn(pctx, mdata)) {
        mr_reset_vector(pctx, idx);
        return -1;
    }

    return 0;
}

static int mr_get_vector(packet_ctx *pctx, int idx, mvalue_t *ppvoid, size_t *plen, bool *pexists) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    *ppvoid = mdata->value; // for a vector, value is a pointer to something or NULL
    *plen = mdata->vlen;
    *pexists = mdata->vexists;
    return 0;
}

int mr_get_str(packet_ctx *pctx, int idx, char **pcv0, bool *pexists) {
    mvalue_t pvoid;
    size_t len;
    mr_get_vector(pctx, idx, &pvoid, &len, pexists);
    *pcv0 = (char *)pvoid;
    return 0;
}

int mr_get_u8v(packet_ctx *pctx, int idx, uint8_t **pu8v0, size_t *plen, bool *pexists) {
    mvalue_t pvoid;
    mr_get_vector(pctx, idx, &pvoid, plen, pexists);
    *pu8v0 = (uint8_t *)pvoid;
    return 0;
}

int mr_get_spv(packet_ctx *pctx, int idx, string_pair **pspv0, size_t *plen, bool *pexists) {
    mvalue_t pvoid;
    mr_get_vector(pctx, idx, &pvoid, plen, pexists);
    *pspv0 = (string_pair *)pvoid;
    return 0;
}

int mr_reset_vector(packet_ctx *pctx, int idx) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    mr_mdata_fn free_fn = _DTYPE[mdata->dtype].free_fn;
    return free_fn(pctx, mdata);
}

int mr_reset_scalar(packet_ctx *pctx, int idx) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    mdata->value = 0;
    mdata->vexists = false;
    return 0;
}

int mr_pack_packet(packet_ctx *pctx) {
    if (pctx->u8valloc && mr_free(pctx->u8v0)) return -1;
    const mr_mdata_fn vbi_count_fn = _DTYPE[MR_VBI_DTYPE].count_fn;
    mr_mdata *mdata = pctx->mdata0 + pctx->mdata_count - 1; // last one

    for (int i = pctx->mdata_count - 1; i > -1; mdata--, i--) { // go in reverse to calculate VBIs
        if (mdata->vexists) {
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
            mr_mdata_fn pack_fn = _DTYPE[mdata->dtype].pack_fn;
            if (pack_fn && pack_fn(pctx, mdata)) return -1; // each pack_fn increments pctx->u8vpos
        }
    }

    return 0;
}

int mr_init_unpack_packet(
    packet_ctx **ppctx,
    const mr_mdata *MDATA_TEMPLATE, size_t mdata_count,
    uint8_t *u8v0, size_t u8vlen
) {
    if (mr_init_packet(ppctx, MDATA_TEMPLATE, mdata_count)) return -1;
    packet_ctx *pctx = *ppctx;
    pctx->u8v0 = u8v0;
    pctx->u8vlen = u8vlen;
    pctx->u8valloc = false;
    if (mr_unpack_packet(pctx)) return -1;
    pctx->u8v0 = NULL; // dereference
    pctx->u8vlen = 0;
    return 0;
}

static int mr_unpack_packet(packet_ctx *pctx) {
    mr_mdata *mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        if (!mdata->propid) { // properties are handled separately
            if (mdata->flagid) { // controlling flagid exists
                mr_mdata *flag_mdata = pctx->mdata0 + mdata->flagid;
                if (!flag_mdata->value) continue; // skip this mdata if flag is not set
            }

            mr_mdata_fn unpack_fn = _DTYPE[mdata->dtype].unpack_fn;
            if (unpack_fn && unpack_fn(pctx, mdata)) return -1;
            mr_mdata_fn validate_fn = _DTYPE[mdata->dtype].validate_fn;
            if (validate_fn && validate_fn(pctx, mdata)) return -1;
        }
    }

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

static int mr_count_VBI(packet_ctx *pctx, mr_mdata *mdata) {
    size_t cum_len = 0;
    mr_mdata *cnt_mdata;

    //  accumulate u8vlens in cum_len for the range of the VBI
    for (int i = mdata->idx + 1; i <= mdata->link; i++) {
        cnt_mdata = pctx->mdata0 + i;
        if (cnt_mdata->vexists) cum_len += cnt_mdata->u8vlen;
    }

    mdata->value = cum_len;
    uint32_t u32 = mdata->value;
    int rc = mr_bytecount_VBI(u32);
    if (rc < 0) return rc;
    mdata->vlen = rc;
    mdata->u8vlen = rc;
    return 0;
}

static int mr_pack_VBI(packet_ctx *pctx, mr_mdata *mdata) {
    uint32_t u32 = mdata->value;
    uint8_t u8v[4];
    int rc = mr_make_VBI(u32, u8v);
    if (rc < 0) return rc;
    memcpy(pctx->u8v0 + pctx->u8vpos, u8v, mdata->u8vlen);
    pctx->u8vpos += mdata->u8vlen;
    return 0;
}

static int mr_unpack_VBI(packet_ctx *pctx, mr_mdata *mdata) {
    uint32_t u32;
    int rc = mr_get_VBI(&u32, pctx->u8v0 + pctx->u8vpos);
    if (rc < 0) return rc;
    mdata->value = u32;
    mdata->vlen = rc;
    mdata->u8vlen = rc;
    mdata->vexists = true;
    pctx->u8vpos += rc;
    return 0;
 }

static int mr_free_vector(packet_ctx *pctx, mr_mdata *mdata) {
    if (mdata->valloc && mr_free((void *)mdata->value)) return -1;
    mdata->value = (mvalue_t)NULL;
    mdata->valloc = false;
    mdata->vexists = false;
    mdata->vlen = 0;
    mdata->u8vlen = 0;
    return 0;
}

int mr_free_packet_context(packet_ctx *pctx) {
    mr_mdata_fn free_fn;
    mr_mdata *mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; i++, mdata++) {
        free_fn = _DTYPE[mdata->dtype].free_fn;
        if (mdata->valloc && free_fn && free_fn(pctx, mdata)) return -1;
    }

    if (pctx->u8valloc) free(pctx->u8v0);
    free(pctx->mdata_dump);
    free(pctx);
    return 0;
}

int mr_init_packet(packet_ctx **ppctx, const mr_mdata *MDATA_TEMPLATE, size_t mdata_count) {
    packet_ctx *pctx;

    if (mr_calloc((void **)&pctx, 1, sizeof(packet_ctx))) return -1;

    pctx->mdata_count = mdata_count;
    mr_mdata *mdata0;

    if (mr_malloc((void **)&mdata0, mdata_count * sizeof(mr_mdata))) return -1;

    memcpy(mdata0, MDATA_TEMPLATE, mdata_count * sizeof(mr_mdata));
    pctx->mdata0 = mdata0;
    pctx->mqtt_packet_type = mdata0->value; // always the value of the 0th mdata row
    pctx->mqtt_packet_name = _PTYPE_NAME[pctx->mqtt_packet_type >> 4]; // left nibble is index
    *ppctx = pctx;
    return 0;
}

static int mr_pack_u8(packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t propid = mdata->propid;
    if (propid) pctx->u8v0[pctx->u8vpos++] = propid;
    pctx->u8v0[pctx->u8vpos++] = mdata->value;
    return 0;
}

static int mr_unpack_u8(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->vexists = true;
    mdata->value = pctx->u8v0[pctx->u8vpos++];
    return 0;
}

static int mr_pack_u16(packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t propid = mdata->propid;
    if (propid) pctx->u8v0[pctx->u8vpos++] = propid;
    uint16_t u16 = mdata->value;
    pctx->u8v0[pctx->u8vpos++] = (u16 >> 8) & 0xFF;
    pctx->u8v0[pctx->u8vpos++] = u16 & 0xFF;
    return 0;
}

static int mr_unpack_u16(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->vexists = true;
    uint8_t *u8v = pctx->u8v0 + pctx->u8vpos;
    uint16_t u16v[] = {u8v[0], u8v[1]};
    mdata->value = (u16v[0] << 8) + u16v[1];
    pctx->u8vpos += 2;
    return 0;
}

static int mr_pack_u32(packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t propid = mdata->propid;
    if (propid) pctx->u8v0[pctx->u8vpos++] = propid;
    uint32_t u32 = mdata->value;
    pctx->u8v0[pctx->u8vpos++] = (u32 >> 24) & 0xFF;
    pctx->u8v0[pctx->u8vpos++] = (u32 >> 16) & 0xFF;
    pctx->u8v0[pctx->u8vpos++] = (u32 >> 8) & 0xFF;
    pctx->u8v0[pctx->u8vpos++] = u32 & 0xFF;
    return 0;
}

static int mr_unpack_u32(packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t *u8v = pctx->u8v0 + pctx->u8vpos;
    uint32_t u32v[] = {u8v[0], u8v[1], u8v[2], u8v[3]};
    mdata->value = (u32v[0] << 24) + (u32v[1] << 16) + (u32v[2] << 8) + u32v[3];
    mdata->vexists = true;
    pctx->u8vpos += 4;
    return 0;
}

static int mr_count_str(packet_ctx *pctx, mr_mdata *mdata) {
    if (mr_count_u8v(pctx, mdata)) return -1;
    mdata->u8vlen--; // correct for vlen = strlen() + 1
    return 0;
}

static int mr_pack_str(packet_ctx *pctx, mr_mdata *mdata) {
    return mr_pack_u8v(pctx, mdata);
}

static int mr_unpack_str(packet_ctx *pctx, mr_mdata *mdata) {
    return mr_unpack_u8v(pctx, mdata);
}

static int mr_validate_str(packet_ctx *pctx, mr_mdata *mdata) {
    char *pc = (char *)mdata->value;
    int err_pos = utf8val((uint8_t *)pc, strlen(pc)); // returns error position

    if (err_pos) {
        dzlog_error(
            "invalid utf8:: packet: %s; name: %s; value: %s, pos: %d",
            pctx->mqtt_packet_name, mdata->name, pc, err_pos
        );

        return -1;
    }

    return 0;
}

static int mr_count_spv(packet_ctx *pctx, mr_mdata *mdata) { // spv's are properties
    string_pair *spv = (string_pair *)mdata->value;

    mdata->u8vlen = 0;
    for (int i = 0; i < mdata->vlen; i++) { // propid(u8) + u16 + nlen + u16 + vlen
        mdata->u8vlen += 1 + 2 + strlen(spv[i].name) + 2 + strlen(spv[i].value);
    }

    return 0;
}

static int mr_pack_spv(packet_ctx *pctx, mr_mdata *mdata) {
    string_pair *spv = (string_pair *)mdata->value;
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

static int mr_unpack_spv(packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t *u8v = pctx->u8v0 + pctx->u8vpos;

    uint16_t u16v[] = {u8v[0], u8v[1]}; u8v += 2;
    size_t namelen = (u16v[0] << 8) + u16v[1];
    char *name;
    if (mr_malloc((void **)&name, namelen + 1)) return -1;
    memcpy(name, u8v, namelen); u8v += namelen;
    name[namelen] = '\0';

    u16v[0] = u8v[0]; u16v[1] = u8v[1]; u8v += 2;
    size_t valuelen = (u16v[0] << 8) + u16v[1];
    char *value;
    if (mr_malloc((void **)&value, valuelen + 1)) return -1;
    memcpy(value, u8v, valuelen); u8v += valuelen;
    value[valuelen] = '\0';

    string_pair *spv0, *psp;
    if (mdata->valloc) {
        spv0 = (string_pair *)mdata->value;
        if (mr_realloc((void **)&spv0, (mdata->vlen + 1) * sizeof(string_pair))) return -1;
    }
    else {
        if (mr_malloc((void **)&spv0, sizeof(string_pair))) return -1;
        mdata->vlen = 0; // incremented below
    }

    mdata->value = (mvalue_t)spv0;
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

static int mr_validate_spv(packet_ctx *pctx, mr_mdata *mdata) {
    string_pair *spv = (string_pair *)mdata->value;

    for (int i = 0; i < mdata->vlen; i++) { // utf8val returns error position
        int err_pos = utf8val((uint8_t *)spv[i].name, strlen(spv[i].name));

        if (err_pos) {
            dzlog_error(
                "invalid utf8: packet: %s; string_pair: %d; name: %s; pos: %d",
                pctx->mqtt_packet_name, i, spv[i].name, err_pos
            );

            return -1;
        }

        err_pos = utf8val((uint8_t *)spv[i].value, strlen(spv[i].value));

        if (err_pos) {
            dzlog_error(
                "invalid utf8: packet: %s; string_pair: %d; value: %s; pos: %d",
                pctx->mqtt_packet_name, i, spv[i].value, err_pos
            );

            return -1;
        }
    }

    return 0;
}

static int mr_free_spv(packet_ctx *pctx, mr_mdata *mdata) {
    string_pair *spv0 = (string_pair *)mdata->value;

    string_pair *psp = spv0;
    for (int i = 0; i < mdata->vlen; psp++, i++) {
        free(psp->name);
        free(psp->value);
    }

    free(spv0);
    mdata->value = (mvalue_t)NULL;
    mdata->vexists = false;
    mdata->valloc = false;
    mdata->vlen = 0;
    mdata->u8vlen = 0;
    return 0;
}

static int mr_unpack_props(packet_ctx *pctx, mr_mdata *mdata) {
    dzlog_debug(
        "mr_unpack_props:: packet: %s; name: %s",
        pctx->mqtt_packet_name, mdata->name
    );

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

        if (prop_mdata->vexists && prop_mdata->dtype != MR_SPV_DTYPE) {
            dzlog_error(
                "duplicate property value:: packet: %s; name: %s",
                pctx->mqtt_packet_name, prop_mdata->name
            );

            return -1;
        }

        unpack_fn = _DTYPE[prop_mdata->dtype].unpack_fn;
        if (unpack_fn(pctx, prop_mdata)) return -1;
        validate_fn = _DTYPE[mdata->dtype].validate_fn;
        if (validate_fn && validate_fn(pctx, mdata)) return -1;
    }

    return 0;
}

static int mr_count_u8v(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->u8vlen = 2 + mdata->vlen;
    if (mdata->propid) mdata->u8vlen++;
    return 0;
}

static int mr_pack_u8v(packet_ctx *pctx, mr_mdata *mdata) {
    bool bstr = mdata->dtype == MR_STR_DTYPE;
    uint8_t propid = mdata->propid;
    if (propid) pctx->u8v0[pctx->u8vpos++] = propid;
    uint16_t u16 = mdata->vlen - (bstr ? 1 : 0);
    pctx->u8v0[pctx->u8vpos++] = (u16 >> 8) & 0xFF;
    pctx->u8v0[pctx->u8vpos++] = u16 & 0xFF;
    memcpy(pctx->u8v0 + pctx->u8vpos, (uint8_t *)mdata->value, u16);
    pctx->u8vpos += u16;
    return 0;
}

static int mr_unpack_u8v(packet_ctx *pctx, mr_mdata *mdata) {
    bool bstr = mdata->dtype == MR_STR_DTYPE;
    uint8_t *u8v = pctx->u8v0 + pctx->u8vpos;
    uint16_t u16v[] = {u8v[0], u8v[1]}; u8v += 2;
    size_t u8vlen = (u16v[0] << 8) + u16v[1];
    size_t vlen = u8vlen + (bstr ? 1 : 0);

    uint8_t *value;
    if (mr_calloc((void **)&value, vlen, 1)) return -1;

    memcpy(value, u8v, u8vlen);
    mdata->value = (mvalue_t)value;
    mdata->vlen = vlen;
    mdata->vexists = true;
    mdata->valloc = true;
    mdata->u8vlen = (mdata->propid ? 1 : 0) + 2 + u8vlen;
    pctx->u8vpos += 2 + u8vlen;
    return 0;
}

//  idx is vlen: # of bits to be (re)set
static const uint8_t _BIT_MASKS[] = {
    0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};

static int mr_pack_bits_in_value(packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t bitpos = mdata->bpos;
    mr_mdata *flags_mdata = pctx->mdata0 + mdata->link;
    uint8_t flags_u8 = flags_mdata->value;

    flags_u8 &= ~(_BIT_MASKS[mdata->vlen] << bitpos); // reset

    if (mdata->value) {
        uint8_t u8 = mdata->value;
        flags_u8 |= u8 << bitpos; // set
    }

    flags_mdata->value = flags_u8;
    return 0;
}

static int mr_pack_bits(packet_ctx *pctx, mr_mdata *mdata) { // don't advance pctx->u8vpos
    uint8_t bitpos = mdata->bpos;
    uint8_t *pu8 = pctx->u8v0 + pctx->u8vpos;

    *pu8 &= ~(_BIT_MASKS[mdata->vlen] << bitpos); // reset

    if (mdata->value) {
        uint8_t u8 = mdata->value;
        *pu8 |= u8 << bitpos; // set
    }

    return 0;
}

static int mr_pack_incr1(packet_ctx *pctx, mr_mdata *mdata) { // now advance - bits all packed
    pctx->u8vpos++;
    return 0;
}

static int mr_unpack_bits(packet_ctx *pctx, mr_mdata *mdata) {  // don't advance pctx->u8vpos; unpacking
    uint8_t bitpos = mdata->bpos;                               // the following flags byte will do that
    uint8_t *pu8 = pctx->u8v0 + pctx->u8vpos;
    mdata->value = *pu8 >> bitpos & _BIT_MASKS[mdata->vlen];
    mdata->vexists = true;
    return 0;
}
