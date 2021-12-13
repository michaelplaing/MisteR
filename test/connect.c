/* connect.c */

#include "pack.h"

const uint8_t PNM[] = {0x00, 0x04, 'M', 'Q', 'T', 'T'};  // protocol name
#define PNMSZ 6
#define PROTO_VER 5 // protocol version
#define NA 0

const mr_mdata CONNECT_HDRS_TEMPLATE[] = {
//  Fixed Header
//   name                   index   link        function            value           bitpos  vlen    exists  id      isalloc buflen  buf
    {"packet_type",         0,      "",         pack_uint8,         CMD_CONNECT,    NA,     1,      true,   NA,     false,  0,      NULL},
    {"remaining_length",    0,      "last",     pack_VBI,           0,              NA,     0,      true,   NA,     false,  0,      NULL},
//  Variable Header
//   name                   index   link        function            value           bitpos  vlen    exists  id      isalloc buflen  buf
    {"protocol_name",       0,      "",         pack_char_buf,      (Word_t)PNM,    NA,     PNMSZ,  true,   NA,     false,  0,      NULL},
    {"protocol_version",    0,      "",         pack_uint8,         PROTO_VER,      NA,     2,      true,   NA,     false,  0,      NULL},
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

pack_ctx *init_connect_pctx(void) {
    printf("init_connect_pctx\n");
    size_t mdata_count = sizeof(CONNECT_HDRS_TEMPLATE) / sizeof(mr_mdata);
    return init_pack_context(CONNECT_HDRS_TEMPLATE, mdata_count);
}

int pack_connect_buffer(pack_ctx *pctx) {
    printf("pack_connect_buffer\n");
    return pack_mdata_buffer(pctx);
}

int free_connect_pctx(pack_ctx *pctx) {
    printf("free_connect_context\n");
    return free_pack_context(pctx);
}

