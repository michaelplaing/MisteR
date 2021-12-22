/* packet.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "packet_internal.h"

int mr_set_scalar(packet_ctx *pctx, int index, Word_t value) {
    mr_mdata *mdata = pctx->mdata0 + index;
    mdata->value = value;
    mdata->exists = true;
    return 0;
}

static int mr_get_scalar(packet_ctx *pctx, int index, Word_t *pvalue) {
    int rc;
    mr_mdata *mdata = pctx->mdata0 + index;

    if (mdata->exists) {
        *pvalue = mdata->value;
        rc = 0;
    }
    else {
        rc = -1;
    }

    return rc;
}

int mr_get_boolean(packet_ctx *pctx, int index, bool *pboolean) {
    Word_t value;
    int rc = mr_get_scalar(pctx, index, &value);
    if (!rc) *pboolean = (bool)value;
    return rc;
}

int mr_get_u8(packet_ctx *pctx, int index, uint8_t *pu8) {
    Word_t value;
    int rc = mr_get_scalar(pctx, index, &value);
    if (!rc) *pu8 = (uint8_t)value;
    return rc;
}

int mr_get_u16(packet_ctx *pctx, int index, uint16_t *pu16) {
    Word_t value;
    int rc = mr_get_scalar(pctx, index, &value);
    if (!rc) *pu16 = (uint16_t)value;
    return rc;
}

int mr_get_u32(packet_ctx *pctx, int index, uint32_t *pu32) {
    Word_t value;
    int rc = mr_get_scalar(pctx, index, &value);
    if (!rc) *pu32 = (uint32_t)value;
    return rc;
}

int mr_set_vector(packet_ctx *pctx, int index, void *pvoid, size_t len) {
    mr_mdata *mdata = pctx->mdata0 + index;
    mdata->value = (Word_t)pvoid;
    mdata->exists = true;
    mdata->vlen = len;
    return 0;
}

static int mr_get_vector(packet_ctx *pctx, int index, Word_t *ppvoid, size_t *plen) {
    int rc;
    mr_mdata *mdata = pctx->mdata0 + index;

    if (mdata->exists) {
        *ppvoid = mdata->value; // for a vector, value is a pointer to something
        *plen = mdata->vlen;
        rc = 0;
    }
    else {
        rc = -1;
    }

    return rc;
}

int mr_get_u8v(packet_ctx *pctx, int index, uint8_t **pu8v0, size_t *plen) {
    Word_t pvoid;
    size_t len;
    int rc = mr_get_vector(pctx, index, &pvoid, &len);

    if (!rc) {
        *pu8v0 = (uint8_t *)pvoid;
        *plen = len;
    }

    return rc;
}

int mr_get_spv(packet_ctx *pctx, int index, string_pair **pspv0, size_t *plen) {
    Word_t pvoid;
    size_t len;
    int rc = mr_get_vector(pctx, index, &pvoid, &len);

    if (!rc) {
        *pspv0 = (string_pair *)pvoid;
        *plen = len;
    }

    return rc;
}

int mr_reset_value(packet_ctx *pctx, int index) {
    mr_mdata *mdata = pctx->mdata0 + index;
    mdata->value = 0;
    mdata->exists = false;
    mdata->vlen = 0;
    return 0;
}

//  pack each mdata in reverse order of the table
//  into its allocated buffer using its packing function
//  then allocate pctx->u8v0 and catenate mdata buffers in it
//  free each mdata buffer if allocated
int mr_pack_mdata_u8v0(packet_ctx *pctx) {
    mr_mdata *mdata = pctx->mdata0 + pctx->mdata_count - 1;

    for (int i = pctx->mdata_count - 1; i > -1; mdata--, i--) {
        mdata->pack_fn(pctx, mdata);
    }

    mdata = pctx->mdata0 + 1; // <CONTROL_PACKET_NAME>_REMAINING_LENGTH is always index 1
    pctx->len = mdata->value + mdata->u8vlen + 1;

    uint8_t *u8v0 = malloc(pctx->len); // TODO: err checks on malloc

    pctx->u8v0 = u8v0;
    pctx->isalloc = true;

    mdata = pctx->mdata0;
    uint8_t *pu8 = u8v0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        memcpy(pu8, mdata->u8v0, mdata->u8vlen);
        pu8 += mdata->u8vlen;

        if (mdata->isalloc) {
            free(mdata->u8v0);
            mdata->isalloc = false;
        }
    }

    return 0;
}

int mr_unpack_mdata_u8v0(packet_ctx *pctx) {
    mr_mdata *mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        if (mdata->unpack_fn) mdata->unpack_fn(pctx, mdata);
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
        if (u32) *pu8 = *pu8 | 0x80;
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
int mr_pack_VBI(packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    size_t cum_len = 0;
    mr_mdata *current_mdata;

    //  accumulate buffer lengths in cum_len for the range of the VBI
    for (int j = mdata->index + 1; j <= mdata->link; j++) {
        current_mdata = pctx->mdata0 + j;
        if (current_mdata->exists) cum_len += current_mdata->u8vlen;
    }

    mdata->value = cum_len;
    uint32_t u32 = mdata->value; // TODO: err if too large for 4 bytes
    uint8_t u8v[5]; // enough for uint32_t even if too big
    mdata->u8vlen = mr_make_VBI(u32, u8v);

    uint8_t *u8v0 = malloc(mdata->u8vlen); // TODO: err checks on malloc

    mdata->isalloc = true;
    mdata->u8v0 = u8v0;
    memcpy(mdata->u8v0, u8v, mdata->u8vlen);
    return 0;
}

int mr_unpack_VBI(packet_ctx *pctx, mr_mdata *mdata) {
    uint32_t u32;
    int rc = mr_get_VBI(&u32, pctx->u8v0 + pctx->pos);

    if (rc > 0) {
        mdata->value = u32;
        mdata->vlen = rc;
        pctx->pos += rc;
        rc = 0;
    }

    return rc;
 }

int mr_free_packet_context(packet_ctx *pctx) {
    mr_mdata *mdata;
    Word_t bytes_freed;

    //  free mdata buffers
    for (int i = 0; i < pctx->mdata_count; i++) {
        mdata = pctx->mdata0 + i;
        if (mdata->isalloc) free(mdata->u8v0);
    }

    //  free packet buffer
    if (pctx->isalloc) free(pctx->u8v0);

    // free pack context
    free(pctx);

    return 0;
}

int mr_init_packet_context(packet_ctx **ppctx, const mr_mdata *MDATA_TEMPLATE, size_t mdata_count) {
    packet_ctx *pctx = calloc(1, sizeof(packet_ctx));
    pctx->mdata_count = mdata_count;
    mr_mdata *mdata0 = calloc(pctx->mdata_count, sizeof(mr_mdata));
    //  copy template
    for (int i = 0; i < pctx->mdata_count; i++) mdata0[i] = MDATA_TEMPLATE[i];
    pctx->mdata0 = mdata0;
    *ppctx = pctx;
    return 0;
}

int mr_pack_u8(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->u8vlen = 1;
    uint8_t *u8v0 = malloc(mdata->u8vlen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->u8v0 = u8v0;
    *u8v0 = mdata->value;
    return 0;
}

int mr_unpack_u8(packet_ctx *pctx, mr_mdata *mdata) {
    //if (!pctx->isalloc || pctx->pos >= pctx->len) return 0;
    mdata->exists = true;
    mdata->vlen = 1;
    mdata->value = pctx->u8v0[pctx->pos++];
    return 0;
}

int mr_pack_u16(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->u8vlen = 2;
    uint8_t *u8v0 = malloc(mdata->u8vlen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->u8v0 = u8v0;
    uint16_t u16 = mdata->value;
    uint8_t *pu8 = u8v0;
    *pu8++ = (u16 >> 8) & 0xFF;
    *pu8 = u16 & 0xFF;
    return 0;
}

int mr_unpack_u16(packet_ctx *pctx, mr_mdata *mdata) {
    //if (!pctx->isalloc || pctx->pos >= pctx->len) return 0;
    mdata->vlen = 2;
    mdata->exists = true;
    uint8_t *u8v = pctx->u8v0 + pctx->pos;
    uint16_t u16v[] = {u8v[0], u8v[1]};
    mdata->value = (u16v[0] << 8) + u16v[1];
    pctx->pos += 2;
    return 0;
}

int mr_pack_u32(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->u8vlen = 4;
    uint8_t *u8v0 = malloc(mdata->u8vlen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->u8v0 = u8v0;
    uint32_t u32 = mdata->value;
    uint8_t *pu8 = u8v0;
    *pu8++ = (u32 >> 24) & 0xFF;
    *pu8++ = (u32 >> 16) & 0xFF;
    *pu8++ = (u32 >> 8) & 0xFF;
    *pu8 = u32 & 0xFF;
    return 0;
}

int mr_unpack_u32(packet_ctx *pctx, mr_mdata *mdata) {
    //if (!pctx->isalloc || pctx->pos >= pctx->len) return 0;
    mdata->vlen = 4;
    uint8_t *u8v = pctx->u8v0 + pctx->pos;
    uint32_t u32v[] = {u8v[0], u8v[1], u8v[2], u8v[3]};
    mdata->value = (u32v[0] << 24) + (u32v[1] << 16) + (u32v[2] << 8) + u32v[3];
    pctx->pos += 4;
    return 0;
}

int mr_pack_prop_u8(packet_ctx *pctx, mr_mdata *mdata) {
    if (mdata->exists) {
        mdata->u8vlen = 2;
        uint8_t *u8v0 = malloc(mdata->u8vlen); // TODO: err checks on malloc
        mdata->isalloc = true;
        mdata->u8v0 = u8v0;
        uint8_t *pu8 = u8v0;
        *pu8++ = mdata->id;
        *pu8 = mdata->value;
    }

    return 0;
}

int mr_pack_prop_u16(packet_ctx *pctx, mr_mdata *mdata) {
    if (mdata->exists) {
        mdata->u8vlen = 3;
        uint8_t *u8v0 = malloc(mdata->u8vlen); // TODO: err checks on malloc
        mdata->isalloc = true;
        mdata->u8v0 = u8v0;
        uint16_t u16 = mdata->value;
        uint8_t *pu8 = u8v0;
        *pu8++ = mdata->id;
        *pu8++ = (u16 >> 8) & 0xFF;
        *pu8 = u16 & 0xFF;
    }

    return 0;
}

int mr_pack_prop_u32(packet_ctx *pctx, mr_mdata *mdata) {
    if (mdata->exists) {
        mdata->u8vlen = 5;
        uint8_t *u8v0 = malloc(mdata->u8vlen); // TODO: err checks on malloc
        mdata->isalloc = true;
        mdata->u8v0 = u8v0;
        uint32_t u32 = mdata->value;
        uint8_t *pu8 = u8v0;
        *pu8++ = mdata->id;
        *pu8++ = (u32 >> 24) & 0xFF;
        *pu8++ = (u32 >> 16) & 0xFF;
        *pu8++ = (u32 >> 8) & 0xFF;
        *pu8 = u32 & 0xFF;
    }

    return 0;
}

int mr_pack_str(packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    uint8_t *u8v = (uint8_t *)mdata->value;
    uint16_t u16 = strlen((char *)u8v); // get number of bytes (not chars)
    size_t u8vlen = 2 + u16;

    uint8_t *u8v0 = malloc(u8vlen); // TODO: err checks on malloc

    mdata->isalloc = true;
    mdata->u8v0 = u8v0;
    mdata->u8vlen = u8vlen;

    uint8_t *pu8 = u8v0;
    *pu8++ = (u16 >> 8) & 0xFF;
    *pu8++ = u16 & 0xFF;
    memcpy(pu8, u8v, u16);

    return 0;
}

int mr_pack_prop_str(packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    uint8_t *u8v = (uint8_t *)mdata->value;
    uint16_t u16 = strlen((char *)u8v); // get number of bytes (not chars)
    size_t u8vlen = 1 + 2 + u16;

    uint8_t *u8v0 = malloc(u8vlen); // TODO: err checks on malloc

    mdata->isalloc = true;
    mdata->u8v0 = u8v0;
    mdata->u8vlen = u8vlen;

    uint8_t *pu8 = u8v0;
    *pu8++ = mdata->id; // <- set id
    *pu8++ = (u16 >> 8) & 0xFF;
    *pu8++ = u16 & 0xFF;
    memcpy(pu8, u8v, u16);

    return 0;
}

int mr_pack_prop_u8v(packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    size_t u8vlen = 1 + mdata->vlen;

    uint8_t *u8v0 = malloc(u8vlen); // TODO: err checks on malloc

    mdata->isalloc = true;
    mdata->u8v0 = u8v0;
    mdata->u8vlen = u8vlen;

    uint8_t *pu8 = u8v0;
    *pu8++ = mdata->id;
    memcpy(pu8, (uint8_t *)mdata->value, mdata->vlen);

    return 0;
}

//  for property vectors, e.g. connect_payload:will_properties:user_properties
int mr_pack_prop_spv(packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    string_pair *spv = (string_pair *)mdata->value;
    size_t u8vlen = 0;

    for (int i = 0; i < mdata->vlen; i++) {
        u8vlen += 1 + 2 + strlen((char *)spv[i].name) + 2 + strlen((char *)spv[i].value);
    }

    uint8_t *u8v0 = malloc(u8vlen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->u8v0 = u8v0;
    mdata->u8vlen = u8vlen;
    uint8_t *pu8 = u8v0;
    uint16_t u16;

    for (int i = 0; i < mdata->vlen; i++) {
        *pu8++ = mdata->id;
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

int mr_pack_u8v(packet_ctx *pctx, mr_mdata *mdata) { // do not allocate
    if (mdata->exists) {
        mdata->u8vlen = mdata->vlen;
        mdata->u8v0 = (uint8_t *)mdata->value;
    }

    return 0;
}

int mr_unpack_u8v(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->exists = true;
    mdata->vlen = mdata->u8vlen;
    mdata->value = (Word_t)(mdata->u8v0 + pctx->pos);
    pctx->pos += mdata->u8vlen;
    return 0;
}

//  index is vlen: # of bits to be (re)set
const uint8_t BIT_MASKS[] = {
    0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};

//  get the link mdata byte, reset specified bit(s) and set if value is non-zero
int mr_pack_bits(packet_ctx *pctx, mr_mdata *mdata) {
    mr_mdata *link_mdata = pctx->mdata0 + mdata->link;
    *link_mdata->u8v0 = *link_mdata->u8v0 & ~(BIT_MASKS[mdata->vlen] << mdata->bitpos);

    if (mdata->value) {
        uint8_t u8 = mdata->value;
        *link_mdata->u8v0 = *link_mdata->u8v0 | (u8 << mdata->bitpos);
    }

    return 0;
}

int mr_unpack_bits(packet_ctx *pctx, mr_mdata *mdata) { // don't advance pctx->pos
    mdata->exists = true;
    mdata->value = pctx->u8v0[pctx->pos] >> mdata->bitpos & BIT_MASKS[mdata->vlen];
    return 0;
}
