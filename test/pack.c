/* pack.c */

#include <Judy.h>

#include "pack.h"


const uint8_t PNM[] = {0x00, 0x04, 'M', 'Q', 'T', 'T'};
#define PNMSZ 6
#define NA 0

const connect_hdr CONNECT_HDRS_TEMPLATE[] = {
//   name                   index   parent      function            value           bitpos  vlen    exists  id      isalloc buflen  buf
    {"packet_type",         0,      "",         pack_uint8,         CMD_CONNECT,    NA,     1,      true,   NA,     false,  0,      NULL},
    {"remaining_length",    0,      "last",     pack_VBI,           0,              NA,     0,      true,   NA,     false,  0,      NULL},
    {"protocol_name",       0,      "",         pack_char_buffer,   (Word_t)PNM,    NA,     PNMSZ,  true,   NA,     false,  0,      NULL},
    {"protocol_version",    0,      "",         pack_uint8,         5,              NA,     2,      true,   NA,     false,  0,      NULL},
    {"reserved",            0,      "flags",    pack_in_parent,     0,              0,      1,      true,   NA,     false,  0,      NULL},
    {"clean_start",         0,      "flags",    pack_in_parent,     0,              1,      1,      true,   NA,     false,  0,      NULL},
    {"will_flag",           0,      "flags",    pack_in_parent,     0,              2,      1,      true,   NA,     false,  0,      NULL},
    {"will_qos",            0,      "flags",    pack_in_parent,     0,              3,      2,      true,   NA,     false,  0,      NULL},
    {"will_retain",         0,      "flags",    pack_in_parent,     0,              5,      1,      true,   NA,     false,  0,      NULL},
    {"password_flag",       0,      "flags",    pack_in_parent,     0,              6,      1,      true,   NA,     false,  0,      NULL},
    {"username_flag",       0,      "flags",    pack_in_parent,     0,              7,      1,      true,   NA,     false,  0,      NULL},
    {"flags",               0,      "",         pack_flags_alloc,   NA,             NA,     NA,     true,   NA,     false,  0,      NULL},
    {"keep_alive",          0,      "flags",    pack_uint16,        0,              NA,     2,      true,   NA,     false,  0,      NULL},
    {"property_length",     0,      "receive_maximum",
                                                pack_VBI,           0,              NA,     0,      true,   NA,     false,  0,      NULL},
//   name                   index   parent     function             value           bitpos  vlen    exists  id      isalloc buflen  buf
    {"session_expiry",      0,      "",         pack_sprop_uint32,  0,              NA,     0,      false,  0x11,   false,  0,      NULL},
    {"receive_maximum",     0,      "",         pack_sprop_uint16,  0,              NA,     0,      false,  0x21,   false,  0,      NULL}
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

//  pack each header var in reverse order of the table
//  into its allocated buffer using its packing function
//  then allocate pctx->buf and accumulate header var buffers
int pack_connect_buffer(pack_ctx *pctx) {
    printf("pack_connect_buffer\n");
    connect_hdr *chdr;
    printf("pack each chdr: chdr_count: %u\n", pctx->chdr_count);
    for (int i = pctx->chdr_count - 1; i > -1; i--) {
        printf("  chdr index: %u\n", i);
        chdr = &pctx->connect_hdrs[i];
        chdr->pack_fn(pctx, chdr);
    }
    printf("malloc pctx-buf: buflen: %u; value: %u\n", chdr->buflen, chdr->value);
    chdr = &pctx->connect_hdrs[1];
    pctx->len = chdr->value + chdr->buflen + 1;
    uint8_t *buf = malloc(pctx->len); // TODO: err checks on malloc
    pctx->buf = buf;
    printf("copy chdr bufs into pctx->buf\n");
    size_t pos = 0;
    for (int i = 0; i < pctx->chdr_count; i++) {
        chdr = &pctx->connect_hdrs[i];
        memcpy(buf + pos, chdr->buf, chdr->buflen);
        pos += chdr->buflen;
    }

    return 0;
}

//  calculate length, convert to VBI, & pack into buffer
int pack_VBI(pack_ctx *pctx, connect_hdr *chdr) {
    printf("pack_VBI: %s\n", chdr->name);
    size_t cum_len = 0;
    connect_hdr **Pchdr, *end_chdr, *current_chdr;

    if (strcmp(chdr->parent, "last")) {
        JSLG(Pchdr, pctx->PJSLArray, chdr->parent);
        end_chdr = *Pchdr;
    }
    else {
        end_chdr = &pctx->connect_hdrs[pctx->chdr_count - 1];
    }
    printf("accumulate buffer lengths: %u to %u\n", chdr->index + 1, end_chdr->index);
    //  accumulate buffer lengths in cum_len for the range of the VBI
    for (int j = chdr->index + 1; j <= end_chdr->index; j++) {
        current_chdr = &pctx->connect_hdrs[j];
        if (current_chdr->exists) cum_len += current_chdr->buflen;
    }

    chdr->value = cum_len;
    uint32_t tmp_val32 = chdr->value; // TODO: err if too large for 4 bytes

    //  handle 7 bits at a time; use bit 7 as the continution flag
    int i;
    uint8_t tmp_buf[5];
    for (i = 0; ; i++) {
        tmp_buf[i] = tmp_val32 & 0x7F;
        tmp_val32 = tmp_val32 >> 7;

        if (tmp_val32 > 0) {
            tmp_buf[i] = tmp_buf[i] | 0x80;
        }
        else {
            break;
        }
    }

    chdr->buflen = i + 1;
    uint8_t *buf = malloc(chdr->buflen); // TODO: err checks on malloc
    chdr->isalloc = true;
    for (i = 0; i < chdr->buflen; i++) buf[i] = tmp_buf[i];
    chdr->buf = buf;
    return 0;
}

pack_ctx *init_pack_context(size_t bufsize) {
    printf("init_pack_context\n");
    connect_hdr **Pchdr;

    pack_ctx *pctx = malloc(sizeof(pack_ctx));
    // pctx->buf = calloc(bufsize, 1);
    pctx->PJSLArray = (Pvoid_t)NULL;  // initialize JudySL array
    pctx->chdr_count = sizeof(CONNECT_HDRS_TEMPLATE) / sizeof(connect_hdr);
    connect_hdr *connect_hdrs = calloc(pctx->chdr_count, sizeof(connect_hdr));
    printf("copy template\n");
    //  copy template
    for (int i = 0; i < pctx->chdr_count; i++) {
        connect_hdrs[i] = CONNECT_HDRS_TEMPLATE[i];
        connect_hdrs[i].index = i;
    }
    printf("map hv name\n");
    //  map hv name to hv structure pointer
    for (int i = 0; i < pctx->chdr_count; i++) {
        JSLI(Pchdr, pctx->PJSLArray, connect_hdrs[i].name);
        *Pchdr = &connect_hdrs[i];
    }

    pctx->connect_hdrs = connect_hdrs;
    return pctx;
}

int pack_uint8(pack_ctx *pctx, connect_hdr *chdr) {
    chdr->buflen = 1;
    uint8_t *buf = malloc(chdr->buflen); // TODO: err checks on malloc
    chdr->isalloc = true;
    buf[0] = chdr->value;
    chdr->buf = buf;
    return 0;
}

int pack_uint16(pack_ctx *pctx, connect_hdr *chdr) {
    chdr->buflen = 2;
    uint8_t *buf = malloc(chdr->buflen); // TODO: err checks on malloc
    chdr->isalloc = true;
    uint16_t val16 = chdr->value;
    buf[0] = (val16 >> 8) & 0xFF;
    buf[1] = val16 & 0xFF;
    chdr->buf = buf;
    return 0;
}

int pack_uint32(pack_ctx *pctx, connect_hdr *chdr) {
    chdr->buflen = 4;
    uint8_t *buf = malloc(chdr->buflen); // TODO: err checks on malloc
    chdr->isalloc = true;
    uint32_t val32 = chdr->value;
    buf[0] = (val32 >> 24) & 0xFF;
    buf[1] = (val32 >> 16) & 0xFF;
    buf[2] = (val32 >> 8) & 0xFF;
    buf[3] = val32 & 0xFF;
    chdr->buf = buf;
    return 0;
}

int pack_sprop_uint16(pack_ctx *pctx, connect_hdr *chdr) {
    if (chdr->exists) {
        chdr->buflen = 3;
        uint8_t *buf = malloc(chdr->buflen); // TODO: err checks on malloc
        chdr->isalloc = true;
        buf[0] = chdr->id;
        uint16_t val16 = chdr->value;
        buf[1] = (val16 >> 8) & 0xFF;
        buf[2] = val16 & 0xFF;
        chdr->buf = buf;
    }

    return 0;
}

int pack_sprop_uint32(pack_ctx *pctx, connect_hdr *chdr) {
    if (chdr->exists) {
        chdr->buflen = 5;
        uint8_t *buf = malloc(chdr->buflen); // TODO: err checks on malloc
        chdr->isalloc = true;
        buf[0] = chdr->id;
        uint32_t val32 = chdr->value;
        buf[1] = (val32 >> 24) & 0xFF;
        buf[2] = (val32 >> 16) & 0xFF;
        buf[3] = (val32 >> 8) & 0xFF;
        buf[4] = val32 & 0xFF;
        chdr->buf = buf;
    }

    return 0;
}

//  for property vectors, e.g. connect_payload:will_properties:user_properties
int pack_mprop_strpair(pack_ctx *pctx, connect_hdr *chdr) {
    ;
}

int pack_char_buffer(pack_ctx *pctx, connect_hdr *chdr) {
    chdr->buflen = chdr->vlen;
    chdr->buf = (uint8_t *)(chdr->value);
    return 0;
}

int pack_flags_alloc(pack_ctx *pctx, connect_hdr *chdr) {
    chdr->buflen = 1;
    uint8_t *buf = calloc(1, chdr->buflen); // TODO: err checks on malloc
    chdr->isalloc = true;
    chdr->buf = buf;
    return 0;
};

//  index is vlen: # of bits to be (re)set
const uint8_t BIT_MASKS[] = {
    0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};

//  get the parent (flags) chdr, reset bit(s) and set if value is non-zero
int pack_in_parent(pack_ctx *pctx, connect_hdr *chdr) {
    connect_hdr **Pchdr;
    JSLG(Pchdr, pctx->PJSLArray, chdr->parent);
    connect_hdr *flags_chdr = *Pchdr;
    flags_chdr->buf[0] = flags_chdr->buf[0] & ~(BIT_MASKS[chdr->vlen] << chdr->bitpos);

    if (chdr->value) {
        uint8_t val = chdr->value;
        flags_chdr->buf[0] = flags_chdr->buf[0] | (val << chdr->bitpos);
    }

    return 0;
}
