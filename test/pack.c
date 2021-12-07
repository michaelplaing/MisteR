/* pack.c */

#include <Judy.h>

#include "pack.h"

const uint8_t PNM[] = {0x00, 0x04, 'M', 'Q', 'T', 'T'};
#define PNMSZ 6
#define NA 0

const connect_hdr CONNECT_HDRS_TEMPLATE[] = {
//   name                   function            value           bitpos  vlen    exists  id      blen    bval
    {"packet_type",         pack_uint8,         CMD_CONNECT,    NA,      1,     true,   NA,     0,      0},
    {"remaining_length",    pack_VBI,           0,              NA,      0,     false,  NA,     0,      0},
    {"protocol_name",       pack_char_buffer,   (Word_t)PNM,    NA,      PNMSZ, true,   NA,     0,      0},
    {"protocol_version",    pack_uint8,         5,              NA,      2,     true,   NA,     0,      0},
    {"reserved",            pack_bits_in_uint8, 0,              0,       1,     true,   NA,     0,      0},
    {"clean_start",         pack_bits_in_uint8, 0,              1,       1,     true,   NA,     0,      0},
    {"will_flag",           pack_bits_in_uint8, 0,              2,       1,     true,   NA,     0,      0},
    {"will_qos",            pack_bits_in_uint8, 0,              3,       2,     true,   NA,     0,      0},
    {"will_retain",         pack_bits_in_uint8, 0,              5,       1,     true,   NA,     0,      0},
    {"password_flag",       pack_bits_in_uint8, 0,              6,       1,     true,   NA,     0,      0},
    {"username_flag",       pack_bits_in_uint8, 0,              7,       1,     true,   NA,     0,      0},
    {"keep_alive",          pack_uint16,        0,              NA,      2,     true,   NA,     0,      0},
    {"property_length",     pack_VBI,           0,              NA,      0,     true,   NA,     0,      0},
//   name                   function            value           bitpos  vlen    exists  id      blen    bval
    {"session_expiry",      pack_prop_uint32,   0,              NA,      0,     false,  0x11,   0,      0},
    {"receive_maximum",     pack_prop_uint16,   0,              NA,      0,     false,  0x21,   0,      0}
};
/*
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
    connect_hdr **Pchdr;

    JSLG(Pchdr, pctx->PJSLArray, name);
    (*Pchdr)->value = value;
    (*Pchdr)->exists = true;
    return 0;
}

int reset_header_value(pack_ctx *pctx, char *name) {
    connect_hdr **Pchdr;

    JSLG(Pchdr, pctx->PJSLArray, name);
    (*Pchdr)->value = 0;
    (*Pchdr)->exists = false;
    return 0;
}

//  pack each header var in order into a buffer using its packing function
int pack_connect_buffer(pack_ctx *pctx) {
    connect_hdr *chdr;

    for (int i = 0; i < pctx->chdr_count; i++) {
        chdr = &(pctx->connect_hdrs[i]);
        chdr->pack_fn(pctx, chdr);
    }

    return 0;
}

pack_ctx *init_pack_context(size_t bufsize){
    connect_hdr **Pchdr;

    pack_ctx *pctx = calloc(sizeof(pack_ctx), 1);
    pctx->buf = calloc(bufsize, 1);
    pctx->PJSLArray = (Pvoid_t)NULL;  // initialize JudySL array
    pctx->chdr_count = sizeof(CONNECT_HDRS_TEMPLATE) / sizeof(connect_hdr);
    connect_hdr *connect_hdrs = calloc(pctx->chdr_count, sizeof(connect_hdr));

    //  copy template
    for (int i = 0; i < pctx->chdr_count; i++) {
        connect_hdrs[i] = CONNECT_HDRS_TEMPLATE[i];
    }

    //  map hv name to hv structure pointer
    for (int i = 0; i < pctx->chdr_count; i++) {
        JSLI(Pchdr, pctx->PJSLArray, connect_hdrs[i].name);
        *Pchdr = &connect_hdrs[i];
    }

    pctx->connect_hdrs = connect_hdrs;
    return pctx;
}

int pack_uint8(pack_ctx *pctx, connect_hdr *chdr){
    pctx->buf[pctx->pos] = chdr->value;
    pctx->pos++;
    return 0;
}

int pack_uint16(pack_ctx *pctx, connect_hdr *chdr){
    uint16_t val16 = chdr->value;
    pctx->buf[pctx->pos] = (val16 >> 8) & 0xFF;
    pctx->buf[pctx->pos + 1] = val16 & 0xFF;
    pctx->pos += 2;
    return 0;
}

int pack_uint32(pack_ctx *pctx, connect_hdr *chdr){
    uint32_t val32 = chdr->value;
    pctx->buf[pctx->pos] = (val32 >> 24) & 0xFF;
    pctx->buf[pctx->pos + 1] = (val32 >> 16) & 0xFF;
    pctx->buf[pctx->pos + 2] = (val32 >> 8) & 0xFF;
    pctx->buf[pctx->pos + 3] = val32 & 0xFF;
    pctx->pos += 4;
    return 0;
}

int pack_prop_uint16(pack_ctx *pctx, connect_hdr *chdr) {
    if (chdr->exists) {
        uint16_t tempvalue = chdr->value;
        chdr->value = chdr->id;
        pack_uint8(pctx, chdr);
        chdr->value = tempvalue;
        pack_uint16(pctx, chdr);
    }

    return 0;
}

int pack_prop_uint32(pack_ctx *pctx, connect_hdr *chdr) {
    if (chdr->exists) {
        uint32_t tempvalue = chdr->value;
        chdr->value = chdr->id;
        pack_uint8(pctx, chdr);
        chdr->value = tempvalue;
        pack_uint32(pctx, chdr);
    }

    return 0;
}

//  assume 1 byte for now
int pack_VBI(pack_ctx *pctx, connect_hdr *chdr) {
    pctx->buf[pctx->pos] = chdr->value;
    pctx->pos++;
    return 0;
}

int pack_char_buffer(pack_ctx *pctx, connect_hdr *chdr){
    memcpy(pctx->buf + pctx->pos, (uint8_t *)(chdr->value), chdr->vlen);
    pctx->pos += chdr->vlen;
    return 0;
}

const uint8_t BIT_MASKS[] = {
    0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};

//  clear bit(s) then set if value is non-zero
//  should be invoked in bitpos order for all bits in each byte
int pack_bits_in_uint8(pack_ctx *pctx, connect_hdr *chdr){
    uint8_t *Pbyte = pctx->buf + pctx->pos;
    *Pbyte = *Pbyte & ~(BIT_MASKS[chdr->vlen] << chdr->bitpos);

    if (chdr->value) {
        uint8_t val = chdr->value;
        *Pbyte = *Pbyte | (val << chdr->bitpos);
    }

    if (chdr->bitpos + chdr->vlen > 7) pctx->pos++;
    return 0;
}
