/* pack.c */

#include <Judy.h>

#include "pack.h"

const uint8_t PROTO_NM[] = {0x00, 0x04, 'M', 'Q', 'T', 'T'};

const connect_hv CONNECT_HVS_TEMPLATE[] = {
//    name                  function            value               bitpos len                  exists  id
    {"packet_type",         pack_uint8,         CMD_CONNECT,        0,      0,                  true,   0},
    {"remaining_length",    pack_VBI,           0,                  0,      0,                  false,  0},
    {"protocol_name",       pack_buffer,        (Word_t)PROTO_NM,   0,      sizeof(PROTO_NM),   true,   0},
    {"protocol_version",    pack_uint8,         5,                  0,      0,                  true,   0},
    {"reserved",            pack_bits_in_uint8, 0,                  0,      1,                  false,  0},
    {"clean_start",         pack_bits_in_uint8, 0,                  1,      1,                  false,  0},
    {"will_flag",           pack_bits_in_uint8, 0,                  2,      1,                  false,  0},
    {"will_qos",            pack_bits_in_uint8, 0,                  3,      2,                  false,  0},
    {"will_retain",         pack_bits_in_uint8, 0,                  5,      1,                  false,  0},
    {"password_flag",       pack_bits_in_uint8, 0,                  6,      1,                  false,  0},
    {"username_flag",       pack_bits_in_uint8, 0,                  7,      1,                  false,  0},
    {"keep_alive",          pack_uint16,        0,                  0,      0,                  false,  0},
    {"property_length",     pack_VBI,           0,                  0,      0,                  false,  0},
//    name                  function            value               bitpos  len                 exists  id
    {"session_expiry",      pack_prop_uint32,   0,                  0,      0,                  false,  0x11},
    {"receive_maximum",     pack_prop_uint16,   0,                  0,      0,                  false,  0x21}
};
/*
    uint8_t receive_maximum_id;
    uint16_t receive_maximum;
    uint8_t maximum_packet_size_id;
    uint32_t maximum_packet_size;
    uint8_t topic_alias_maximum_id;
    uint16_t topic_alias_maximum;
    uint8_t request_response_information_id;
    uint8_t request_response_information;
    uint8_t request_problem_information_id;
    uint8_t request_problem_information;
    uint8_t user_property_id;
    uint8_t *user_properties;
    uint8_t authentication_method_id;
    uint8_t *authentication_method;
    uint8_t authentication_data_id;
    uint8_t *authentication_data;
*/

int set_header_value(pack_ctx *pctx, char *name, Word_t value) {
    connect_hv **Pchv;

    JSLG(Pchv, pctx->PJSLArray, name);
    (*Pchv)->value = value;
    (*Pchv)->exists = true;
    return 0;
}

int reset_header_value(pack_ctx *pctx, char *name) {
    connect_hv **Pchv;

    JSLG(Pchv, pctx->PJSLArray, name);
    (*Pchv)->value = 0;
    (*Pchv)->exists = false;
    return 0;
}

//  pack each header var in order into a buffer using its packing function
int pack_connect_buffer(pack_ctx *pctx) {
    connect_hv *chv;

    for (int i = 0; i < pctx->hv_count; i++) {
        chv = &(pctx->connect_hvs[i]);
        chv->pack_fn(pctx, chv);
    }

    return 0;
}

pack_ctx *init_pack_context(size_t bufsize){
    connect_hv **Pchv;

    pack_ctx *pctx = calloc(sizeof(pack_ctx), 1);
    pctx->buf = calloc(bufsize, 1);
    pctx->PJSLArray = (Pvoid_t)NULL;  // initialize JudySL array
    pctx->hv_count = sizeof(CONNECT_HVS_TEMPLATE) / sizeof(connect_hv);
    connect_hv *connect_hvs = calloc(pctx->hv_count, sizeof(connect_hv));

    //  copy template
    for (int i = 0; i < pctx->hv_count; i++) {
        connect_hvs[i] = CONNECT_HVS_TEMPLATE[i];
    }

    //  map hv name to hv structure pointer
    for (int i = 0; i < pctx->hv_count; i++) {
        JSLI(Pchv, pctx->PJSLArray, connect_hvs[i].name);
        *Pchv = &connect_hvs[i];
    }

    pctx->connect_hvs = connect_hvs;
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

int pack_prop_uint16(pack_ctx *pctx, connect_hv *chv) {
    if (chv->exists) {
        uint16_t tempvalue = chv->value;
        chv->value = chv->id;
        pack_uint8(pctx, chv);
        chv->value = tempvalue;
        pack_uint16(pctx, chv);
    }

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

//  assume 1 byte for now
int pack_VBI(pack_ctx *pctx, connect_hv *chv) {
    pctx->buf[pctx->pos] = chv->value;
    pctx->pos++;
    return 0;
}

int pack_buffer(pack_ctx *pctx, connect_hv *chv){
    memcpy(pctx->buf + pctx->pos, (uint8_t *)(chv->value), chv->len);
    pctx->pos += chv->len;
    return 0;
}

const uint8_t BIT_MASKS[] = {
    0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};

//  clear bit(s) then set if value is non-zero
//  should be invoked in bitpos order for all bits in each byte
int pack_bits_in_uint8(pack_ctx *pctx, connect_hv *chv){
    uint8_t *Pbyte = pctx->buf + pctx->pos;
    *Pbyte = *Pbyte & ~(BIT_MASKS[chv->len] << chv->bitpos);

    if (chv->value) {
        uint8_t val = chv->value;
        *Pbyte = *Pbyte | (val << chv->bitpos);
    }

    if (chv->bitpos + chv->len > 7) pctx->pos++;
    return 0;
}
