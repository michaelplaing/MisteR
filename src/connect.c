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
//   name                   index                       link                isprop  pack_fn             unpack_fn           value           bitpos  vlen    exists  id      isalloc u8vlen  u8v0
    {"packet_type",         CONNECT_PACKET_TYPE,        0,                  false,  mr_pack_u8,         mr_unpack_u8,       MQTT_CONNECT,   NA,     NA,     true,   NA,     false,  0,      NULL},
    {"remaining_length",    CONNECT_REMAINING_LENGTH,   CONNECT_PASSWORD,   false,  mr_pack_VBI,        mr_unpack_VBI,      0,              NA,     0,      true,   NA,     false,  0,      NULL},
//  Var
//   name                   index                       link                isprop  pack_fn             unpack_fn           value           bitpos  vlen    exists  id      isalloc u8vlen  u8v0
    {"protocol_name",       CONNECT_PROTOCOL_NAME,      0,                  false,  mr_pack_u8v,        mr_unpack_u8v,      (Word_t)PNM,    NA,     PNMSZ,  true,   NA,     false,  0,      NULL},
    {"protocol_version",    CONNECT_PROTOCOL_VERSION,   0,                  false,  mr_pack_u8,         mr_unpack_u8,       PROTO_VERSION,  NA,     NA,     true,   NA,     false,  0,      NULL},
    {"reserved",            CONNECT_RESERVED,           CONNECT_FLAGS,      false,  mr_pack_bits,       mr_unpack_bits,     0,              0,      1,      true,   NA,     false,  0,      NULL},
    {"clean_start",         CONNECT_CLEAN_START,        CONNECT_FLAGS,      false,  mr_pack_bits,       mr_unpack_bits,     0,              1,      1,      true,   NA,     false,  0,      NULL},
    {"will_flag",           CONNECT_WILL_FLAG,          CONNECT_FLAGS,      false,  mr_pack_bits,       mr_unpack_bits,     0,              2,      1,      true,   NA,     false,  0,      NULL},
    {"will_qos",            CONNECT_WILL_QOS,           CONNECT_FLAGS,      false,  mr_pack_bits,       mr_unpack_bits,     0,              3,      2,      true,   NA,     false,  0,      NULL},
    {"will_retain",         CONNECT_WILL_RETAIN,        CONNECT_FLAGS,      false,  mr_pack_bits,       mr_unpack_bits,     0,              5,      1,      true,   NA,     false,  0,      NULL},
    {"password_flag",       CONNECT_PASSWORD_FLAG,      CONNECT_FLAGS,      false,  mr_pack_bits,       mr_unpack_bits,     0,              6,      1,      true,   NA,     false,  0,      NULL},
    {"username_flag",       CONNECT_USERNAME_FLAG,      CONNECT_FLAGS,      false,  mr_pack_bits,       mr_unpack_bits,     0,              7,      1,      true,   NA,     false,  0,      NULL},
    {"flags",               CONNECT_FLAGS,              0,                  false,  mr_pack_u8,         mr_unpack_noop,     NA,             NA,     1,      true,   NA,     false,  0,      NULL},
    {"keep_alive",          CONNECT_KEEP_ALIVE,         0,                  false,  mr_pack_u16,        mr_unpack_u16,      0,              NA,     NA,     true,   NA,     false,  0,      NULL},
//  Variable Header Properties
//   name                   index                       link                isprop  pack_fn             unpack_fn           value           bitpos  vlen    exists  id                                      isalloc u8vlen  u8v0
    {"property_length",     CONNECT_PROPERTY_LENGTH,    CONNECT_AUTHENTICATION_DATA,
                                                                            false,  mr_pack_VBI,        mr_unpack_prop_VBI, 0,              NA,     0,      true,   NA,                                     false,  0,      NULL},
    {"session_expiry_interval",
                            CONNECT_SESSION_EXPIRY_INTERVAL,
                                                        0,                  true,   mr_pack_prop_u32,   mr_unpack_prop_u32, 0,              NA,     NA,     false,  MQTT_PROP_SESSION_EXPIRY_INTERVAL,      false,  0,      NULL},
    {"receive_maximum",     CONNECT_RECEIVE_MAXIMUM,    0,                  true,   mr_pack_prop_u16,   NULL,               0,              NA,     NA,     false,  MQTT_PROP_RECEIVE_MAXIMUM,              false,  0,      NULL},
    {"maximum_packet_size", CONNECT_MAXIMUM_PACKET_SIZE,0,                  true,   mr_pack_prop_u32,   NULL,               0,              NA,     NA,     false,  MQTT_PROP_MAXIMUM_PACKET_SIZE,          false,  0,      NULL},
    {"topic_alias_maximum", CONNECT_TOPIC_ALIAS_MAXIMUM,0,                  true,   mr_pack_prop_u16,   NULL,               0,              NA,     NA,     false,  MQTT_PROP_TOPIC_ALIAS_MAXIMUM,          false,  0,      NULL},
    {"request_response_information",
                            CONNECT_REQUEST_RESPONSE_INFORMATION,
                                                        0,                  true,   mr_pack_prop_u8,    NULL,               0,              NA,     NA,     false,  MQTT_PROP_REQUEST_RESPONSE_INFORMATION, false,  0,      NULL},
    {"request_problem_information",
                            CONNECT_REQUEST_PROBLEM_INFORMATION,
                                                        0,                  true,   mr_pack_prop_u8,    NULL,               0,              NA,     NA,     false,  MQTT_PROP_REQUEST_PROBLEM_INFORMATION,  false,  0,      NULL},
    {"user_properties",     CONNECT_USER_PROPERTIES,    0,                  true,   mr_pack_prop_spv,   NULL,               (Word_t)NULL,   NA,     0,      false,  MQTT_PROP_USER_PROPERTY,                false,  0,      NULL},
    {"authentication_method",
                            CONNECT_AUTHENTICATION_METHOD,
                                                        0,                  true,   mr_pack_prop_str,   NULL,               (Word_t)NULL,   NA,     0,      false,  MQTT_PROP_AUTHENTICATION_METHOD,        false,  0,      NULL},
    {"authentication_data", CONNECT_AUTHENTICATION_DATA,0,                  true,   mr_pack_prop_u8v,   NULL,               (Word_t)NULL,   NA,     0,      false,  MQTT_PROP_AUTHENTICATION_DATA,          false,  0,      NULL},
// Payload
//   name                   index                       link                isprop  pack_fn             unpack_fn           value           bitpos  vlen    exists  id      isalloc u8vlen  u8v0
    {"client_identifier",   CONNECT_CLIENT_IDENTIFIER,  0,                  false,  mr_pack_str,        NULL,               (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL},
// Payload Will Properties
//   name                   index                       link                isprop  pack_fn             unpack_fn           value           bitpos  vlen    exists  id                                      isalloc u8vlen  u8v0
    {"will_property_length",
                            CONNECT_WILL_PROPERTY_LENGTH,
                                                        CONNECT_WILL_USER_PROPERTIES,
                                                                            false,  mr_pack_VBI,        NULL,               0,              NA,     0,      false,  NA,                                     false,  0,      NULL},
    {"will_delay_interval", CONNECT_WILL_DELAY_INTERVAL,0,                  true,   mr_pack_prop_u32,   NULL,               0,              NA,     NA,     false,  MQTT_PROP_WILL_DELAY_INTERVAL,          false,  0,      NULL},
    {"payload_format_indicator",
                            CONNECT_PAYLOAD_FORMAT_INDICATOR,
                                                        0,                  true,   mr_pack_prop_u8,    NULL,               0,              NA,     NA,     false,  MQTT_PROP_PAYLOAD_FORMAT_INDICATOR,     false,  0,      NULL},
    {"message_expiry_interval",
                            CONNECT_MESSAGE_EXPIRY_INTERVAL,
                                                        0,                  true,   mr_pack_prop_u32,   NULL,               0,              NA,     NA,     false,  MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,      false,  0,      NULL},
    {"content_type",        CONNECT_CONTENT_TYPE,       0,                  true,   mr_pack_prop_str,   NULL,               (Word_t)NULL,   NA,     0,      false,  MQTT_PROP_CONTENT_TYPE,                 false,  0,      NULL},
    {"response_topic",      CONNECT_RESPONSE_TOPIC,     0,                  true,   mr_pack_prop_str,   NULL,               (Word_t)NULL,   NA,     0,      false,  MQTT_PROP_RESPONSE_TOPIC,               false,  0,      NULL},
    {"correlation_data",    CONNECT_CORRELATION_DATA,   0,                  true,   mr_pack_prop_u8v,   NULL,               (Word_t)NULL,   NA,     0,      false,  MQTT_PROP_CORRELATION_DATA,             false,  0,      NULL},
    {"will_user_properties",CONNECT_WILL_USER_PROPERTIES,
                                                        0,                  true,   mr_pack_prop_spv,   NULL,               (Word_t)NULL,   NA,     0,      false,  MQTT_PROP_RESPONSE_INFORMATION,         false,  0,      NULL},
// Payload (remainder)
//   name                   index                       link                isprop  pack_fn             unpack_fn           value           bitpos  vlen    exists  id      isalloc u8vlen  u8v0
    {"will_topic",          CONNECT_WILL_TOPIC,         0,                  false,  mr_pack_str,        NULL,               (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL},
    {"will_payload",        CONNECT_WILL_PAYLOAD,       0,                  false,  mr_pack_u8v,        NULL,               (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL},
    {"user_name",           CONNECT_USER_NAME,          0,                  false,  mr_pack_str,        NULL,               (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL},
    {"password",            CONNECT_PASSWORD,           0,                  false,  mr_pack_u8v,        NULL,               (Word_t)NULL,   NA,     0,      false,  NA,     false,  0,      NULL}
};

int mr_init_connect_pctx(packet_ctx **Ppctx) {
    size_t mdata_count = sizeof(CONNECT_MDATA_TEMPLATE) / sizeof(mr_mdata);
    return mr_init_packet_context(Ppctx, CONNECT_MDATA_TEMPLATE, mdata_count);
}

int mr_pack_connect_u8v0(packet_ctx *pctx) {
    return mr_pack_mdata_u8v0(pctx);
}

int mr_unpack_connect_u8v0(packet_ctx *pctx) {
    return mr_unpack_mdata_u8v0(pctx);
}

int mr_free_connect_pctx(packet_ctx *pctx) {
    return mr_free_packet_context(pctx);
}

//    const uint8_t packet_type;
int mr_get_connect_packet_type(packet_ctx *pctx, uint8_t *pu8) {
    return mr_get_u8(pctx, CONNECT_PACKET_TYPE, pu8);
}

//    uint32_t remaining_length;
int mr_get_connect_remaining_length(packet_ctx *pctx, uint32_t *pu32) {
    return mr_get_u32(pctx, CONNECT_REMAINING_LENGTH, pu32);
}

//    const uint8_t *protocol_name;
int mr_get_connect_protocol_name(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    return mr_get_u8v(pctx, CONNECT_PROTOCOL_NAME, pu8v0, plen);
}

//    const uint8_t protocol_version;
int mr_get_connect_protocol_version(packet_ctx *pctx, uint8_t *pu8) {
    return mr_get_u8(pctx, CONNECT_PROTOCOL_VERSION, pu8);
}

//    const bool reserved;
int mr_get_connect_reserved(packet_ctx *pctx, bool *pboolean) {
    return mr_get_boolean(pctx, CONNECT_RESERVED, pboolean);
}

//    bool clean_start;
int mr_set_connect_clean_start(packet_ctx *pctx, bool boolean) {
    return mr_set_scalar(pctx, CONNECT_CLEAN_START, boolean);
}

int mr_get_connect_clean_start(packet_ctx *pctx, bool *pboolean) {
    return mr_get_boolean(pctx, CONNECT_CLEAN_START, pboolean);
}

int mr_reset_connect_clean_start(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_CLEAN_START);
}

//    bool will_flag;
int mr_set_connect_will_flag(packet_ctx *pctx, bool boolean) {
    return mr_set_scalar(pctx, CONNECT_WILL_FLAG, boolean);
}

int mr_get_connect_will_flag(packet_ctx *pctx, bool *pboolean) {
    return mr_get_boolean(pctx, CONNECT_WILL_FLAG, pboolean);
}

int mr_reset_connect_will_flag(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_WILL_FLAG);
}

//    uint8_t will_qos;
int mr_set_connect_will_qos(packet_ctx *pctx, uint8_t u8) {
    return mr_set_scalar(pctx, CONNECT_WILL_QOS, u8);
}

int mr_get_connect_will_qos(packet_ctx *pctx, uint8_t *pu8) {
    return mr_get_u8(pctx, CONNECT_WILL_QOS, pu8);
}

int mr_reset_connect_will_qos(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_WILL_QOS);
}

//    bool will_retain;
int mr_set_connect_will_retain(packet_ctx *pctx, bool boolean) {
    return mr_set_scalar(pctx, CONNECT_WILL_RETAIN, boolean);
}

int mr_get_connect_will_retain(packet_ctx *pctx, bool *pboolean) {
    return mr_get_boolean(pctx, CONNECT_WILL_RETAIN, pboolean);
}

int mr_reset_connect_will_retain(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_WILL_RETAIN);
}

//    bool password_flag;
int mr_set_connect_password_flag(packet_ctx *pctx, bool boolean) {
    return mr_set_scalar(pctx, CONNECT_PASSWORD_FLAG, boolean);
}

int mr_get_connect_password_flag(packet_ctx *pctx, bool *pboolean) {
    return mr_get_boolean(pctx, CONNECT_PASSWORD_FLAG, pboolean);
}

int mr_reset_connect_password_flag(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_PASSWORD_FLAG);
}

//    bool username_flag;
int mr_set_connect_username_flag(packet_ctx *pctx, bool boolean) {
    return mr_set_scalar(pctx, CONNECT_USERNAME_FLAG, boolean);
}

int mr_get_connect_username_flag(packet_ctx *pctx, bool *pboolean) {
    return mr_get_boolean(pctx, CONNECT_USERNAME_FLAG, pboolean);
}

int mr_reset_connect_username_flag(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_USERNAME_FLAG);
}

//    uint16_t keep_alive;
int mr_set_connect_keep_alive(packet_ctx *pctx, uint16_t u16) {
    return mr_set_scalar(pctx, CONNECT_KEEP_ALIVE, u16);
}

int mr_get_connect_keep_alive(packet_ctx *pctx, uint16_t *pu16) {
    return mr_get_u16(pctx, CONNECT_KEEP_ALIVE, pu16);
}

int mr_reset_connect_keep_alive(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_KEEP_ALIVE);
}

//    uint32_t property_length;
int mr_get_connect_property_length(packet_ctx *pctx, uint32_t *pu32) {
    return mr_get_u32(pctx, CONNECT_PROPERTY_LENGTH, pu32);
}

//    uint32_t session_expiry;
int mr_set_connect_session_expiry_interval(packet_ctx *pctx, uint32_t u32) {
    return mr_set_scalar(pctx, CONNECT_SESSION_EXPIRY_INTERVAL, u32);
}

int mr_get_connect_session_expiry_interval(packet_ctx *pctx, uint32_t *pu32) {
    return mr_get_u32(pctx, CONNECT_SESSION_EXPIRY_INTERVAL, pu32);
}

int mr_reset_connect_session_expiry_interval(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_SESSION_EXPIRY_INTERVAL);
}

//    uint16_t receive_maximum;
int mr_set_connect_receive_maximum(packet_ctx *pctx, uint16_t u16) {
    return mr_set_scalar(pctx, CONNECT_RECEIVE_MAXIMUM, u16);
}

int mr_get_connect_receive_maximum(packet_ctx *pctx, uint16_t *pu16) {
    return mr_get_u16(pctx, CONNECT_RECEIVE_MAXIMUM, pu16);
}

int mr_reset_connect_receive_maximum(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_RECEIVE_MAXIMUM);
}

//    uint32_t maximum_packet_size;
int mr_set_connect_maximum_packet_size(packet_ctx *pctx, uint32_t u32) {
    return mr_set_scalar(pctx, CONNECT_MAXIMUM_PACKET_SIZE, u32);
}

int mr_get_connect_maximum_packet_size(packet_ctx *pctx, uint32_t *pu32) {
    return mr_get_u32(pctx, CONNECT_MAXIMUM_PACKET_SIZE, pu32);
}

int mr_reset_connect_maximum_packet_size(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_MAXIMUM_PACKET_SIZE);
}

//    uint16_t topic_alias_maximum;
int mr_set_connect_topic_alias_maximum(packet_ctx *pctx, uint16_t u16) {
    return mr_set_scalar(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM, u16);
}

int mr_get_connect_topic_alias_maximum(packet_ctx *pctx, uint16_t *pu16) {
    return mr_get_u16(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM, pu16);
}

int mr_reset_connect_topic_alias_maximum(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM);
}

//    uint8_t request_response_information;
int mr_set_connect_request_response_information(packet_ctx *pctx, uint8_t u8) {
    return mr_set_scalar(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION, u8);
}

int mr_get_connect_request_response_information(packet_ctx *pctx, uint8_t *pu8) {
    return mr_get_u8(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION, pu8);
}

int mr_reset_connect_request_response_information(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION);
}

//    uint8_t request_problem_information;
int mr_set_connect_request_problem_information(packet_ctx *pctx, uint8_t u8) {
    return mr_set_scalar(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION, u8);
}

int mr_get_connect_request_problem_information(packet_ctx *pctx, uint8_t *pu8) {
    return mr_get_u8(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION, pu8);
}

int mr_reset_connect_request_problem_information(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION);
}

//    string_pair *user_properties;
int mr_set_connect_user_properties(packet_ctx *pctx, string_pair *spv0, size_t len) {
    return mr_set_vector(pctx, CONNECT_USER_PROPERTIES, spv0, len);
}

int mr_get_connect_user_properties(packet_ctx *pctx, string_pair **pspv0, size_t *plen) {
    return mr_get_spv(pctx, CONNECT_USER_PROPERTIES, pspv0, plen);
}

int mr_reset_connect_user_properties(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_USER_PROPERTIES);
}

//    uint8_t *authentication_method;
int mr_set_connect_authentication_method(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    return mr_set_vector(pctx, CONNECT_AUTHENTICATION_METHOD, u8v0, len);
}

int mr_get_connect_authentication_method(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    return mr_get_u8v(pctx, CONNECT_AUTHENTICATION_METHOD, pu8v0, plen);
}

int mr_reset_connect_authentication_method(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_AUTHENTICATION_METHOD);
}

//    uint8_t *authentication_data;
int mr_set_connect_authentication_data(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    return mr_set_vector(pctx, CONNECT_AUTHENTICATION_DATA, u8v0, len);
}

int mr_get_connect_authentication_data(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    return mr_get_u8v(pctx, CONNECT_AUTHENTICATION_DATA, pu8v0, plen);
}

int mr_reset_connect_authentication_data(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_AUTHENTICATION_DATA);
}

//    uint8_t *client_identifier;
int mr_set_connect_client_identifier(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    return mr_set_vector(pctx, CONNECT_CLIENT_IDENTIFIER, u8v0, len);
}

int mr_get_connect_client_identifier(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    return mr_get_u8v(pctx, CONNECT_CLIENT_IDENTIFIER, pu8v0, plen);
}

int mr_reset_connect_client_identifier(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_CLIENT_IDENTIFIER);
}

//    uint32_t will_property_length;
int mr_get_will_property_length(packet_ctx *pctx, uint32_t *pu32) {
    return mr_get_u32(pctx, CONNECT_WILL_PROPERTY_LENGTH, pu32);
}

//    uint32_t will_delay_interval;
int mr_set_connect_will_delay_interval(packet_ctx *pctx, uint32_t u32) {
    return mr_set_scalar(pctx, CONNECT_WILL_DELAY_INTERVAL, u32);
}

int mr_get_connect_will_delay_interval(packet_ctx *pctx, uint32_t *pu32) {
    return mr_get_u32(pctx, CONNECT_WILL_DELAY_INTERVAL, pu32);
}

int mr_reset_connect_will_delay_interval(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_WILL_DELAY_INTERVAL);
}

//    uint8_t payload_format_indicator;
int mr_set_connect_payload_format_indicator(packet_ctx *pctx, uint8_t u8) {
    return mr_set_scalar(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, u8);
}

int mr_get_connect_payload_format_indicator(packet_ctx *pctx, uint8_t *pu8) {
    return mr_get_u8(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, pu8);
}

int mr_reset_connect_payload_format_indicator(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR);
}

//    uint32_t message_expiry_interval;
int mr_set_connect_message_expiry_interval(packet_ctx *pctx, uint32_t u32) {
    return mr_set_scalar(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, u32);
}

int mr_get_connect_message_expiry_interval(packet_ctx *pctx, uint32_t *pu32) {
    return mr_get_u32(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, pu32);
}

int mr_reset_connect_message_expiry_interval(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL);
}

//    uint8_t *content_type;
int mr_set_connect_content_type(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    return mr_set_vector(pctx, CONNECT_CONTENT_TYPE, u8v0, len);
}

int mr_get_connect_content_type(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    return mr_get_u8v(pctx, CONNECT_CONTENT_TYPE, pu8v0, plen);
}

int mr_reset_connect_content_type(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_CONTENT_TYPE);
}

//    uint8_t *response_topic;
int mr_set_connect_response_topic(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    return mr_set_vector(pctx, CONNECT_RESPONSE_TOPIC, u8v0, len);
}

int mr_get_connect_response_topic(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    return mr_get_u8v(pctx, CONNECT_RESPONSE_TOPIC, pu8v0, plen);
}

int mr_reset_connect_response_topic(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_RESPONSE_TOPIC);
}

//    uint8_t *correlation_data;
int mr_set_connect_correlation_data(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    return mr_set_vector(pctx, CONNECT_CORRELATION_DATA, u8v0, len);
}

int mr_get_connect_correlation_data(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    return mr_get_u8v(pctx, CONNECT_CORRELATION_DATA, pu8v0, plen);
}

int mr_reset_connect_correlation_data(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_CORRELATION_DATA);
}

//    string_pair *will_user_properties;
int mr_set_connect_will_user_properties(packet_ctx *pctx, string_pair *spv0, size_t len) {
    return mr_set_vector(pctx, CONNECT_WILL_USER_PROPERTIES, spv0, len);
}

int mr_get_connect_will_user_properties(packet_ctx *pctx, string_pair **pspv0, size_t *plen) {
    return mr_get_spv(pctx, CONNECT_WILL_USER_PROPERTIES, pspv0, plen);
}

int mr_reset_connect_will_user_properties(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_WILL_USER_PROPERTIES);
}

//    uint8_t *will_topic;
int mr_set_connect_will_topic(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    return mr_set_vector(pctx, CONNECT_WILL_TOPIC, u8v0, len);
}

int mr_get_connect_will_topic(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    return mr_get_u8v(pctx, CONNECT_WILL_TOPIC, pu8v0, plen);
}

int mr_reset_connect_will_topic(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_WILL_TOPIC);
}

//    uint8_t *will_payload;
int mr_set_connect_will_payload(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    return mr_set_vector(pctx, CONNECT_WILL_PAYLOAD, u8v0, len);
}

int mr_get_connect_will_payload(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    return mr_get_u8v(pctx, CONNECT_WILL_PAYLOAD, pu8v0, plen);
}

int mr_reset_connect_will_payload(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_WILL_PAYLOAD);
}

//    uint8_t *user_name;
int mr_set_connect_user_name(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    return mr_set_vector(pctx, CONNECT_USER_NAME, u8v0, len);
}

int mr_get_connect_user_name(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    return mr_get_u8v(pctx, CONNECT_USER_NAME, pu8v0, plen);
}

int mr_reset_connect_user_name(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_USER_NAME);
}

//    uint8_t *password;
int mr_set_connect_password(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    return mr_set_vector(pctx, CONNECT_PASSWORD, u8v0, len);
}

int mr_get_connect_password(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    return mr_get_u8v(pctx, CONNECT_PASSWORD, pu8v0, plen);
}

int mr_reset_connect_password(packet_ctx *pctx) {
    return mr_reset_value(pctx, CONNECT_PASSWORD);
}
