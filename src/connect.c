/* connect.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mister/mister.h"
#include "connect_internal.h"
#include "packet_internal.h"

const uint8_t PNM[] = {0x00, 0x04, 'M', 'Q', 'T', 'T'};  // protocol signature
#define PNMSZ 6
#define PROTO_VERSION 5 // protocol version
#define NA 0

const mr_mdata CONNECT_MDATA_TEMPLATE[] = {
//  Fixed Header
//   name                   index                       link
//      pack_fn             unpack_fn           value           bitpos  vlen    exists  id      isalloc blen  buf
    {"packet_type",         CONNECT_PACKET_TYPE,        0,
        pack_uint8,         unpack_uint8,       CMD_CONNECT,    NA,     NA,     true,   NA,     false,  0,      NULL},
    {"remaining_length",    CONNECT_REMAINING_LENGTH,   CONNECT_PASSWORD,
        pack_VBI,           unpack_VBI,         0,              NA,     0,      true,   NA,     false,  0,      NULL},
//  Variable Header
//   name                   index                       link
//      pack_fn             unpack_fn           value           bitpos  vlen    exists  id      isalloc blen  buf
    {"protocol_name",       CONNECT_PROTOCOL_NAME,      0,
        pack_char_buf,      NULL,               (Word_t)PNM,    NA,     PNMSZ,  true,   NA,     false,  0,      NULL},
    {"protocol_version",    CONNECT_PROTOCOL_VERSION,   0,
        pack_uint8,         NULL,               PROTO_VERSION,  NA,     NA,     true,   NA,     false,  0,      NULL},
    {"reserved",            CONNECT_RESERVED,           CONNECT_FLAGS,
        pack_in_parent,     NULL,               0,              0,      1,      true,   NA,     false,  0,      NULL},
    {"clean_start",         CONNECT_CLEAN_START,        CONNECT_FLAGS,
        pack_in_parent,     NULL,               0,              1,      1,      true,   NA,     false,  0,      NULL},
    {"will_flag",           CONNECT_WILL_FLAG,          CONNECT_FLAGS,
        pack_in_parent,     NULL,               0,              2,      1,      true,   NA,     false,  0,      NULL},
    {"will_qos",            CONNECT_WILL_QOS,           CONNECT_FLAGS,
        pack_in_parent,     NULL,               0,              3,      2,      true,   NA,     false,  0,      NULL},
    {"will_retain",         CONNECT_WILL_RETAIN,        CONNECT_FLAGS,
        pack_in_parent,     NULL,               0,              5,      1,      true,   NA,     false,  0,      NULL},
    {"password_flag",       CONNECT_PASSWORD_FLAG,      CONNECT_FLAGS,
        pack_in_parent,     NULL,               0,              6,      1,      true,   NA,     false,  0,      NULL},
    {"username_flag",       CONNECT_USERNAME_FLAG,      CONNECT_FLAGS,
        pack_in_parent,     NULL,               0,              7,      1,      true,   NA,     false,  0,      NULL},
    {"flags",               CONNECT_FLAGS,              0, // not in protocol - used for allocation of bits
        pack_flags_alloc,   NULL,               NA,             NA,     1,      true,   NA,     false,  0,      NULL},
    {"keep_alive",          CONNECT_KEEP_ALIVE,         0,
        pack_uint16,        NULL,               0,              NA,     NA,     true,   NA,     false,  0,      NULL},
//  Variable Header Properties
//   name                   index                       link
//      pack_fn             unpack_fn           value           bitpos  vlen    exists  id      isalloc blen  buf
    {"property_length",     CONNECT_PROPERTY_LENGTH,    CONNECT_AUTHENTICATION_DATA,
        pack_VBI,           NULL,               0,              NA,     0,      true,   NA,     false,  0,      NULL},
    {"session_expiry",      CONNECT_SESSION_EXPIRY,     0,
        pack_sprop_uint32,  NULL,               0,              NA,     NA,     false,  0x11,   false,  0,      NULL},
    {"receive_maximum",     CONNECT_RECEIVE_MAXIMUM,    0,
        pack_sprop_uint16,  NULL,               0,              NA,     NA,     false,  0x21,   false,  0,      NULL},
    {"maximum_packet_size", CONNECT_MAXIMUM_PACKET_SIZE,0,
        pack_sprop_uint32,  NULL,               0,              NA,     NA,     false,  0x27,   false,  0,      NULL},
    {"topic_alias_maximum", CONNECT_TOPIC_ALIAS_MAXIMUM,0,
        pack_sprop_uint16,  NULL,               0,              NA,     NA,     false,  0x22,   false,  0,      NULL},
    {"request_response_information",
                            CONNECT_REQUEST_RESPONSE_INFORMATION,
                                                        0,
        pack_sprop_uint8,   NULL,               0,              NA,     NA,     false,  0x19,   false,  0,      NULL},
    {"request_problem_information",
                            CONNECT_REQUEST_PROBLEM_INFORMATION,
                                                        0,
        pack_sprop_uint8,   NULL,               0,              NA,     NA,     false,  0x17,   false,  0,      NULL},
    {"user_properties",     CONNECT_USER_PROPERTIES,    0,
        pack_mprop_strpair, NULL,               (Word_t)NULL,   NA,     0,      false,  0x26,   false,  0,      NULL},
    {"authentication_method",
                            CONNECT_AUTHENTICATION_METHOD,
                                                        0,
        pack_sprop_str,     NULL,               (Word_t)NULL,   NA,     0,      false,  0x15,   false,  0,      NULL},
    {"authentication_data", CONNECT_AUTHENTICATION_DATA,0,
        pack_sprop_char_buf,NULL,               (Word_t)NULL,   NA,     0,      false,  0x16,   false,  0,      NULL},
// Payload
//   name                   index                       link
//      pack_fn             unpack_fn           value           bitpos  vlen    exists  id      isalloc blen  buf
    {"client_identifier",   CONNECT_CLIENT_IDENTIFIER,  0,
        pack_str,           NULL,               (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL},
// Payload Will Properties
    {"will_property_length",
                            CONNECT_WILL_PROPERTY_LENGTH,
                                                        CONNECT_WILL_USER_PROPERTIES,
        pack_VBI,           NULL,               0,              NA,     0,      false,  NA,     false,  0,      NULL},
    {"will_delay_interval", CONNECT_WILL_DELAY_INTERVAL,0,
        pack_sprop_uint32,  NULL,               0,              NA,     NA,     false,  0x18,   false,  0,      NULL},
    {"payload_format_indicator",
                            CONNECT_PAYLOAD_FORMAT_INDICATOR,
                                                        0,
        pack_sprop_uint8,   NULL,               0,              NA,     NA,     false,  0x01,   false,  0,      NULL},
    {"message_expiry_interval",
                            CONNECT_MESSAGE_EXPIRY_INTERVAL,
                                                        0,
        pack_sprop_uint32,  NULL,               0,              NA,     NA,     false,  0x02,   false,  0,      NULL},
    {"content_type",        CONNECT_CONTENT_TYPE,       0,
        pack_sprop_str,     NULL,               (Word_t)NULL,   NA,     0,      false,  0x03,   false,  0,      NULL},
    {"response_topic",      CONNECT_RESPONSE_TOPIC,     0,
        pack_sprop_str,     NULL,               (Word_t)NULL,   NA,     0,      false,  0x08,   false,  0,      NULL},
    {"correlation_data",    CONNECT_CORRELATION_DATA,   0,
        pack_sprop_char_buf,NULL,               (Word_t)NULL,   NA,     0,      false,  0x09,   false,  0,      NULL},
    {"will_user_properties",
                            CONNECT_WILL_USER_PROPERTIES,
                                                        0,
        pack_mprop_strpair, NULL,               (Word_t)NULL,   NA,     0,      false,  0x26,   false,  0,      NULL},
// Payload (remainder)
    {"will_topic",          CONNECT_WILL_TOPIC,         0,
        pack_str,           NULL,               (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL},
    {"will_payload",        CONNECT_WILL_PAYLOAD,       0,
        pack_char_buf,      NULL,               (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL},
    {"user_name",           CONNECT_USER_NAME,          0,
        pack_str,           NULL,               (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL},
    {"password",            CONNECT_PASSWORD,           0,
        pack_char_buf,      NULL,               (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL}
};

int init_connect_pctx(packet_ctx **Ppctx) {
    size_t mdata_count = sizeof(CONNECT_MDATA_TEMPLATE) / sizeof(mr_mdata);
    return init_packet_context(Ppctx, CONNECT_MDATA_TEMPLATE, mdata_count);
}

int pack_connect_buffer(packet_ctx *pctx) {
    return pack_mdata_buffer(pctx);
}

int unpack_connect_buffer(packet_ctx *pctx) {
    return unpack_mdata_buffer(pctx);
}

int free_connect_pctx(packet_ctx *pctx) {
    return free_packet_context(pctx);
}

//    const uint8_t packet_type;
int get_connect_packet_type(packet_ctx *pctx, uint8_t *Puint8) {
    return get_uint8_scalar(pctx, CONNECT_PACKET_TYPE, Puint8);
}

//    uint32_t remaining_length;
int get_connect_remaining_length(packet_ctx *pctx, uint32_t *Puint32) {
    return get_uint32_scalar(pctx, CONNECT_REMAINING_LENGTH, Puint32);
}

//    const uint8_t *protocol_name;
int get_connect_protocol_name(packet_ctx *pctx, uint8_t **Puint80, size_t *Plen) {
    return get_uint8_vector(pctx, CONNECT_PROTOCOL_NAME, Puint80, Plen);
}

//    const uint8_t protocol_version;
int get_connect_protocol_version(packet_ctx *pctx, uint8_t *Puint8) {
    return get_uint8_scalar(pctx, CONNECT_PROTOCOL_VERSION, Puint8);
}

//    const bool reserved;
int get_connect_reserved(packet_ctx *pctx, bool *Pboolean) {
    return get_boolean_scalar(pctx, CONNECT_RESERVED, Pboolean);
}

//    bool clean_start;
int set_connect_clean_start(packet_ctx *pctx, bool boolean) {
    return set_scalar(pctx, CONNECT_CLEAN_START, boolean);
}

int get_connect_clean_start(packet_ctx *pctx, bool *Pboolean) {
    return get_boolean_scalar(pctx, CONNECT_CLEAN_START, Pboolean);
}

int reset_connect_clean_start(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_CLEAN_START);
}

//    bool will_flag;
int set_connect_will_flag(packet_ctx *pctx, bool boolean) {
    return set_scalar(pctx, CONNECT_WILL_FLAG, boolean);
}

int get_connect_will_flag(packet_ctx *pctx, bool *Pboolean) {
    return get_boolean_scalar(pctx, CONNECT_WILL_FLAG, Pboolean);
}

int reset_connect_will_flag(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_WILL_FLAG);
}

//    uint8_t will_qos;
int set_connect_will_qos(packet_ctx *pctx, uint8_t uint8) {
    return set_scalar(pctx, CONNECT_WILL_QOS, uint8);
}

int get_connect_will_qos(packet_ctx *pctx, uint8_t *Puint8) {
    return get_uint8_scalar(pctx, CONNECT_WILL_QOS, Puint8);
}

int reset_connect_will_qos(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_WILL_QOS);
}

//    bool will_retain;
int set_connect_will_retain(packet_ctx *pctx, bool boolean) {
    return set_scalar(pctx, CONNECT_WILL_RETAIN, boolean);
}

int get_connect_will_retain(packet_ctx *pctx, bool *Pboolean) {
    return get_boolean_scalar(pctx, CONNECT_WILL_RETAIN, Pboolean);
}

int reset_connect_will_retain(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_WILL_RETAIN);
}

//    bool password_flag;
int set_connect_password_flag(packet_ctx *pctx, bool boolean) {
    return set_scalar(pctx, CONNECT_PASSWORD_FLAG, boolean);
}

int get_connect_password_flag(packet_ctx *pctx, bool *Pboolean) {
    return get_boolean_scalar(pctx, CONNECT_PASSWORD_FLAG, Pboolean);
}

int reset_connect_password_flag(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_PASSWORD_FLAG);
}

//    bool username_flag;
int set_connect_username_flag(packet_ctx *pctx, bool boolean) {
    return set_scalar(pctx, CONNECT_USERNAME_FLAG, boolean);
}

int get_connect_username_flag(packet_ctx *pctx, bool *Pboolean) {
    return get_boolean_scalar(pctx, CONNECT_USERNAME_FLAG, Pboolean);
}

int reset_connect_username_flag(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_USERNAME_FLAG);
}

//    uint16_t keep_alive;
int set_connect_keep_alive(packet_ctx *pctx, uint16_t uint16) {
    return set_scalar(pctx, CONNECT_KEEP_ALIVE, uint16);
}

int get_connect_keep_alive(packet_ctx *pctx, uint16_t *Puint16) {
    return get_uint16_scalar(pctx, CONNECT_KEEP_ALIVE, Puint16);
}

int reset_connect_keep_alive(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_KEEP_ALIVE);
}

//    uint32_t property_length;
int get_connect_property_length(packet_ctx *pctx, uint32_t *Puint32) {
    return get_uint32_scalar(pctx, CONNECT_PROPERTY_LENGTH, Puint32);
}

//    uint32_t session_expiry;
int set_connect_session_expiry(packet_ctx *pctx, uint32_t uint32) {
    return set_scalar(pctx, CONNECT_SESSION_EXPIRY, uint32);
}

int get_connect_session_expiry(packet_ctx *pctx, uint32_t *Puint32) {
    return get_uint32_scalar(pctx, CONNECT_SESSION_EXPIRY, Puint32);
}

int reset_connect_session_expiry(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_SESSION_EXPIRY);
}

//    uint16_t receive_maximum;
int set_connect_receive_maximum(packet_ctx *pctx, uint16_t uint16) {
    return set_scalar(pctx, CONNECT_RECEIVE_MAXIMUM, uint16);
}

int get_connect_receive_maximum(packet_ctx *pctx, uint16_t *Puint16) {
    return get_uint16_scalar(pctx, CONNECT_RECEIVE_MAXIMUM, Puint16);
}

int reset_connect_receive_maximum(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_RECEIVE_MAXIMUM);
}

//    uint32_t maximum_packet_size;
int set_connect_maximum_packet_size(packet_ctx *pctx, uint32_t uint32) {
    return set_scalar(pctx, CONNECT_MAXIMUM_PACKET_SIZE, uint32);
}

int get_connect_maximum_packet_size(packet_ctx *pctx, uint32_t *Puint32) {
    return get_uint32_scalar(pctx, CONNECT_MAXIMUM_PACKET_SIZE, Puint32);
}

int reset_connect_maximum_packet_size(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_MAXIMUM_PACKET_SIZE);
}

//    uint16_t topic_alias_maximum;
int set_connect_topic_alias_maximum(packet_ctx *pctx, uint16_t uint16) {
    return set_scalar(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM, uint16);
}

int get_connect_topic_alias_maximum(packet_ctx *pctx, uint16_t *Puint16) {
    return get_uint16_scalar(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM, Puint16);
}

int reset_connect_topic_alias_maximum(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM);
}

//    uint8_t request_response_information;
int set_connect_request_response_information(packet_ctx *pctx, uint8_t uint8) {
    return set_scalar(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION, uint8);
}

int get_connect_request_response_information(packet_ctx *pctx, uint8_t *Puint8) {
    return get_uint8_scalar(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION, Puint8);
}

int reset_connect_request_response_information(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION);
}

//    uint8_t request_problem_information;
int set_connect_request_problem_information(packet_ctx *pctx, uint8_t uint8) {
    return set_scalar(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION, uint8);
}

int get_connect_request_problem_information(packet_ctx *pctx, uint8_t *Puint8) {
    return get_uint8_scalar(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION, Puint8);
}

int reset_connect_request_problem_information(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION);
}

//    string_pair *user_properties;
int set_connect_user_properties(packet_ctx *pctx, string_pair *sp0, size_t len) {
    return set_vector(pctx, CONNECT_USER_PROPERTIES, sp0, len);
}

int get_connect_user_properties(packet_ctx *pctx, string_pair **Psp0, size_t *Plen) {
    return get_string_pair_vector(pctx, CONNECT_USER_PROPERTIES, Psp0, Plen);
}

int reset_connect_user_properties(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_USER_PROPERTIES);
}

//    uint8_t *authentication_method;
int set_connect_authentication_method(packet_ctx *pctx, uint8_t *uint80, size_t len) {
    return set_vector(pctx, CONNECT_AUTHENTICATION_METHOD, uint80, len);
}

int get_connect_authentication_method(packet_ctx *pctx, uint8_t **Puint80, size_t *Plen) {
    return get_uint8_vector(pctx, CONNECT_AUTHENTICATION_METHOD, Puint80, Plen);
}

int reset_connect_authentication_method(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_AUTHENTICATION_METHOD);
}

//    uint8_t *authentication_data;
int set_connect_authentication_data(packet_ctx *pctx, uint8_t *uint80, size_t len) {
    return set_vector(pctx, CONNECT_AUTHENTICATION_DATA, uint80, len);
}

int get_connect_authentication_data(packet_ctx *pctx, uint8_t **Puint80, size_t *Plen) {
    return get_uint8_vector(pctx, CONNECT_AUTHENTICATION_DATA, Puint80, Plen);
}

int reset_connect_authentication_data(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_AUTHENTICATION_DATA);
}

//    uint8_t *client_identifier;
int set_connect_client_identifier(packet_ctx *pctx, uint8_t *uint80, size_t len) {
    return set_vector(pctx, CONNECT_CLIENT_IDENTIFIER, uint80, len);
}

int get_connect_client_identifier(packet_ctx *pctx, uint8_t **Puint80, size_t *Plen) {
    return get_uint8_vector(pctx, CONNECT_CLIENT_IDENTIFIER, Puint80, Plen);
}

int reset_connect_client_identifier(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_CLIENT_IDENTIFIER);
}

//    uint32_t will_property_length;
int get_will_property_length(packet_ctx *pctx, uint32_t *Puint32) {
    return get_uint32_scalar(pctx, CONNECT_WILL_PROPERTY_LENGTH, Puint32);
}

//    uint32_t will_delay_interval;
int set_connect_will_delay_interval(packet_ctx *pctx, uint32_t uint32) {
    return set_scalar(pctx, CONNECT_WILL_DELAY_INTERVAL, uint32);
}

int get_connect_will_delay_interval(packet_ctx *pctx, uint32_t *Puint32) {
    return get_uint32_scalar(pctx, CONNECT_WILL_DELAY_INTERVAL, Puint32);
}

int reset_connect_will_delay_interval(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_WILL_DELAY_INTERVAL);
}

//    uint8_t payload_format_indicator;
int set_connect_payload_format_indicator(packet_ctx *pctx, uint8_t uint8) {
    return set_scalar(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, uint8);
}

int get_connect_payload_format_indicator(packet_ctx *pctx, uint8_t *Puint8) {
    return get_uint8_scalar(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, Puint8);
}

int reset_connect_payload_format_indicator(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR);
}

//    uint32_t message_expiry_interval;
int set_connect_message_expiry_interval(packet_ctx *pctx, uint32_t uint32) {
    return set_scalar(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, uint32);
}

int get_connect_message_expiry_interval(packet_ctx *pctx, uint32_t *Puint32) {
    return get_uint32_scalar(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, Puint32);
}

int reset_connect_message_expiry_interval(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL);
}

//    uint8_t *content_type;
int set_connect_content_type(packet_ctx *pctx, uint8_t *uint80, size_t len) {
    return set_vector(pctx, CONNECT_CONTENT_TYPE, uint80, len);
}

int get_connect_content_type(packet_ctx *pctx, uint8_t **Puint80, size_t *Plen) {
    return get_uint8_vector(pctx, CONNECT_CONTENT_TYPE, Puint80, Plen);
}

int reset_connect_content_type(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_CONTENT_TYPE);
}

//    uint8_t *response_topic;
int set_connect_response_topic(packet_ctx *pctx, uint8_t *uint80, size_t len) {
    return set_vector(pctx, CONNECT_RESPONSE_TOPIC, uint80, len);
}

int get_connect_response_topic(packet_ctx *pctx, uint8_t **Puint80, size_t *Plen) {
    return get_uint8_vector(pctx, CONNECT_RESPONSE_TOPIC, Puint80, Plen);
}

int reset_connect_response_topic(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_RESPONSE_TOPIC);
}

//    uint8_t *correlation_data;
int set_connect_correlation_data(packet_ctx *pctx, uint8_t *uint80, size_t len) {
    return set_vector(pctx, CONNECT_CORRELATION_DATA, uint80, len);
}

int get_connect_correlation_data(packet_ctx *pctx, uint8_t **Puint80, size_t *Plen) {
    return get_uint8_vector(pctx, CONNECT_CORRELATION_DATA, Puint80, Plen);
}

int reset_connect_correlation_data(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_CORRELATION_DATA);
}

//    string_pair *will_user_properties;
int set_connect_will_user_properties(packet_ctx *pctx, string_pair *sp0, size_t len) {
    return set_vector(pctx, CONNECT_WILL_USER_PROPERTIES, sp0, len);
}

int get_connect_will_user_properties(packet_ctx *pctx, string_pair **Psp0, size_t *Plen) {
    return get_string_pair_vector(pctx, CONNECT_WILL_USER_PROPERTIES, Psp0, Plen);
}

int reset_connect_will_user_properties(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_WILL_USER_PROPERTIES);
}

//    uint8_t *will_topic;
int set_connect_will_topic(packet_ctx *pctx, uint8_t *uint80, size_t len) {
    return set_vector(pctx, CONNECT_WILL_TOPIC, uint80, len);
}

int get_connect_will_topic(packet_ctx *pctx, uint8_t **Puint80, size_t *Plen) {
    return get_uint8_vector(pctx, CONNECT_WILL_TOPIC, Puint80, Plen);
}

int reset_connect_will_topic(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_WILL_TOPIC);
}

//    uint8_t *will_payload;
int set_connect_will_payload(packet_ctx *pctx, uint8_t *uint80, size_t len) {
    return set_vector(pctx, CONNECT_WILL_PAYLOAD, uint80, len);
}

int get_connect_will_payload(packet_ctx *pctx, uint8_t **Puint80, size_t *Plen) {
    return get_uint8_vector(pctx, CONNECT_WILL_PAYLOAD, Puint80, Plen);
}

int reset_connect_will_payload(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_WILL_PAYLOAD);
}

//    uint8_t *user_name;
int set_connect_user_name(packet_ctx *pctx, uint8_t *uint80, size_t len) {
    return set_vector(pctx, CONNECT_USER_NAME, uint80, len);
}

int get_connect_user_name(packet_ctx *pctx, uint8_t **Puint80, size_t *Plen) {
    return get_uint8_vector(pctx, CONNECT_USER_NAME, Puint80, Plen);
}

int reset_connect_user_name(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_USER_NAME);
}

//    uint8_t *password;
int set_connect_password(packet_ctx *pctx, uint8_t *uint80, size_t len) {
    return set_vector(pctx, CONNECT_PASSWORD, uint80, len);
}

int get_connect_password(packet_ctx *pctx, uint8_t **Puint80, size_t *Plen) {
    return get_uint8_vector(pctx, CONNECT_PASSWORD, Puint80, Plen);
}

int reset_connect_password(packet_ctx *pctx) {
    return reset_value(pctx, CONNECT_PASSWORD);
}
