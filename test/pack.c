/* pack.c */

#include <Judy.h>

#include "pack.h"


const uint8_t PNM[] = {0x00, 0x04, 'M', 'Q', 'T', 'T'};
#define PNMSZ 6
#define NA 0

const connect_hdr CONNECT_HDRS_TEMPLATE[] = {
//  Fixed Header
//   name                   index   link        function            value           bitpos  vlen    exists  id      isalloc buflen  buf
    {"packet_type",         0,      "",         pack_uint8,         CMD_CONNECT,    NA,     1,      true,   NA,     false,  0,      NULL},
    {"remaining_length",    0,      "last",     pack_VBI,           0,              NA,     0,      true,   NA,     false,  0,      NULL},
//  Variable Header
//   name                   index   link        function            value           bitpos  vlen    exists  id      isalloc buflen  buf
    {"protocol_name",       0,      "",         pack_char_buf,      (Word_t)PNM,    NA,     PNMSZ,  true,   NA,     false,  0,      NULL},
    {"protocol_version",    0,      "",         pack_uint8,         5,              NA,     2,      true,   NA,     false,  0,      NULL},
    {"reserved",            0,      "flags",    pack_in_parent,     0,              0,      1,      true,   NA,     false,  0,      NULL},
    {"clean_start",         0,      "flags",    pack_in_parent,     0,              1,      1,      true,   NA,     false,  0,      NULL},
    {"will_flag",           0,      "flags",    pack_in_parent,     0,              2,      1,      true,   NA,     false,  0,      NULL},
    {"will_qos",            0,      "flags",    pack_in_parent,     0,              3,      2,      true,   NA,     false,  0,      NULL},
    {"will_retain",         0,      "flags",    pack_in_parent,     0,              5,      1,      true,   NA,     false,  0,      NULL},
    {"password_flag",       0,      "flags",    pack_in_parent,     0,              6,      1,      true,   NA,     false,  0,      NULL},
    {"username_flag",       0,      "flags",    pack_in_parent,     0,              7,      1,      true,   NA,     false,  0,      NULL},
    {"flags",               0,      "",         pack_flags_alloc,   NA,             NA,     1,      true,   NA,     false,  0,      NULL},
    {"keep_alive",          0,      "",         pack_uint16,        0,              NA,     2,      true,   NA,     false,  0,      NULL},
//  Variable Header Properties
//   name                   index   link        function            value           bitpos  vlen    exists  id      isalloc buflen  buf
    {"property_length",     0,      "authentication_data",
                                                pack_VBI,           0,              NA,     0,      true,   NA,     false,  0,      NULL},
    {"session_expiry",      0,      "",         pack_sprop_uint32,  0,              NA,     0,      false,  0x11,   false,  0,      NULL},
    {"receive_maximum",     0,      "",         pack_sprop_uint16,  0,              NA,     0,      false,  0x21,   false,  0,      NULL},
    {"maximum_packet_size", 0,      "",         pack_sprop_uint32,  0,              NA,     0,      false,  0x27,   false,  0,      NULL},
    {"topic_alias_maximum", 0,      "",         pack_sprop_uint16,  0,              NA,     0,      false,  0x22,   false,  0,      NULL},
    {"request_response_information",
                            0,      "",         pack_sprop_uint8,   0,              NA,     0,      false,  0x19,   false,  0,      NULL},
    {"request_problem_information",
                            0,      "",         pack_sprop_uint8,   0,              NA,     0,      false,  0x17,   false,  0,      NULL},
    {"user_properties",     0,      "",         pack_mprop_strpair, (Word_t)NULL,   NA,     0,      false,  0x26,   false,  0,      NULL},
    {"authentication_method",
                            0,      "",         pack_sprop_str,     (Word_t)NULL,   NA,     0,      false,  0x15,   false,  0,      NULL},
    {"authentication_data", 0,      "",         pack_sprop_char_buf,(Word_t)NULL,   NA,     0,      false,  0x16,   false,  0,      NULL},
// Payload
//   name                   index   link        function            value           bitpos  vlen    exists  id      isalloc buflen  buf
    {"client_identifier",   0,      "",         pack_str,           (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL},
// Payload Will Properties
    {"will_property_length",0,      "will_user_properties",
                                                pack_VBI,           0,              NA,     0,      false,  NA,     false,  0,      NULL},
    {"will_delay_interval", 0,      "",         pack_sprop_uint32,  0,              NA,     0,      false,  0x18,   false,  0,      NULL},
    {"payload_format_indicator",
                            0,      "",         pack_sprop_uint8,   0,              NA,     0,      false,  0x01,   false,  0,      NULL},
    {"message_expiry_interval",
                            0,      "",         pack_sprop_uint32,  0,              NA,     0,      false,  0x02,   false,  0,      NULL},
    {"content_type",        0,      "",         pack_sprop_str,     (Word_t)NULL,   NA,     0,      false,  0x03,   false,  0,      NULL},
    {"response_topic",      0,      "",         pack_sprop_str,     (Word_t)NULL,   NA,     0,      false,  0x08,   false,  0,      NULL},
    {"correlation_data",    0,      "",         pack_sprop_char_buf,(Word_t)NULL,   NA,     0,      false,  0x09,   false,  0,      NULL},
    {"will_user_properties",0,      "",         pack_mprop_strpair, (Word_t)NULL,   NA,     0,      false,  0x26,   false,  0,      NULL},
// Payload (remainder)
    {"will_topic",          0,      "",         pack_str,           (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL},
    {"will_payload",        0,      "",         pack_char_buf,      (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL},
    {"user_name",           0,      "",         pack_str,           (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL},
    {"password",            0,      "",         pack_char_buf,      (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL}
};

int set_scalar_value(pack_ctx *pctx, char *name, Word_t value) {
    connect_hdr **Pchdr;
    JSLG(Pchdr, pctx->PJSLArray, name);
    (*Pchdr)->value = value;
    (*Pchdr)->exists = true;
    return 0;
}

int set_vector_value(pack_ctx *pctx, char *name, Word_t value, size_t len) {
    connect_hdr **Pchdr;
    JSLG(Pchdr, pctx->PJSLArray, name);
    (*Pchdr)->value = value;
    (*Pchdr)->exists = true;
    (*Pchdr)->vlen = len;
    return 0;
}

int reset_header_value(pack_ctx *pctx, char *name) {
    connect_hdr **Pchdr;
    JSLG(Pchdr, pctx->PJSLArray, name);
    (*Pchdr)->value = 0;
    (*Pchdr)->exists = false;
    (*Pchdr)->vlen = 0;
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
    chdr = &pctx->connect_hdrs[1];
    printf("malloc pctx->buf using from remaining_length: buflen: %u + value: %u + 1\n", chdr->buflen, chdr->value);
    pctx->len = chdr->value + chdr->buflen + 1;
    uint8_t *buf = malloc(pctx->len); // TODO: err checks on malloc
    pctx->buf = buf;
    pctx->isalloc = true;
    printf("memcpy chdr bufs into pctx->buf\n");
    uint8_t *bufpos = buf;
    for (int i = 0; i < pctx->chdr_count; i++) {
        chdr = &pctx->connect_hdrs[i];
        memcpy(bufpos, chdr->buf, chdr->buflen);
        bufpos += chdr->buflen;
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
int pack_VBI(pack_ctx *pctx, connect_hdr *chdr) {
    if (!chdr->exists) return 0;

    printf("pack_VBI: %s\n", chdr->name);
    size_t cum_len = 0;
    connect_hdr **Pchdr, *end_chdr, *current_chdr;

    if (strcmp(chdr->link, "last")) {
        JSLG(Pchdr, pctx->PJSLArray, chdr->link);
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
    printf("  cum_len: %u\n", cum_len);
    chdr->value = cum_len;
    uint32_t tmp_val32 = chdr->value; // TODO: err if too large for 4 bytes
    uint8_t tmp_buf[5]; // enough for uint32_t even if too big

    chdr->buflen = make_VBI(tmp_val32, tmp_buf);

    uint8_t *buf = malloc(chdr->buflen); // TODO: err checks on malloc
    chdr->isalloc = true;
    chdr->buf = buf;
    memcpy(chdr->buf, tmp_buf, chdr->buflen);
    return 0;
}

int free_pack_context(pack_ctx *pctx) {
    printf("free_ack_context\n");
    connect_hdr *chdr;
    Word_t bytes_freed;

    //  free connect hdr buffers
    for (int i = 0; i < pctx->chdr_count; i++) {
        chdr = &pctx->connect_hdrs[i];
        if (chdr->isalloc) free(chdr->buf);
    }

    //  free packet buffer - don't rely on NULL pointer...
    if (pctx->isalloc) free(pctx->buf);

    //  free Judy array
    JSLFA(bytes_freed, pctx->PJSLArray);

    // free pack context
    free(pctx);

    return 0;
}

pack_ctx *init_pack_context(void) {
    printf("init_pack_context\n");
    connect_hdr **Pchdr;

    pack_ctx *pctx = calloc(1, sizeof(pack_ctx));
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

int pack_sprop_uint8(pack_ctx *pctx, connect_hdr *chdr) {
    if (chdr->exists) {
        chdr->buflen = 2;
        uint8_t *buf = malloc(chdr->buflen); // TODO: err checks on malloc
        chdr->isalloc = true;
        buf[0] = chdr->id;
        buf[1] = chdr->value;
        chdr->buf = buf;
    }

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

int pack_str(pack_ctx *pctx, connect_hdr *chdr) {
    if (!chdr->exists) return 0;

    char *strval = (char *)chdr->value;
    uint16_t val16 = strlen(strval);
    size_t buflen = 2 + val16;

    uint8_t *buf = malloc(buflen); // TODO: err checks on malloc

    chdr->isalloc = true;
    chdr->buf = buf;
    chdr->buflen = buflen;

    uint8_t *bufpos = buf;
    *bufpos = (val16 >> 8) & 0xFF; bufpos++;
    *bufpos = val16 & 0xFF; bufpos++;
    memcpy(bufpos, strval, val16);

    return 0;
}

int pack_sprop_str(pack_ctx *pctx, connect_hdr *chdr) {
    if (!chdr->exists) return 0;

    char *strval = (char *)chdr->value;
    uint16_t val16 = strlen(strval);
    size_t buflen = 1 + 2 + val16;

    uint8_t *buf = malloc(buflen); // TODO: err checks on malloc

    chdr->isalloc = true;
    chdr->buf = buf;
    chdr->buflen = buflen;

    uint8_t *bufpos = buf;
    *bufpos = chdr->id; bufpos++;
    *bufpos = (val16 >> 8) & 0xFF; bufpos++;
    *bufpos = val16 & 0xFF; bufpos++;
    memcpy(bufpos, strval, val16);

    return 0;
}

int pack_sprop_char_buf(pack_ctx *pctx, connect_hdr *chdr) {
    if (!chdr->exists) return 0;

    size_t buflen = 1 + chdr->vlen;

    uint8_t *buf = malloc(buflen); // TODO: err checks on malloc

    chdr->isalloc = true;
    chdr->buf = buf;
    chdr->buflen = buflen;

    uint8_t *bufpos = buf;
    *bufpos = chdr->id; bufpos++;
    memcpy(bufpos, (uint8_t *)chdr->value, chdr->vlen);

    return 0;
}

//  for property vectors, e.g. connect_payload:will_properties:user_properties
//  chdr->value should be a *string_pair
int pack_mprop_strpair(pack_ctx *pctx, connect_hdr *chdr) {
    if (!chdr->exists) return 0;

    string_pair *strpair = (string_pair *)chdr->value;
    size_t buflen = 0;

    for (int i = 0; i < chdr->vlen; i++) {
        buflen += 1 + 2 + strlen(strpair[i].name) + 2 + strlen(strpair[i].value);
    }

    uint8_t *buf = malloc(buflen); // TODO: err checks on malloc
    chdr->isalloc = true;
    chdr->buf = buf;
    chdr->buflen = buflen;
    uint8_t *bufpos = buf;
    uint16_t val16;

    for (int i = 0; i < chdr->vlen; i++) {
        *bufpos = chdr->id; bufpos++;
        // name
        val16 = strlen(strpair[i].name);
        *bufpos = (val16 >> 8) & 0xFF; bufpos++;
        *bufpos = val16 & 0xFF; bufpos++;
        memcpy(bufpos, strpair[i].name, val16); bufpos += val16;
        // value
        val16 = strlen(strpair[i].value);
        *bufpos = (val16 >> 8) & 0xFF; bufpos++;
        *bufpos = val16 & 0xFF; bufpos++;
        memcpy(bufpos, strpair[i].value, val16); bufpos += val16;
    }

    return 0;
}

int pack_char_buf(pack_ctx *pctx, connect_hdr *chdr) {
    if (chdr->exists) {
        chdr->buflen = chdr->vlen;
        chdr->buf = (uint8_t *)chdr->value;
    }

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

//  get the link (parent) chdr, reset bit(s) and set if value is non-zero
int pack_in_parent(pack_ctx *pctx, connect_hdr *chdr) {
    connect_hdr **Pchdr;
    JSLG(Pchdr, pctx->PJSLArray, chdr->link);
    connect_hdr *link_chdr = *Pchdr;
    link_chdr->buf[0] = link_chdr->buf[0] & ~(BIT_MASKS[chdr->vlen] << chdr->bitpos);

    if (chdr->value) {
        uint8_t val = chdr->value;
        link_chdr->buf[0] = link_chdr->buf[0] | (val << chdr->bitpos);
    }

    return 0;
}
