/* pack.c */

#include <Judy.h>

#include "pack.h"

const uint8_t PROTO_NAME[] = {0x00, 0x04, 'M', 'Q', 'T', 'T'};

const connect_hv connect_hvs_template[] = {
/*  name, function, value, bitpos, exists, id, len */
    {"packet_type", pack_uint8, CMD_CONNECT, 0},
    {"remaining_length", pack_VBI, 0},
    {"protocol_name", pack_buffer, (Word_t)PROTO_NAME, 0, 0, 0, sizeof(PROTO_NAME)},
    {"protocol_version", pack_uint8, 5, 0},
    {"reserved", pack_bool_in_uint8, false, 0, 0},
    {"clean_start", pack_bool_in_uint8, false, 1, 0},
    {"will_flag", pack_bool_in_uint8, false, 2, 0},
    {"will_qos", pack_uint8_in_uint8, 0, 3, 0},
    {"will_retain", pack_bool_in_uint8, false, 5, 0},
    {"password_flag", pack_bool_in_uint8, false, 6, 0},
    {"username_flag", pack_bool_in_uint8, false, 7, 0},
    {"keep_alive", pack_uint16, 0},
    {"property_length", pack_VBI, 0},
    {"session_expiry", pack_prop_uint32, 0, 0, false, 0x11, 0}
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

/*
    JSLG(Pchv, PJSLArray, "remaining_length"); // get the handle
    (*Pchv)->value = 42; // reset the value for the header variable

    JSLG(Pchv, PJSLArray, "clean_start"); // set clean_start boolean
    (*Pchv)->value = true;

    JSLG(Pchv, PJSLArray, "will_qos"); // set will_qos to 3
    (*Pchv)->value = 3;

    (*Pchv)->value = 1; // reset will_qos to 1
*/

int set_header_value(pack_ctx *pctx, char *name, Word_t value) {
    connect_hv **Pchv;

    JSLG(Pchv, pctx->PJSLArray, name); // get the handle
    (*Pchv)->value = value; // reset the value for the header variable

    return 0;
}

//  pack each header var into a buffer using its packing function
int pack_connect_buffer(pack_ctx *pctx) {
    connect_hv chv;

    for (int i = 0; i < pctx->hv_count; i++) {
        chv = pctx->connect_hvs[i];
        chv.pack_fn(pctx, &chv);
    }

    return 0;
}

pack_ctx *init_pack_context(size_t bufsize){
    connect_hv **Pchv;

    puts("init_pack_context");

    pack_ctx *pctx = calloc(sizeof(pack_ctx) + bufsize, 1);
    puts("assigning buf\n");
    pctx->buf = (uint8_t *)pctx + bufsize;
    puts("assigning PJSLArray\n");
    pctx->PJSLArray = (Pvoid_t)NULL;  // initialize JudySL array
    puts("assigning hv_count\n");
    pctx->hv_count = sizeof(connect_hvs_template) / sizeof(connect_hv);
    printf("calloc connect_hvs: %u\n", sizeof(connect_hvs_template));
    connect_hv *connect_hvs = calloc(pctx->hv_count, sizeof(connect_hv));
    printf("copy to connect_hvs: %u\n", pctx->hv_count);

    for (int i = 0; i < pctx->hv_count; i++) {
        printf("name: %s\n", connect_hvs_template[i].name);
        connect_hvs[i] = connect_hvs_template[i];
    }
    // memcpy(*connect_hvs, &connect_hvs_template, sizeof(connect_hvs_template));
    puts("map name\n");
    //  map hv name to hv structure
    for (int i = 0; i < pctx->hv_count; i++) {
        JSLI(Pchv, pctx->PJSLArray, connect_hvs[i].name);
        *Pchv = connect_hvs;
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
