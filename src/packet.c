/* packet.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "packet_internal.h"

int set_scalar_value(packet_ctx *pctx, int index, Word_t value) {
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

int set_vector_value(packet_ctx *pctx, int index, Word_t value, size_t len) {
    mr_mdata *mdata = pctx->mdata0 + index;
    mdata->value = value;
    mdata->exists = true;
    mdata->vlen = len;
    return 0;
}

int get_vector_value(packet_ctx *pctx, int index, Word_t *Pvalue, size_t *Plen) {
    int rc;
    mr_mdata *mdata = pctx->mdata0 + index;
    
    if (mdata->exists) {
        *Pvalue = mdata->value; // for a vector, value is some sort of pointer
        *Plen = mdata->vlen;
        rc = 0;
    }
    else {
        rc = -1;
    }

    return rc;
}

int reset_header_value(packet_ctx *pctx, int index) {
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
    printf("pack_mdata_buffer\n");
    mr_mdata *mdata;
    printf("pack each mdata: mdata_count: %lu\n", pctx->mdata_count);
    
    for (int i = pctx->mdata_count - 1; i > -1; i--) {
        mdata = pctx->mdata0 + i;
        mdata->pack_fn(pctx, mdata);
    }
    
    mdata = pctx->mdata0 + 1; // remaining_length
    printf("malloc pctx->buf using from remaining_length: blen: %lu + value: %lu + 1\n", mdata->blen, mdata->value);
    pctx->len = mdata->value + mdata->blen + 1;
    
    uint8_t *buf = malloc(pctx->len); // TODO: err checks on malloc
    
    pctx->buf = buf;
    pctx->isalloc = true;
    
    printf("memcpy each mdata buf into pctx->buf, then free it\n");
    uint8_t *tbuf = buf;
    for (int i = 0; i < pctx->mdata_count; i++) {
        mdata = pctx->mdata0 + i;
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
    printf("unpack_mdata_buffer\n");
    mr_mdata *mdata;
    printf("unpack each mdata: mdata_count: %lu\n", pctx->mdata_count);
    
    for (int i = 0; i < pctx->mdata_count; i++) {
        mdata = pctx->mdata0 + i;
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

    printf("pack_VBI: %s\n", mdata->name);
    size_t cum_len = 0;
    mr_mdata *current_mdata;

    printf("accumulate buffer lengths: %u to %u\n", mdata->index + 1, mdata->link);
    //  accumulate buffer lengths in cum_len for the range of the VBI
    for (int j = mdata->index + 1; j <= mdata->link; j++) {
        current_mdata = pctx->mdata0 + j;
        if (current_mdata->exists) cum_len += current_mdata->blen;
    }
    printf("  cum_len: %lu\n", cum_len);
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

int free_packet_context(packet_ctx *pctx) {
    printf("free_packet_context\n");
    mr_mdata *mdata;
    Word_t bytes_freed;

    //  free mdata buffers
    for (int i = 0; i < pctx->mdata_count; i++) {
        mdata = pctx->mdata0 + i;
        if (mdata->isalloc) free(mdata->buf);
    }

    //  free packet buffer - don't rely on NULL pointer...
    if (pctx->isalloc) free(pctx->buf);

    // free pack context
    free(pctx);

    return 0;
}

packet_ctx *init_packet_context(const mr_mdata *MDATA_TEMPLATE, size_t mdata_count) {
    printf("init_packet_context\n");
    packet_ctx *pctx = calloc(1, sizeof(packet_ctx));
    pctx->mdata_count = mdata_count; //sizeof(MDATA_TEMPLATE) / sizeof(mr_mdata);
    mr_mdata *mdata0 = calloc(pctx->mdata_count, sizeof(mr_mdata));
    printf("copy template\n");
    //  copy template
    for (int i = 0; i < pctx->mdata_count; i++) mdata0[i] = MDATA_TEMPLATE[i];
    pctx->mdata0 = mdata0;
    return pctx;
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
    printf("value: %lu; vlen: %lu; pos: %lu\n", mdata->value, mdata->vlen, pctx->pos);
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
    printf("value: %lu; vlen: %lu; pos: %lu\n", mdata->value, mdata->vlen, pctx->pos);
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
