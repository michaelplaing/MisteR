/* packet.c */

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mister/mister.h"
#include "packet_internal.h"
#include "util_internal.h"
#include "will_internal.h"
#include "mister/memory.h"
#include "mister/mrzlog.h"

static char *PTYPE_NAME[] = {   // same order as mqtt_packet_type
    "", "CONNECT", "CONNACK",   // Note: enum high nibbles are 1-based, hence dummy str
    "PUBLISH", "PUBACK", "PUBREC", "PUBREL", "PUBCOMP",
    "SUBSCRIBE", "SUBACK", "UNSUBSCRIBE", "UNSUBACK",
    "PINGREQ", "PINGRESP", "DISCONNECT", "AUTH"
};

static const mr_dtype DTYPE_MDATA0[] = { // same order as mr_dtypes enum
//   dtype idx          pack_fn             unpack_fn           validate_fn         free_fn
    {MR_U8_DTYPE,       mr_pack_u8,         mr_unpack_u8,       NULL,               NULL},
    {MR_U16_DTYPE,      mr_pack_u16,        mr_unpack_u16,      NULL,               NULL},
    {MR_U32_DTYPE,      mr_pack_u32,        mr_unpack_u32,      NULL,               NULL},
    {MR_VBI_DTYPE,      mr_pack_VBI,        mr_unpack_VBI,      NULL,               NULL},
    {MR_BITS_DTYPE,     mr_pack_bits,       mr_unpack_bits,     NULL,               NULL},
    {MR_U8V_DTYPE,      mr_pack_u8v,        mr_unpack_u8v,      NULL,               mr_free_value},
    {MR_STR_DTYPE,      mr_pack_str,        mr_unpack_str,      mr_validate_str,    mr_free_value},
    {MR_SPV_DTYPE,      mr_pack_spv,        mr_unpack_spv,      mr_validate_spv,    mr_free_spv},
    {MR_FLAGS_DTYPE,    mr_pack_u8,         mr_unpack_incr1,    NULL,               NULL},
    {MR_PROPS_DTYPE,    NULL,               mr_unpack_props,    NULL,               NULL}
};

int mr_print_existing_mdata(packet_ctx *pctx) {
    printf("mdata for packet: %s\n", pctx->mqtt_packet_name);

    mr_mdata *mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        if (!mdata->vexists) continue;

        switch (mdata->dtype) {
            case MR_U8_DTYPE:
            case MR_U16_DTYPE:
            case MR_U32_DTYPE:
            case MR_VBI_DTYPE:
            case MR_BITS_DTYPE:
            case MR_FLAGS_DTYPE:
                printf("    %s: %u\n", mdata->name, (uint32_t)mdata->value);
                break;
            case MR_U8V_DTYPE:
            case MR_PROPS_DTYPE:
                printf("    %s\n", mdata->name);
                print_hexdump((uint8_t *)mdata->value, mdata->vlen);
                break;
            case MR_STR_DTYPE:
                printf("    %s: %.*s\n", mdata->name, (int)mdata->vlen, (char *)mdata->value);
                break;
            case MR_SPV_DTYPE:
                printf("    %s\n", mdata->name);
                string_pair *spv = (string_pair *)mdata->value;

                for (int i = 0; i < mdata->vlen; i++) {
                    printf(
                        "        name: %.*s; value: %.*s\n",
                        spv[i].nlen, spv[i].name, spv[i].vlen, spv[i].value
                    );
                }

                break;
        }
    }

    return 0;
}

int mr_validate_u8v(packet_ctx *pctx, int idx) { // use when u8v is validated like an str
    int rc = 0;
    mr_mdata *mdata = pctx->mdata0 + idx;
    return mr_validate_str(pctx, mdata);
}

int mr_set_scalar(packet_ctx *pctx, int idx, Word_t value) {
    int rc = 0;
    mr_mdata *mdata = pctx->mdata0 + idx;
    mr_mdata_fn validate_fn = DTYPE_MDATA0[mdata->dtype].validate_fn;
    mdata->value = value;
    mdata->vexists = true;
    if (validate_fn) rc = validate_fn(pctx, mdata);
    return rc;
}

static int mr_get_scalar(packet_ctx *pctx, int idx, Word_t *pvalue) {
    int rc;
    mr_mdata *mdata = pctx->mdata0 + idx;

    if (mdata->vexists) {
        *pvalue = mdata->value;
        rc = 0;
    }
    else {
        rc = -1;
    }

    return rc;
}

int mr_get_boolean(packet_ctx *pctx, int idx, bool *pboolean) {
    Word_t value;
    int rc = mr_get_scalar(pctx, idx, &value);
    if (!rc) *pboolean = (bool)value;
    return rc;
}

int mr_get_u8(packet_ctx *pctx, int idx, uint8_t *pu8) {
    Word_t value;
    int rc = mr_get_scalar(pctx, idx, &value);
    if (!rc) *pu8 = (uint8_t)value;
    return rc;
}

int mr_get_u16(packet_ctx *pctx, int idx, uint16_t *pu16) {
    Word_t value;
    int rc = mr_get_scalar(pctx, idx, &value);
    if (!rc) *pu16 = (uint16_t)value;
    return rc;
}

int mr_get_u32(packet_ctx *pctx, int idx, uint32_t *pu32) {
    Word_t value;
    int rc = mr_get_scalar(pctx, idx, &value);
    if (!rc) *pu32 = (uint32_t)value;
    return rc;
}

int mr_set_vector(packet_ctx *pctx, int idx, void *pvoid, size_t len) {
    int rc = 0;
    mr_mdata *mdata = pctx->mdata0 + idx;
    mr_mdata_fn validate_fn = DTYPE_MDATA0[mdata->dtype].validate_fn;
    mdata->value = (Word_t)pvoid;
    mdata->vexists = mdata->value ? true : false;
    mdata->vlen = mdata->value ? len : 0;

    if (validate_fn && mdata->value) {
        rc = validate_fn(pctx, mdata);
        if (rc) mr_reset_value(pctx, idx);
    }

    return rc;
}

static int mr_get_vector(packet_ctx *pctx, int idx, Word_t *ppvoid, size_t *plen) {
    int rc;
    mr_mdata *mdata = pctx->mdata0 + idx;

    if (mdata->vexists) {
        *ppvoid = mdata->value; // for a vector, value is a pointer to something
        *plen = mdata->vlen;
        rc = 0;
    }
    else {
        rc = -1;
    }

    return rc;
}

int mr_get_u8v(packet_ctx *pctx, int idx, uint8_t **pu8v0, size_t *plen) {
    Word_t pvoid;
    size_t len;
    int rc = mr_get_vector(pctx, idx, &pvoid, &len);

    if (!rc) {
        *pu8v0 = (uint8_t *)pvoid;
        *plen = len;
    }

    return rc;
}

int mr_get_spv(packet_ctx *pctx, int idx, string_pair **pspv0, size_t *plen) {
    Word_t pvoid;
    size_t len;
    int rc = mr_get_vector(pctx, idx, &pvoid, &len);

    if (!rc) {
        *pspv0 = (string_pair *)pvoid;
        *plen = len;
    }

    return rc;
}

int mr_reset_value(packet_ctx *pctx, int idx) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    mdata->value = 0;
    mdata->vexists = false;
    mdata->vlen = 0;
    return 0;
}

//  pack each mdata in reverse order of the table
//  into its allocated buffer using its packing function
//  then allocate pctx->u8v0 and catenate mdata buffers in it
//  free each mdata buffer if allocated
int mr_pack_pctx_u8v0(packet_ctx *pctx) {
    int rc;
    mr_mdata_fn pack_fn;
    mr_mdata *mdata = pctx->mdata0 + pctx->mdata_count - 1;

    for (int i = pctx->mdata_count - 1; i > -1; mdata--, i--) {
        if (mdata->vexists) {
            pack_fn = DTYPE_MDATA0[mdata->dtype].pack_fn;

            if(pack_fn) {
                rc = pack_fn(pctx, mdata);
                if (rc) return rc;
            }
        }
    }

    mdata = pctx->mdata0 + 1; // <CONTROL_PACKET_NAME>_REMAINING_LENGTH is always idx 1
    pctx->len = mdata->value + mdata->u8vlen + 1;

    if (mr_malloc((void **)&pctx->u8v0, pctx->len)) return -1;

    pctx->ualloc = true;

    mdata = pctx->mdata0;
    uint8_t *pu8 = pctx->u8v0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        if (mdata->u8vlen) { // is there anything to pack
            memcpy(pu8, mdata->u8v0, mdata->u8vlen);
            pu8 += mdata->u8vlen;

            if (mdata->ualloc) {
                free(mdata->u8v0);
                mdata->ualloc = false;
            }

            mdata->u8v0 = NULL;
            mdata->u8vlen = 0;
        }
    }

    return 0;
}

int mr_init_unpack_pctx(
    packet_ctx **ppctx,
    const mr_mdata *MDATA_TEMPLATE, size_t mdata_count,
    uint8_t *u8v0, size_t ulen
) {
    if (mr_init_packet_context(ppctx, MDATA_TEMPLATE, mdata_count)) return -1;
    packet_ctx *pctx = *ppctx;
    pctx->u8v0 = u8v0;
    pctx->len = ulen;
    pctx->ualloc = false;
    if (mr_unpack_pctx_u8v0(pctx)) return -1;
    pctx->u8v0 = NULL; // dereference
    pctx->len = 0;
    return 0;
}

static int mr_unpack_pctx_u8v0(packet_ctx *pctx) {
    mr_mdata *mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        if (!mdata->propid) { // properties are handled separately
            if (mdata->flagid) { // controlling flagid exists
                mr_mdata *flag_mdata = pctx->mdata0 + mdata->flagid;
                if (!flag_mdata->value) continue; // skip this mdata if flag is not set
            }

            mr_mdata_fn unpack_fn = DTYPE_MDATA0[mdata->dtype].unpack_fn;
            if (unpack_fn && unpack_fn(pctx, mdata)) return -1;
            mr_mdata_fn validate_fn = DTYPE_MDATA0[mdata->dtype].validate_fn;
            if (validate_fn && validate_fn(pctx, mdata)) return -1;
        }
    }

    return 0;
}

//  calculate length, convert to VBI, & pack into buffer
static int mr_pack_VBI(packet_ctx *pctx, mr_mdata *mdata) {
    size_t cum_len = 0;
    mr_mdata *current_mdata;

    //  accumulate buffer lengths in cum_len for the range of the VBI
    for (int i = mdata->idx + 1; i <= mdata->link; i++) {
        current_mdata = pctx->mdata0 + i;
        cum_len += current_mdata->u8vlen;
    }

    mdata->value = cum_len;
    uint32_t u32 = mdata->value; // TODO: err if too large for 4 bytes
    uint8_t u8v[5]; // enough for uint32_t even if too large
    int rc = mr_make_VBI(u32, u8v);
    if (rc < 0) return rc;
    mdata->u8vlen = rc;

    if (mr_malloc((void **)&mdata->u8v0, mdata->u8vlen)) return -1;

    mdata->ualloc = true;
    memcpy(mdata->u8v0, u8v, mdata->u8vlen);

    return 0;
}

static int mr_unpack_VBI(packet_ctx *pctx, mr_mdata *mdata) {
    uint32_t u32;
    int rc = mr_get_VBI(&u32, pctx->u8v0 + pctx->pos);
    if (rc < 0) return rc;

    mdata->value = u32;
    mdata->vlen = rc;
    mdata->vexists = true;
    pctx->pos += rc;

    return 0;
 }

static int mr_free_value(packet_ctx *pctx, mr_mdata *mdata) {
    free((void *)mdata->value);
    mdata->value = (Word_t)NULL;
    mdata->valloc = false;
    mdata->vexists = false;
    mdata->vlen = 0;
    return 0;
}

int mr_free_packet_context(packet_ctx *pctx) {
    mr_mdata_fn free_fn;
    mr_mdata *mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; i++, mdata++) {
        if (mdata->ualloc) free((void *)mdata->u8v0);

        if (mdata->valloc) {
            free_fn = DTYPE_MDATA0[mdata->dtype].free_fn;
            if (free_fn) free_fn(pctx, mdata);
        }
    }

    if (pctx->ualloc) free(pctx->u8v0);
    free(pctx);

    return 0;
}

int mr_init_packet_context(packet_ctx **ppctx, const mr_mdata *MDATA_TEMPLATE, size_t mdata_count) {
    packet_ctx *pctx;

    if (mr_calloc((void **)&pctx, 1, sizeof(packet_ctx))) return -1;

    pctx->mdata_count = mdata_count;
    mr_mdata *mdata0;

    if (mr_malloc((void **)&mdata0, mdata_count * sizeof(mr_mdata))) return -1;

    memcpy(mdata0, MDATA_TEMPLATE, mdata_count * sizeof(mr_mdata)); // copy template

    pctx->mdata0 = mdata0;
    pctx->mqtt_packet_type = mdata0->value; // always the value of the 0th mdata row
    pctx->mqtt_packet_name = PTYPE_NAME[pctx->mqtt_packet_type >> 4]; // left nibble is index
    *ppctx = pctx;
    return 0;
}

static int mr_pack_u8(packet_ctx *pctx, mr_mdata *mdata) {
    size_t u8vlen = 1;
    uint8_t propid = mdata->propid;
    if (propid) u8vlen++;

    uint8_t *pu8;
    if (mr_malloc((void **)&pu8, mdata->u8vlen)) return -1;

    mdata->u8v0 = pu8;
    if (propid) *pu8++ = propid;
    *pu8 = mdata->value;
    mdata->ualloc = true;
    mdata->u8vlen = u8vlen;
    return 0;
}

static int mr_unpack_u8(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->vexists = true;
    mdata->vlen = 1;
    mdata->value = pctx->u8v0[pctx->pos++];
    return 0;
}

static int mr_pack_u16(packet_ctx *pctx, mr_mdata *mdata) {
    size_t u8vlen = 2;
    uint8_t propid = mdata->propid;
    if (propid) u8vlen++;

    uint8_t *pu8;
    if (mr_malloc((void **)&pu8, mdata->u8vlen)) return -1;

    mdata->u8v0 = pu8;
    if (propid) *pu8++ = propid;
    uint16_t u16 = mdata->value;
    *pu8++ = (u16 >> 8) & 0xFF;
    *pu8 = u16 & 0xFF;
    mdata->ualloc = true;
    mdata->u8vlen = u8vlen;
    return 0;
}

static int mr_unpack_u16(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->vlen = 2;
    mdata->vexists = true;
    uint8_t *u8v = pctx->u8v0 + pctx->pos;
    uint16_t u16v[] = {u8v[0], u8v[1]};
    mdata->value = (u16v[0] << 8) + u16v[1];
    pctx->pos += 2;
    return 0;
}

static int mr_pack_u32(packet_ctx *pctx, mr_mdata *mdata) {
    size_t u8vlen = 4;
    uint8_t propid = mdata->propid;
    if (propid) u8vlen++;

    uint8_t *pu8;
    if (mr_malloc((void **)&pu8, mdata->u8vlen)) return -1;

    mdata->u8v0 = pu8;
    if (propid) *pu8++ = propid;
    uint32_t u32 = mdata->value;
    *pu8++ = (u32 >> 24) & 0xFF;
    *pu8++ = (u32 >> 16) & 0xFF;
    *pu8++ = (u32 >> 8) & 0xFF;
    *pu8 = u32 & 0xFF;
    mdata->ualloc = true;
    mdata->u8vlen = u8vlen;
    return 0;
}

static int mr_unpack_u32(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->vlen = 4;
    uint8_t *u8v = pctx->u8v0 + pctx->pos;
    uint32_t u32v[] = {u8v[0], u8v[1], u8v[2], u8v[3]};
    mdata->value = (u32v[0] << 24) + (u32v[1] << 16) + (u32v[2] << 8) + u32v[3];
    mdata->vexists = true;
    pctx->pos += 4;
    return 0;
}

static int mr_pack_str(packet_ctx *pctx, mr_mdata *mdata) {
    return mr_pack_u8v(pctx, mdata);
}

static int mr_unpack_str(packet_ctx *pctx, mr_mdata *mdata) {
    return mr_unpack_u8v(pctx, mdata);
}

static int mr_validate_str(packet_ctx *pctx, mr_mdata *mdata) {
    int rc = 0;
    int err_pos = utf8val((uint8_t *)mdata->value, mdata->vlen); // returns error position

    if (err_pos) {
        dzlog_error(
            "invalid utf8:: packet: %s; name: %s; value: %s, pos: %d",
            pctx->mqtt_packet_name, mdata->name, (char *)mdata->value, err_pos
        );

        rc = -1;
    }

    return rc;
}

static int mr_pack_spv(packet_ctx *pctx, mr_mdata *mdata) {
    string_pair *spv = (string_pair *)mdata->value;

    mdata->u8vlen = 0;
    for (int i = 0; i < mdata->vlen; i++) {
        mdata->u8vlen += 1 + 2 + spv[i].nlen + 2 + spv[i].vlen;
    }

    if (mr_malloc((void **)&mdata->u8v0, mdata->u8vlen)) return -1;

    mdata->ualloc = true;
    uint8_t *pu8 = mdata->u8v0;
    uint16_t u16;

    for (int i = 0; i < mdata->vlen; i++) {
        *pu8++ = mdata->propid;
        // name
        u16 = spv[i].nlen;
        *pu8++ = (u16 >> 8) & 0xFF;
        *pu8++ = u16 & 0xFF;
        memcpy(pu8, spv[i].name, u16);
        pu8 += u16;
        // value
        u16 = spv[i].vlen;
        *pu8++ = (u16 >> 8) & 0xFF;
        *pu8++ = u16 & 0xFF;
        memcpy(pu8, spv[i].value, u16);
        pu8 += u16;
    }

    return 0;
}

static int mr_unpack_spv(packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t *u8v = pctx->u8v0 + pctx->pos;

    uint16_t u16v[] = {u8v[0], u8v[1]}; u8v += 2;
    size_t nlen = (u16v[0] << 8) + u16v[1];

    uint8_t *name;
    if (mr_malloc((void **)&name, nlen)) return -1;

    memcpy(name, u8v, nlen); u8v += nlen;

    u16v[0] = u8v[0]; u16v[1] = u8v[1]; u8v += 2;
    size_t vlen = (u16v[0] << 8) + u16v[1];

    uint8_t *value;
    if (mr_malloc((void **)&value, vlen)) return -1;

    memcpy(value, u8v, vlen); u8v += vlen;

    string_pair *spv0, *psp;

    if (mdata->valloc) {
        spv0 = (string_pair *)mdata->value;
        if (mr_realloc((void **)&spv0, (mdata->vlen + 1) * sizeof(string_pair))) return -1;
    }
    else {
        if (mr_malloc((void **)&spv0, sizeof(string_pair))) return -1;
        mdata->vlen = 0;
    }

    mdata->value = (Word_t)spv0;
    psp = spv0 + mdata->vlen;
    psp->nlen = nlen;
    psp->name = name;
    psp->vlen = vlen;
    psp->value = value;
    mdata->vlen++;
    mdata->vexists = true;
    mdata->valloc = true;
    pctx->pos += 2 + nlen + 2 + vlen;
    return 0;
}

static int mr_validate_spv(packet_ctx *pctx, mr_mdata *mdata) {
    string_pair *spv = (string_pair *)mdata->value;

    for (int i = 0; i < mdata->vlen; i++) { // utf8val returns error position
        int err_pos = utf8val(spv[i].name, spv[i].nlen);

        if (err_pos) {
            dzlog_error(
                "invalid utf8: packet: %s; string_pair: %d; name: %.*s; pos: %d",
                pctx->mqtt_packet_name, i, spv[i].nlen, spv[i].name, err_pos
            );

            return -1;
        }

        err_pos = utf8val(spv[i].value, spv[i].vlen);

        if (err_pos) {
            dzlog_error(
                "invalid utf8: packet: %s; string_pair: %d; value: %.*s; pos: %d",
                pctx->mqtt_packet_name, i, spv[i].vlen, spv[i].value, err_pos
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
    mdata->value = (Word_t)NULL;
    mdata->vexists = false;
    mdata->valloc = false;
    mdata->vlen = 0;
    return 0;
}

static int mr_unpack_props(packet_ctx *pctx, mr_mdata *mdata) {
    dzlog_debug(
        "mr_unpack_props:: packet: %s; name: %s",
        pctx->mqtt_packet_name, mdata->name
    );

    mdata->vexists = true;

/*
    if (mdata->flagid) {
        mr_mdata *flag_mdata = pctx->mdata0 + mdata->flagid;
        if (!flag_mdata->value) return 0;
    }
*/
    size_t end_pos = pctx->pos + (mdata - 1)->value; // use property_length
    uint8_t *pu8, *pprop_index;
    int prop_index;
    mr_mdata *prop_mdata;
    mr_mdata_fn unpack_fn;

    for (; pctx->pos < end_pos;) {
        pu8 = pctx->u8v0 + pctx->pos++;
        pprop_index = memchr((uint8_t *)mdata->value, *pu8, mdata->vlen);

        if (!pprop_index) {
            dzlog_error(
                "property id not found:: packet: %s; name: %s; propid: %d",
                pctx->mqtt_packet_name, mdata->name, *pu8
            );

            return -1;
        }
        else {
            dzlog_debug(
                "mr_unpack_props:: packet: %s; name: %s; propid: %d",
                pctx->mqtt_packet_name, mdata->name, *pu8
            );
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

        unpack_fn = DTYPE_MDATA0[prop_mdata->dtype].unpack_fn;
        if (unpack_fn && unpack_fn(pctx, prop_mdata)) break;
    }

    return 0;
}

static int mr_pack_u8v(packet_ctx *pctx, mr_mdata *mdata) {
    size_t u8vlen = 2 + mdata->vlen;
    uint8_t propid = mdata->propid;
    if (propid) u8vlen++;

    if (mr_malloc((void **)&mdata->u8v0, u8vlen)) return -1;

    mdata->ualloc = true;
    mdata->u8vlen = u8vlen;

    uint8_t *pu8 = mdata->u8v0;
    if (propid) *pu8++ = propid;
    uint16_t u16 = mdata->vlen;
    *pu8++ = (u16 >> 8) & 0xFF;
    *pu8++ = u16 & 0xFF;
    memcpy(pu8, (uint8_t *)mdata->value, u16);
    return 0;
}

static int mr_unpack_u8v(packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t *u8v = pctx->u8v0 + pctx->pos;
    uint16_t u16v[] = {u8v[0], u8v[1]}; u8v += 2;
    size_t vlen = (u16v[0] << 8) + u16v[1];

    uint8_t *value;
    if (mr_malloc((void **)&value, vlen)) return -1;

    memcpy(value, u8v, vlen);
    mdata->value = (Word_t)value;
    mdata->vlen = vlen;
    mdata->vexists = true;
    mdata->valloc = true;
    pctx->pos += 2 + vlen;
    return 0;
}

//  idx is vlen: # of bits to be (re)set
static const uint8_t BIT_MASKS[] = {
    0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};

//  get the link mdata byte, reset specified bit(s) and set if value is non-zero
static int mr_pack_bits(packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t bitpos = mdata->bp;
    mr_mdata *link_mdata = pctx->mdata0 + mdata->link;
    uint8_t *u8v0 = link_mdata->u8v0;

    *u8v0 &= ~(BIT_MASKS[mdata->vlen] << bitpos);

    if (mdata->value) {
        uint8_t u8 = mdata->value;
        *u8v0 |= u8 << bitpos;
    }

    return 0;
}

static int mr_unpack_bits(packet_ctx *pctx, mr_mdata *mdata) { // don't advance pctx->pos
    uint8_t bitpos = mdata->bp;
    uint8_t *pu8 = pctx->u8v0 + pctx->pos;
    mdata->value = *pu8 >> bitpos & BIT_MASKS[mdata->vlen];
    mdata->vexists = true;
    return 0;
}

static int mr_unpack_incr1(packet_ctx *pctx, mr_mdata *mdata) { // now advance - bits all unpacked
    mdata->vexists = true;
    mdata->vlen = 1;
    pctx->pos++;
    return 0;
}
