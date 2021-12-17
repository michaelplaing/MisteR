/* pack.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "pack_internal.h"

int set_scalar_value(pack_ctx *pctx, int index, Word_t value) {
    mr_mdata *mdata = pctx->mdata0 + index;
    mdata->value = value;
    mdata->exists = true;
    return 0;
}

int get_scalar_value(pack_ctx *pctx, int index, Word_t *Pvalue) {
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

int set_vector_value(pack_ctx *pctx, int index, Word_t value, size_t len) {
    mr_mdata *mdata = pctx->mdata0 + index;
    mdata->value = value;
    mdata->exists = true;
    mdata->vlen = len;
    return 0;
}

int get_vector_value(pack_ctx *pctx, int index, Word_t *Pvalue, size_t *Plen) {
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

int reset_header_value(pack_ctx *pctx, int index) {
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
int pack_mdata_buffer(pack_ctx *pctx) {
    printf("pack_mdata_buffer\n");
    mr_mdata *mdata;
    printf("pack each mdata: mdata_count: %lu\n", pctx->mdata_count);
    
    for (int i = pctx->mdata_count - 1; i > -1; i--) {
        mdata = pctx->mdata0 + i;
        mdata->pack_fn(pctx, mdata);
    }
    
    mdata = pctx->mdata0 + 1; // remaining_length
    printf("malloc pctx->buf using from remaining_length: buflen: %lu + value: %lu + 1\n", mdata->buflen, mdata->value);
    pctx->len = mdata->value + mdata->buflen + 1;
    
    uint8_t *buf = malloc(pctx->len); // TODO: err checks on malloc
    
    pctx->buf = buf;
    pctx->isalloc = true;
    
    printf("memcpy each mdata buf into pctx->buf, then free it\n");
    uint8_t *bufpos = buf;
    for (int i = 0; i < pctx->mdata_count; i++) {
        mdata = pctx->mdata0 + i;
        memcpy(bufpos, mdata->buf, mdata->buflen);
        bufpos += mdata->buflen;
        
        if (mdata->isalloc) {
            free(mdata->buf);
            mdata->isalloc = false;
        }
    }

    return 0;
}

//  handle 7 bits (0-6) at a time; use bit 7 as the continution flag
int make_VBI(uint32_t val32, uint8_t *buf) {
    // Note: buf should point to at least 5 allocated bytes
    int i = 0;
    do {
        buf[i] = val32 & 0x7F;
        val32 = val32 >> 7;
        if (val32 > 0) buf[i] = buf[i] | 0x80;
        i++;
    }
    while (val32 > 0);

    return i;
}

//  calculate length, convert to VBI, & pack into buffer
int pack_VBI(pack_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    printf("pack_VBI: %s\n", mdata->name);
    size_t cum_len = 0;
    mr_mdata *current_mdata;

    printf("accumulate buffer lengths: %u to %u\n", mdata->index + 1, mdata->link);
    //  accumulate buffer lengths in cum_len for the range of the VBI
    for (int j = mdata->index + 1; j <= mdata->link; j++) {
        current_mdata = pctx->mdata0 + j;
        if (current_mdata->exists) cum_len += current_mdata->buflen;
    }
    printf("  cum_len: %lu\n", cum_len);
    mdata->value = cum_len;
    uint32_t tmp_val32 = mdata->value; // TODO: err if too large for 4 bytes
    uint8_t tmp_buf[5]; // enough for uint32_t even if too big

    mdata->buflen = make_VBI(tmp_val32, tmp_buf);

    uint8_t *buf = malloc(mdata->buflen); // TODO: err checks on malloc
    
    mdata->isalloc = true;
    mdata->buf = buf;
    memcpy(mdata->buf, tmp_buf, mdata->buflen);
    return 0;
}

int free_pack_context(pack_ctx *pctx) {
    printf("free_pack_context\n");
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

pack_ctx *init_pack_context(const mr_mdata *MDATA_TEMPLATE, size_t mdata_count) {
    printf("init_pack_context\n");
    pack_ctx *pctx = calloc(1, sizeof(pack_ctx));
    pctx->mdata_count = mdata_count; //sizeof(MDATA_TEMPLATE) / sizeof(mr_mdata);
    mr_mdata *mdata0 = calloc(pctx->mdata_count, sizeof(mr_mdata));
    printf("copy template\n");
    //  copy template
    for (int i = 0; i < pctx->mdata_count; i++) mdata0[i] = MDATA_TEMPLATE[i];
    pctx->mdata0 = mdata0;
    return pctx;
}

int pack_uint8(pack_ctx *pctx, mr_mdata *mdata) {
    mdata->buflen = 1;
    uint8_t *buf = malloc(mdata->buflen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->buf = buf;
    *buf = mdata->value;
    return 0;
}

int pack_uint16(pack_ctx *pctx, mr_mdata *mdata) {
    mdata->buflen = 2;
    uint8_t *buf = malloc(mdata->buflen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->buf = buf;
    uint16_t val16 = mdata->value;
    uint8_t *bufpos = buf;
    *bufpos++ = (val16 >> 8) & 0xFF;
    *bufpos = val16 & 0xFF;
    return 0;
}

int pack_uint32(pack_ctx *pctx, mr_mdata *mdata) {
    mdata->buflen = 4;
    uint8_t *buf = malloc(mdata->buflen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->buf = buf;
    uint32_t val32 = mdata->value;
    uint8_t *bufpos = buf;
    *bufpos++ = (val32 >> 24) & 0xFF;
    *bufpos++ = (val32 >> 16) & 0xFF;
    *bufpos++ = (val32 >> 8) & 0xFF;
    *bufpos = val32 & 0xFF;
    return 0;
}

int pack_sprop_uint8(pack_ctx *pctx, mr_mdata *mdata) {
    if (mdata->exists) {
        mdata->buflen = 2;
        uint8_t *buf = malloc(mdata->buflen); // TODO: err checks on malloc
        mdata->isalloc = true;
        mdata->buf = buf;
        uint8_t *bufpos = buf;
        *bufpos++ = mdata->id;
        *bufpos = mdata->value;
    }

    return 0;
}

int pack_sprop_uint16(pack_ctx *pctx, mr_mdata *mdata) {
    if (mdata->exists) {
        mdata->buflen = 3;
        uint8_t *buf = malloc(mdata->buflen); // TODO: err checks on malloc
        mdata->isalloc = true;
        mdata->buf = buf;
        uint16_t val16 = mdata->value;
        uint8_t *bufpos = buf;
        *bufpos++ = mdata->id;
        *bufpos++ = (val16 >> 8) & 0xFF;
        *bufpos = val16 & 0xFF;
    }

    return 0;
}

int pack_sprop_uint32(pack_ctx *pctx, mr_mdata *mdata) {
    if (mdata->exists) {
        mdata->buflen = 5;
        uint8_t *buf = malloc(mdata->buflen); // TODO: err checks on malloc
        mdata->isalloc = true;
        mdata->buf = buf;
        uint32_t val32 = mdata->value;
        uint8_t *bufpos = buf;
        *bufpos++ = mdata->id;
        *bufpos++ = (val32 >> 24) & 0xFF;
        *bufpos++ = (val32 >> 16) & 0xFF;
        *bufpos++ = (val32 >> 8) & 0xFF;
        *bufpos = val32 & 0xFF;
    }

    return 0;
}

int pack_str(pack_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    uint8_t *strval = (uint8_t *)mdata->value;
    uint16_t val16 = strlen((char *)strval); // get number of bytes (not chars)
    size_t buflen = 2 + val16;

    uint8_t *buf = malloc(buflen); // TODO: err checks on malloc

    mdata->isalloc = true;
    mdata->buf = buf;
    mdata->buflen = buflen;

    uint8_t *bufpos = buf;
    *bufpos++ = (val16 >> 8) & 0xFF;
    *bufpos++ = val16 & 0xFF;
    memcpy(bufpos, strval, val16);

    return 0;
}

int pack_sprop_str(pack_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    uint8_t *strval = (uint8_t *)mdata->value;
    uint16_t val16 = strlen((char *)strval); // get number of bytes (not chars)
    size_t buflen = 1 + 2 + val16;

    uint8_t *buf = malloc(buflen); // TODO: err checks on malloc

    mdata->isalloc = true;
    mdata->buf = buf;
    mdata->buflen = buflen;

    uint8_t *bufpos = buf;
    *bufpos++ = mdata->id;
    *bufpos++ = (val16 >> 8) & 0xFF;
    *bufpos++ = val16 & 0xFF;
    memcpy(bufpos, strval, val16);

    return 0;
}

int pack_sprop_char_buf(pack_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    size_t buflen = 1 + mdata->vlen;

    uint8_t *buf = malloc(buflen); // TODO: err checks on malloc

    mdata->isalloc = true;
    mdata->buf = buf;
    mdata->buflen = buflen;

    uint8_t *bufpos = buf;
    *bufpos++ = mdata->id;
    memcpy(bufpos, (uint8_t *)mdata->value, mdata->vlen);

    return 0;
}

//  for property vectors, e.g. connect_payload:will_properties:user_properties
//  mdata->value should be a *string_pair
int pack_mprop_strpair(pack_ctx *pctx, mr_mdata *mdata) {
    if (!mdata->exists) return 0;

    string_pair *strpair = (string_pair *)mdata->value;
    size_t buflen = 0;

    for (int i = 0; i < mdata->vlen; i++) {
        buflen += 1 + 2 + strlen((char *)strpair[i].name) + 2 + strlen((char *)strpair[i].value);
    }

    uint8_t *buf = malloc(buflen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->buf = buf;
    mdata->buflen = buflen;
    uint8_t *bufpos = buf;
    uint16_t val16;

    for (int i = 0; i < mdata->vlen; i++) {
        *bufpos++ = mdata->id;
        // name
        val16 = strlen((char *)strpair[i].name);
        *bufpos++ = (val16 >> 8) & 0xFF;
        *bufpos++ = val16 & 0xFF;
        memcpy(bufpos, strpair[i].name, val16); bufpos += val16;
        // value
        val16 = strlen((char *)strpair[i].value);
        *bufpos++ = (val16 >> 8) & 0xFF;
        *bufpos++ = val16 & 0xFF;
        memcpy(bufpos, strpair[i].value, val16); bufpos += val16;
    }

    return 0;
}

int pack_char_buf(pack_ctx *pctx, mr_mdata *mdata) { // do not allocate
    if (mdata->exists) {
        mdata->buflen = mdata->vlen;
        mdata->buf = (uint8_t *)mdata->value;
    }

    return 0;
}

int pack_flags_alloc(pack_ctx *pctx, mr_mdata *mdata) {
    mdata->buflen = 1;
    uint8_t *buf = calloc(1, mdata->buflen); // TODO: err checks on malloc
    mdata->isalloc = true;
    mdata->buf = buf;
    return 0;
};

//  index is vlen: # of bits to be (re)set
const uint8_t BIT_MASKS[] = {
    0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};

//  get the link (parent) mdata, reset bit(s) and set if value is non-zero
int pack_in_parent(pack_ctx *pctx, mr_mdata *mdata) {
    mr_mdata *link_mdata = pctx->mdata0 + mdata->link;
    *link_mdata->buf = *link_mdata->buf & ~(BIT_MASKS[mdata->vlen] << mdata->bitpos);

    if (mdata->value) {
        uint8_t val = mdata->value;
        *link_mdata->buf = *link_mdata->buf | (val << mdata->bitpos);
    }

    return 0;
}
