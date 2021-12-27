/* packet.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "packet_internal.h"

const mr_dtype DTYPE_MDATA0[] = {
//   dtype idx          pack_fn             unpack_fn           free_fn
    {MR_U8_DTYPE,       mr_pack_u8,         mr_unpack_u8,       NULL},
    {MR_U16_DTYPE,      mr_pack_u16,        mr_unpack_u16,      NULL},
    {MR_U32_DTYPE,      mr_pack_u32,        mr_unpack_u32,      NULL},
    {MR_VBI_DTYPE,      mr_pack_VBI,        mr_unpack_VBI,      NULL},
    {MR_U8V_DTYPE,      mr_pack_u8v,        mr_unpack_u8v,      mr_free_value},
    {MR_U8VF_DTYPE,     mr_pack_u8vf,       mr_unpack_u8vf,     NULL},
    {MR_BITS_DTYPE,     mr_pack_bits,       mr_unpack_bits,     NULL},
    {MR_STR_DTYPE,      mr_pack_str,        mr_unpack_str,      mr_free_value},
    {MR_SPV_DTYPE,      mr_pack_spv,        mr_unpack_spv,      mr_free_spv},
    {MR_FLAGS_DTYPE,    mr_pack_u8,         mr_unpack_incr1,    NULL},
    {MR_PROPS_DTYPE,    NULL,               mr_unpack_props,    NULL}
};

int mr_set_scalar(packet_ctx *pctx, int idx, Word_t value) {
    mr_mdata *mdata = pctx->mdata0 + idx;
    mdata->value = value;
    mdata->vexists = true;
    return 0;
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
    mr_mdata *mdata = pctx->mdata0 + idx;
    mdata->value = (Word_t)pvoid;
    mdata->vexists = true;
    mdata->vlen = len;
    return 0;
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
int mr_pack_mdata_u8v0(packet_ctx *pctx) {
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

    uint8_t *u8v0 = malloc(pctx->len); // TODO: err checks on malloc

    pctx->u8v0 = u8v0;
    pctx->ualloc = true;

    mdata = pctx->mdata0;
    uint8_t *pu8 = u8v0;
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

int mr_unpack_mdata_u8v0(packet_ctx *pctx) {
    int rc;
    bool isprop;
    mr_mdata_fn unpack_fn;
    mr_mdata *mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        isprop = mdata->id && (mdata->dtype != MR_BITS_DTYPE); // DTYPE_MDATA0[mdata->dtype].isprop;

        if (!isprop) {
            unpack_fn = DTYPE_MDATA0[mdata->dtype].unpack_fn;

            if (unpack_fn) {
                rc = unpack_fn(pctx, mdata);
                if (rc) return rc;
            }
        }
    }

    return 0;
}

static int mr_make_VBI(uint32_t u32, uint8_t *u8v0) {
    if (u32 >> (7 * 4)) { // overflow: too big for 4 bytes
        return -1;
    }

    uint8_t *pu8 = u8v0;
    int i = 0;
    do {
        *pu8 = u32 & 0x7F;
        u32 = u32 >> 7;
        if (u32) *pu8 |= 0x80;
        i++; pu8++;
    } while (u32);

    return i;
}

static int mr_get_VBI(uint32_t *pu32, uint8_t *u8v) {
    uint8_t *pu8 = u8v;
    uint32_t u32, result_u32 = 0;
    int i;
    for (i = 0; i < 4; pu8++, i++){
        u32 = *pu8;
        result_u32 += (u32 & 0x7F) << (7 * i);
        if (!(*pu8 & 0x80)) break;
    }

    if (i == 4) { // overflow: byte[3] has a continuation bit
        return -1;
    }
    else {
        *pu32 = result_u32;
        return i + 1;
    }
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
    uint8_t u8v[5]; // enough for uint32_t even if too big
    int rc = mr_make_VBI(u32, u8v);
    if (rc < 0) return rc;
    mdata->u8vlen = rc;

    uint8_t *u8v0 = malloc(mdata->u8vlen); // TODO: err checks on malloc

    mdata->ualloc = true;
    mdata->u8v0 = u8v0;
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
    // pctx->limit = pctx->pos + u32;

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
    packet_ctx *pctx = calloc(1, sizeof(packet_ctx));
    pctx->mdata_count = mdata_count;
    mr_mdata *mdata0 = malloc(mdata_count * sizeof(mr_mdata));
    //  copy template
    memcpy(mdata0, MDATA_TEMPLATE, mdata_count * sizeof(mr_mdata));
    // for (int i = 0; i < pctx->mdata_count; i++) mdata0[i] = MDATA_TEMPLATE[i];
    pctx->mqtt_packet_type = mdata0->value; // always the value of the 0th mdata row
    pctx->mdata0 = mdata0;
    *ppctx = pctx;
    return 0;
}

static int mr_pack_u8(packet_ctx *pctx, mr_mdata *mdata) {
    size_t u8vlen = 1;
    uint8_t prop_id = mdata->id;
    if (prop_id) u8vlen++;

    uint8_t *pu8 = malloc(mdata->u8vlen); // TODO: err checks on malloc

    mdata->u8v0 = pu8;
    if (prop_id) *pu8++ = prop_id;
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
    uint8_t prop_id = mdata->id;
    if (prop_id) u8vlen++;

    uint8_t *pu8 = malloc(mdata->u8vlen); // TODO: err checks on malloc

    mdata->u8v0 = pu8;
    if (prop_id) *pu8++ = prop_id;
    uint16_t u16 = mdata->value;
    *pu8++ = (u16 >> 8) & 0xFF;
    *pu8 = u16 & 0xFF;
    mdata->ualloc = true;
    mdata->u8vlen = u8vlen;
    return 0;
}

static int mr_unpack_u16(packet_ctx *pctx, mr_mdata *mdata) {
    //if (!pctx->ualloc || pctx->pos >= pctx->len) return 0;
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
    uint8_t prop_id = mdata->id;
    if (prop_id) u8vlen++;

    uint8_t *pu8 = malloc(mdata->u8vlen); // TODO: err checks on malloc

    mdata->u8v0 = pu8;
    if (prop_id) *pu8++ = prop_id;
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
    uint8_t prop_id = mdata->id;
    uint8_t *u8v = (uint8_t *)mdata->value;
    uint16_t u16 = strlen((char *)u8v);
    size_t u8vlen = 2 + u16;
    if (prop_id) u8vlen++;

    uint8_t *u8v0 = malloc(u8vlen); // TODO: err checks on malloc

    mdata->ualloc = true;
    mdata->u8v0 = u8v0;
    mdata->u8vlen = u8vlen;

    uint8_t *pu8 = u8v0;
    if (prop_id) *pu8++ = prop_id;
    *pu8++ = (u16 >> 8) & 0xFF;
    *pu8++ = u16 & 0xFF;
    memcpy(pu8, u8v, u16);

    return 0;
}

static int mr_unpack_str(packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t *u8v = pctx->u8v0 + pctx->pos;
    uint16_t u16v[] = {u8v[0], u8v[1]}; u8v += 2;
    size_t vlen = (u16v[0] << 8) + u16v[1];

    uint8_t *pu8 = malloc(vlen + 1);

    memcpy(pu8, u8v, vlen);
    pu8[vlen] = 0;
    mdata->value = (Word_t)pu8;
    mdata->vlen = vlen;
    mdata->vexists = true;
    mdata->valloc = true;
    pctx->pos += 2 + vlen;
    return 0;
}

static int mr_pack_spv(packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t prop_id = mdata->id; // string_pair vectors are always properties
    string_pair *spv = (string_pair *)mdata->value;
    size_t u8vlen = 0;

    for (int i = 0; i < mdata->vlen; i++) {
        u8vlen += 1 + 2 + strlen((char *)spv[i].name) + 2 + strlen((char *)spv[i].value);
    }

    uint8_t *u8v0 = malloc(u8vlen); // TODO: err checks on malloc

    mdata->ualloc = true;
    mdata->u8v0 = u8v0;
    mdata->u8vlen = u8vlen;
    uint8_t *pu8 = u8v0;
    uint16_t u16;

    for (int i = 0; i < mdata->vlen; i++) {
        *pu8++ = prop_id;
        // name
        u16 = strlen((char *)spv[i].name);
        *pu8++ = (u16 >> 8) & 0xFF;
        *pu8++ = u16 & 0xFF;
        memcpy(pu8, spv[i].name, u16);
        pu8 += u16;
        // value
        u16 = strlen((char *)spv[i].value);
        *pu8++ = (u16 >> 8) & 0xFF;
        *pu8++ = u16 & 0xFF;
        memcpy(pu8, spv[i].value, u16);
        pu8 += u16;
    }

    return 0;
}

static int mr_unpack_spv(packet_ctx *pctx, mr_mdata *mdata) { // multiple
    uint8_t *u8v = pctx->u8v0 + pctx->pos;

    uint16_t u16v[] = {u8v[0], u8v[1]}; u8v += 2;
    size_t name_len = (u16v[0] << 8) + u16v[1];
    uint8_t *name = malloc(name_len + 1);
    memcpy(name, u8v, name_len); u8v += name_len;
    name[name_len] = 0;

    u16v[0] = u8v[0]; u16v[1] = u8v[1]; u8v += 2;
    size_t value_len = (u16v[0] << 8) + u16v[1];
    uint8_t *value = malloc(value_len + 1);
    memcpy(value, u8v, value_len); u8v += value_len;
    value[value_len] = 0;

    string_pair *spv0, *psp;
    if (mdata->valloc) {
        spv0 = (string_pair *)mdata->value;
        psp = realloc(spv0, (mdata->vlen + 1) * sizeof(string_pair));
        spv0 = psp;
    }
    else {
        spv0 = malloc(sizeof(string_pair));
        mdata->vlen = 0;
    }

    mdata->value = (Word_t)spv0;
    psp = spv0 + mdata->vlen;
    psp->name = name;
    psp->value = value;
    mdata->vlen++;
    mdata->vexists = true;
    mdata->valloc = true;
    pctx->pos += 2 + name_len + 2 + value_len;
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
    size_t end_pos = pctx->pos + (mdata - 1)->value; // use property_length
    uint8_t *pu8, *pprop_index;
    int prop_index, rc = 0;
    mr_mdata *prop_mdata;
    mr_mdata_fn unpack_fn;

    for (; pctx->pos < end_pos;) {
        pu8 = pctx->u8v0 + pctx->pos++;
        pprop_index = memchr((uint8_t *)mdata->value, *pu8, mdata->vlen);
        if (!pprop_index) return -1; // not found
        prop_index = pprop_index - (uint8_t *)mdata->value;
        prop_mdata = mdata + prop_index + 1;
        unpack_fn = DTYPE_MDATA0[prop_mdata->dtype].unpack_fn;

        if (unpack_fn) {
            rc = unpack_fn(pctx, prop_mdata); // increments pctx->pos
            if (rc) break;
        }
    }

    return rc;
}

static int mr_pack_u8v(packet_ctx *pctx, mr_mdata *mdata) {
    size_t u8vlen = 2 + mdata->vlen;
    uint8_t prop_id = mdata->id;
    if (prop_id) u8vlen++;

    uint8_t *u8v0 = malloc(mdata->u8vlen); // TODO: err checks on malloc

    mdata->ualloc = true;
    mdata->u8v0 = u8v0;
    mdata->u8vlen = u8vlen;

    uint8_t *pu8 = u8v0;
    if (prop_id) *pu8++ = prop_id;
    uint16_t u16 = mdata->vlen;
    *pu8++ = (u16 >> 8) & 0xFF;
    *pu8++ = u16 & 0xFF;
    memcpy(pu8, (uint8_t *)mdata->value, mdata->vlen);
    return 0;
}

static int mr_unpack_u8v(packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t *u8v = pctx->u8v0 + pctx->pos;
    uint16_t u16v[] = {u8v[0], u8v[1]}; u8v += 2;
    size_t vlen = (u16v[0] << 8) + u16v[1];

    uint8_t *value = malloc(vlen);

    memcpy(value, u8v, vlen);
    mdata->value = (Word_t)value;
    mdata->vlen = vlen;
    mdata->vexists = true;
    mdata->valloc = true;
    pctx->pos += 2 + vlen;
    return 0;
}

static int mr_pack_u8vf(packet_ctx *pctx, mr_mdata *mdata) { // do not allocate
    mdata->u8vlen = mdata->vlen; // fixed length set by template
    mdata->u8v0 = (uint8_t *)mdata->value;
    return 0;
}

static int mr_unpack_u8vf(packet_ctx *pctx, mr_mdata *mdata) { // do not allocate
    mdata->vexists = true;
    mdata->u8vlen = mdata->vlen; // fixed length set by template
    mdata->value = (Word_t)(pctx->u8v0 + pctx->pos);
    pctx->pos += mdata->u8vlen;
    return 0;
}

//  idx is vlen: # of bits to be (re)set
const uint8_t BIT_MASKS[] = {
    0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};

//  get the link mdata byte, reset specified bit(s) and set if value is non-zero
static int mr_pack_bits(packet_ctx *pctx, mr_mdata *mdata) {
    uint8_t bitpos = mdata->id;
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
    uint8_t bitpos = mdata->id;
    uint8_t *pu8 = pctx->u8v0 + pctx->pos;
    mdata->value = *pu8 >> bitpos & BIT_MASKS[mdata->vlen];
    mdata->vexists = true;
    return 0;
}

static int mr_unpack_incr1(packet_ctx *pctx, mr_mdata *mdata) { // now advance; bits all unpacked
    mdata->vexists = true;
    mdata->vlen = 1;
    pctx->pos++;
    return 0;
}
