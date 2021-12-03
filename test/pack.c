/* pack.c */

#include "pack.h"

pack_ctx *initPackContext(size_t bufsize){
    pack_ctx *pctx = calloc(sizeof(pack_ctx) + bufsize, 1);
    pctx->buf = pctx + bufsize;
    return pctx;
}

int pack_uint8(pack_ctx *pctx, connect_hv *chv){
    pctx->buf[pctx->pos] = chv->value;
    pctx->pos++;
    return 0;
}

int pack_uint16(pack_ctx *pctx, connect_hv *chv){
    uint16_t val16 = chv->value;
    pctx->buf[pctx->pos] = (val16 >> 8) & 0xFF;
    pctx->buf[pctx->pos + 1] = val16 & 0xFF;
    pctx->pos += 2;
    return 0;
}

int pack_uint32(pack_ctx *pctx, connect_hv *chv){
    uint32_t val32 = chv->value;
    pctx->buf[pctx->pos] = (val32 >> 24) & 0xFF;
    pctx->buf[pctx->pos + 1] = (val32 >> 16) & 0xFF;
    pctx->buf[pctx->pos + 2] = (val32 >> 8) & 0xFF;
    pctx->buf[pctx->pos + 3] = val32 & 0xFF;
    pctx->pos += 4;
    return 0;
}

int pack_prop_uint32(pack_ctx *pctx, connect_hv *chv) {
    if (chv->exists) {
        uint32_t tempvalue = chv->value;
        chv->value = chv->id;
        pack_uint8(pctx, chv);
        chv->value = tempvalue;
        pack_uint32(pctx, chv);
    }

    return 0;
}

int pack_VBI(pack_ctx *pctx, connect_hv *chv) { // assume 1 byte for now
    pctx->buf[pctx->pos] = chv->value;
    pctx->pos++;
    return 0;
}

int pack_buffer(pack_ctx *pctx, connect_hv *chv){
    memcpy(pctx->buf + pctx->pos, (uint8_t *)(chv->value), chv->len);
    pctx->pos += chv->len;
    return 0;
}

const uint8_t single_bit = 0x01;
const uint8_t two_bits = 0x03;

int pack_bool_in_uint8(pack_ctx *pctx, connect_hv *chv){
    if (chv->value) {
        pctx->buf[pctx->pos] = (pctx->buf[pctx->pos] | (single_bit << chv->bitpos));
    } else {
        pctx->buf[pctx->pos] = (pctx->buf[pctx->pos] & ~(single_bit << chv->bitpos));
    }

    if (chv->bitpos == 7) pctx->pos++;
    return 0;
}

int pack_uint8_in_uint8(pack_ctx *pctx, connect_hv *chv){
    pctx->buf[pctx->pos] = (pctx->buf[pctx->pos] & ~(two_bits << chv->bitpos));

    if (chv->value) {
        uint8_t val = chv->value;
        pctx->buf[pctx->pos] = (pctx->buf[pctx->pos] | (val << chv->bitpos));
    }

    return 0;
}
