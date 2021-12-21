/* packet.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "packet_internal.h"

int set_scalar(packet_ctx *pctx, int index, Word_t value) {
    mr_mdata *mdata = pctx->mdata0 + index;
    mdata->value = value;
    mdata->exists = true;
    return 0;
}

int get_scalar_value(packet_ctx *pctx, int index, Word_t *Pvalue) {
    int rc;
    mr_mdata *mdata = pctx->mdata0 + index;

    if (mdata->exists) {
        *Pvalue = mdata->value;
        rc = 0;
    }
    else {
        rc = -1;
    }

    return rc;
}

int get_boolean_scalar(packet_ctx *pctx, int index, bool *Pboolean) {
    Word_t value;
    int rc = get_scalar_value(pctx, index, &value);
    if (!rc) *Pboolean = (bool)value;
    return rc;
}

int get_uint8_scalar(packet_ctx *pctx, int index, uint8_t *Puint8) {
    Word_t value;
    int rc = get_scalar_value(pctx, index, &value);
    if (!rc) *Puint8 = (uint8_t)value;
    return rc;
}

int get_uint16_scalar(packet_ctx *pctx, int index, uint16_t *Puint16) {
    Word_t value;
    int rc = get_scalar_value(pctx, index, &value);
    if (!rc) *Puint16 = (uint16_t)value;
    return rc;
}

int get_uint32_scalar(packet_ctx *pctx, int index, uint32_t *Puint32) {
    Word_t value;
    int rc = get_scalar_value(pctx, index, &value);
    if (!rc) *Puint32 = (uint32_t)value;
    return rc;
}

int set_vector(packet_ctx *pctx, int index, void *pointer, size_t len) {
    mr_mdata *mdata = pctx->mdata0 + index;
    mdata->value = (Word_t)pointer;
    mdata->exists = true;
    mdata->vlen = len;
    return 0;
}

int get_vector(packet_ctx *pctx, int index, Word_t *Ppointer, size_t *Plen) {
    int rc;
    mr_mdata *mdata = pctx->mdata0 + index;

    if (mdata->exists) {
        *Ppointer = mdata->value; // for a vector, value is some sort of pointer
        *Plen = mdata->vlen;
        rc = 0;
    }
    else {
        rc = -1;
    }

    return rc;
}

int get_uint8_vector(packet_ctx *pctx, int index, uint8_t **Puint80, size_t *Plen) {
    Word_t pointer;
    size_t len;
    int rc = get_vector(pctx, index, &pointer, &len);

    if (!rc) {
        *Puint80 = (uint8_t *)pointer;
        *Plen = len;
    }

    return rc;
}

int get_string_pair_vector(packet_ctx *pctx, int index, string_pair **Psp0, size_t *Plen) {
    Word_t pointer;
    size_t len;
    int rc = get_vector(pctx, index, &pointer, &len);

    if (!rc) {
        *Psp0 = (string_pair *)pointer;
        *Plen = len;
    }

    return rc;
}

int reset_value(packet_ctx *pctx, int index) {
    mr_mdata *mdata = pctx->mdata0 + index;
    mdata->value = 0;
    mdata->exists = false;
    mdata->vlen = 0;
    return 0;
}

//  pack each mdata in reverse order of the table
//  into its allocated buffer using its packing function
//  then allocate pctx->buf and catenate mdata buffers in it
//  free each mdata buffer if allocated
int pack_mdata_buffer(packet_ctx *pctx) {
    mr_mdata *mdata = pctx->mdata0 + pctx->mdata_count - 1;

    for (int i = pctx->mdata_count - 1; i > -1; mdata--, i--) {
        mdata->pack_fn(pctx, mdata);
    }

    mdata = pctx->mdata0 + 1; // <CONTROL_PACKET_NAME>_REMAINING_LENGTH is always index 1
    pctx->len = mdata->value + mdata->blen + 1;

    uint8_t *buf = malloc(pctx->len); // TODO: err checks on malloc

    pctx->buf = buf;
    pctx->isalloc = true;

    mdata = pctx->mdata0;
    uint8_t *tbuf = buf;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        memcpy(tbuf, mdata->buf, mdata->blen);
        tbuf += mdata->blen;

        if (mdata->isalloc) {
            free(mdata->buf);
            mdata->isalloc = false;
        }
    }

    return 0;
}

int unpack_mdata_buffer(packet_ctx *pctx) {
    mr_mdata *mdata = pctx->mdata0;
    for (int i = 0; i < pctx->mdata_count; mdata++, i++) {
        if (mdata->unpack_fn) mdata->unpack_fn(pctx, mdata);
    }

    return 0;
}

int make_VBI(uint32_t val32, uint8_t *buf) {
    if (val32 >> (7 * 4)) { // overflow: too big for 4 bytes
        return -1;
    }

    int i = 0;
    do {
        *buf = val32 & 0x7F;
        val32 = val32 >> 7;
        if (val32) *buf = *buf | 0x80;
        i++; buf++;
    } while (val32);

    return i;
}

int get_VBI(uint32_t *Pval32, uint8_t *buf) {
    uint32_t val32, res32 = 0;
    int i;
    for (i = 0; i < 4; buf++, i++){
        val32 = *buf;
        res32 += (val32 & 0x7F) << (7 * i);
        if (!(*buf & 0x80)) break;
    }

    if (i == 4) { // overflow: byte[3] has a continuation bit
        return -1;
    }
    else {
        *Pval32 = res32;
        return i + 1;
    }
}

//  calculate length, convert to VBI, & pack into buffer
int pack_VBI(packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    size_t cum_len = 0;
    mr_mdata *current_mdata;

    //  accumulate buffer lengths in cum_len for the range of the VBI
    for (int j = mdata->index + 1; j <= mdata->link; j++) {
        current_mdata = pctx->mdata0 + j;
        if (current_mdata->exists) cum_len += current_mdata->blen;
    }

    mdata->value = cum_len;
    uint32_t tmp_val32 = mdata->value; // TODO: err if too large for 4 bytes
    uint8_t tmp_buf[5]; // enough for uint32_t even if too big
    mdata->blen = make_VBI(tmp_val32, tmp_buf);

    uint8_t *buf = malloc(mdata->blen); // TODO: err checks on malloc

    mdata->isalloc = true;
    mdata->buf = buf;
    memcpy(mdata->buf, tmp_buf, mdata->blen);
    return 0;
}

int unpack_VBI(packet_ctx *pctx, mr_mdata *mdata) {
    uint32_t val32;
    int rc = get_VBI(&val32, pctx->buf + pctx->pos);

    if (rc > 0) {
        mdata->value = val32;
        mdata->vlen = rc;
        pctx->pos += rc;
        rc = 0;
    }

    return rc;
 }

int free_packet_context(packet_ctx *pctx) {
    mr_mdata *mdata;
    Word_t bytes_freed;

    //  free mdata buffers
    for (int i = 0; i < pctx->mdata_count; i++) {
        mdata = pctx->mdata0 + i;
        if (mdata->isalloc) free(mdata->buf);
    }

    //  free packet buffer
    if (pctx->isalloc) free(pctx->buf);

    // free pack context
    free(pctx);

    return 0;
}

int init_packet_context(packet_ctx **Ppctx, const mr_mdata *MDATA_TEMPLATE, size_t mdata_count) {
    packet_ctx *pctx = calloc(1, sizeof(packet_ctx));
    pctx->mdata_count = mdata_count;
    mr_mdata *mdata0 = calloc(pctx->mdata_count, sizeof(mr_mdata));
    //  copy template
    for (int i = 0; i < pctx->mdata_count; i++) mdata0[i] = MDATA_TEMPLATE[i];
    pctx->mdata0 = mdata0;
    *Ppctx = pctx;
    return 0;
}

int pack_uint8(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->blen = 1;
    uint8_t *buf = malloc(mdata->blen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->buf = buf;
    *buf = mdata->value;
    return 0;
}

int unpack_uint8(packet_ctx *pctx, mr_mdata *mdata) {
    //if (!pctx->isalloc || pctx->pos >= pctx->len) return 0;
    mdata->vlen = 1;
    mdata->value = pctx->buf[pctx->pos++];
    return 0;
}

int pack_uint16(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->blen = 2;
    uint8_t *buf = malloc(mdata->blen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->buf = buf;
    uint16_t val16 = mdata->value;
    uint8_t *tbuf = buf;
    *tbuf++ = (val16 >> 8) & 0xFF;
    *tbuf = val16 & 0xFF;
    return 0;
}

int unpack_uint16(packet_ctx *pctx, mr_mdata *mdata) {
    //if (!pctx->isalloc || pctx->pos >= pctx->len) return 0;
    mdata->vlen = 2;
    uint8_t *tbuf = pctx->buf + pctx->pos;
    uint16_t val16s[] = {tbuf[0], tbuf[1]};
    mdata->value = (val16s[0] << 8) + val16s[1];
    pctx->pos += 2;
    return 0;
}

int pack_uint32(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->blen = 4;
    uint8_t *buf = malloc(mdata->blen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->buf = buf;
    uint32_t val32 = mdata->value;
    uint8_t *tbuf = buf;
    *tbuf++ = (val32 >> 24) & 0xFF;
    *tbuf++ = (val32 >> 16) & 0xFF;
    *tbuf++ = (val32 >> 8) & 0xFF;
    *tbuf = val32 & 0xFF;
    return 0;
}

int pack_sprop_uint8(packet_ctx *pctx, mr_mdata *mdata) {
    if (mdata->exists) {
        mdata->blen = 2;
        uint8_t *buf = malloc(mdata->blen); // TODO: err checks on malloc
        mdata->isalloc = true;
        mdata->buf = buf;
        uint8_t *tbuf = buf;
        *tbuf++ = mdata->id;
        *tbuf = mdata->value;
    }

    return 0;
}

int pack_sprop_uint16(packet_ctx *pctx, mr_mdata *mdata) {
    if (mdata->exists) {
        mdata->blen = 3;
        uint8_t *buf = malloc(mdata->blen); // TODO: err checks on malloc
        mdata->isalloc = true;
        mdata->buf = buf;
        uint16_t val16 = mdata->value;
        uint8_t *tbuf = buf;
        *tbuf++ = mdata->id;
        *tbuf++ = (val16 >> 8) & 0xFF;
        *tbuf = val16 & 0xFF;
    }

    return 0;
}

int pack_sprop_uint32(packet_ctx *pctx, mr_mdata *mdata) {
    if (mdata->exists) {
        mdata->blen = 5;
        uint8_t *buf = malloc(mdata->blen); // TODO: err checks on malloc
        mdata->isalloc = true;
        mdata->buf = buf;
        uint32_t val32 = mdata->value;
        uint8_t *tbuf = buf;
        *tbuf++ = mdata->id;
        *tbuf++ = (val32 >> 24) & 0xFF;
        *tbuf++ = (val32 >> 16) & 0xFF;
        *tbuf++ = (val32 >> 8) & 0xFF;
        *tbuf = val32 & 0xFF;
    }

    return 0;
}

int pack_str(packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    uint8_t *strval = (uint8_t *)mdata->value;
    uint16_t val16 = strlen((char *)strval); // get number of bytes (not chars)
    size_t blen = 2 + val16;

    uint8_t *buf = malloc(blen); // TODO: err checks on malloc

    mdata->isalloc = true;
    mdata->buf = buf;
    mdata->blen = blen;

    uint8_t *tbuf = buf;
    *tbuf++ = (val16 >> 8) & 0xFF;
    *tbuf++ = val16 & 0xFF;
    memcpy(tbuf, strval, val16);

    return 0;
}

int pack_sprop_str(packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    uint8_t *strval = (uint8_t *)mdata->value;
    uint16_t val16 = strlen((char *)strval); // get number of bytes (not chars)
    size_t blen = 1 + 2 + val16;

    uint8_t *buf = malloc(blen); // TODO: err checks on malloc

    mdata->isalloc = true;
    mdata->buf = buf;
    mdata->blen = blen;

    uint8_t *tbuf = buf;
    *tbuf++ = mdata->id;
    *tbuf++ = (val16 >> 8) & 0xFF;
    *tbuf++ = val16 & 0xFF;
    memcpy(tbuf, strval, val16);

    return 0;
}

int pack_sprop_char_buf(packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    size_t blen = 1 + mdata->vlen;

    uint8_t *buf = malloc(blen); // TODO: err checks on malloc

    mdata->isalloc = true;
    mdata->buf = buf;
    mdata->blen = blen;

    uint8_t *tbuf = buf;
    *tbuf++ = mdata->id;
    memcpy(tbuf, (uint8_t *)mdata->value, mdata->vlen);

    return 0;
}

//  for property vectors, e.g. connect_payload:will_properties:user_properties
//  mdata->value should be a *string_pair
int pack_mprop_strpair(packet_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    string_pair *strpair = (string_pair *)mdata->value;
    size_t blen = 0;

    for (int i = 0; i < mdata->vlen; i++) {
        blen += 1 + 2 + strlen((char *)strpair[i].name) + 2 + strlen((char *)strpair[i].value);
    }

    uint8_t *buf = malloc(blen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->buf = buf;
    mdata->blen = blen;
    uint8_t *tbuf = buf;
    uint16_t val16;

    for (int i = 0; i < mdata->vlen; i++) {
        *tbuf++ = mdata->id;
        // name
        val16 = strlen((char *)strpair[i].name);
        *tbuf++ = (val16 >> 8) & 0xFF;
        *tbuf++ = val16 & 0xFF;
        memcpy(tbuf, strpair[i].name, val16); tbuf += val16;
        // value
        val16 = strlen((char *)strpair[i].value);
        *tbuf++ = (val16 >> 8) & 0xFF;
        *tbuf++ = val16 & 0xFF;
        memcpy(tbuf, strpair[i].value, val16); tbuf += val16;
    }

    return 0;
}

int pack_char_buf(packet_ctx *pctx, mr_mdata *mdata) { // do not allocate
    if (mdata->exists) {
        mdata->blen = mdata->vlen;
        mdata->buf = (uint8_t *)mdata->value;
    }

    return 0;
}

int pack_flags_alloc(packet_ctx *pctx, mr_mdata *mdata) {
    mdata->blen = 1;
    uint8_t *buf = calloc(1, mdata->blen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->buf = buf;
    return 0;
};

//  index is vlen: # of bits to be (re)set
const uint8_t BIT_MASKS[] = {
    0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};

//  get the link (parent) mdata, reset bit(s) and set if value is non-zero
int pack_in_parent(packet_ctx *pctx, mr_mdata *mdata) {
    mr_mdata *link_mdata = pctx->mdata0 + mdata->link;
    *link_mdata->buf = *link_mdata->buf & ~(BIT_MASKS[mdata->vlen] << mdata->bitpos);

    if (mdata->value) {
        uint8_t val = mdata->value;
        *link_mdata->buf = *link_mdata->buf | (val << mdata->bitpos);
    }

    return 0;
}
