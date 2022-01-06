/* connect.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mister/mister.h"
#include "mister/mrzlog.h"
#include "connect_internal.h"
#include "packet_internal.h"

static const uint8_t PNM[] = {'M', 'Q', 'T', 'T'};  // protocol signature
#define PNMSZ 4
#define PROTO_VERSION 5
#define NA 0

static const uint8_t MRCP[] = {
    MQTT_PROP_SESSION_EXPIRY_INTERVAL,
    MQTT_PROP_RECEIVE_MAXIMUM,
    MQTT_PROP_MAXIMUM_PACKET_SIZE,
    MQTT_PROP_TOPIC_ALIAS_MAXIMUM,
    MQTT_PROP_REQUEST_RESPONSE_INFORMATION,
    MQTT_PROP_REQUEST_PROBLEM_INFORMATION,
    MQTT_PROP_USER_PROPERTY,
    MQTT_PROP_AUTHENTICATION_METHOD,
    MQTT_PROP_AUTHENTICATION_DATA
};
#define MRCPSZ 9

static const uint8_t MRCWP[] = {
    MQTT_PROP_WILL_DELAY_INTERVAL,
    MQTT_PROP_PAYLOAD_FORMAT_INDICATOR,
    MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,
    MQTT_PROP_CONTENT_TYPE,
    MQTT_PROP_RESPONSE_TOPIC,
    MQTT_PROP_CORRELATION_DATA,
    MQTT_PROP_RESPONSE_INFORMATION
};
#define MRCWPSZ 7

static const mr_mdata CONNECT_MDATA_TEMPLATE[] = {
//   name                           dtype           bp  value           valloc  vlen    vexists link                            propid                                  flagid                  idx                                     ualloc  u8vlen  u8v0
    {"packet_type",                 MR_U8_DTYPE,    NA, MQTT_CONNECT,   false,  1,      true,   NA,                             NA,                                     NA,                     CONNECT_PACKET_TYPE,                    false,  0,      NULL},
    {"remaining_length",            MR_VBI_DTYPE,   NA, 0,              false,  0,      true,   CONNECT_PASSWORD,               NA,                                     NA,                     CONNECT_REMAINING_LENGTH,               false,  0,      NULL},
    {"protocol_name",               MR_U8V_DTYPE,   NA, (Word_t)PNM,    false,  PNMSZ,  true,   NA,                             NA,                                     NA,                     CONNECT_PROTOCOL_NAME,                  false,  0,      NULL},
    {"protocol_version",            MR_U8_DTYPE,    NA, PROTO_VERSION,  false,  1,      true,   NA,                             NA,                                     NA,                     CONNECT_PROTOCOL_VERSION,               false,  0,      NULL},
    {"reserved",                    MR_BITS_DTYPE,  0,  0,              false,  1,      true,   CONNECT_MR_FLAGS,               NA,                                     NA,                     CONNECT_RESERVED,                       false,  0,      NULL},
    {"clean_start",                 MR_BITS_DTYPE,  1,  0,              false,  1,      true,   CONNECT_MR_FLAGS,               NA,                                     NA,                     CONNECT_CLEAN_START,                    false,  0,      NULL},
    {"will_flag",                   MR_BITS_DTYPE,  2,  0,              false,  1,      true,   CONNECT_MR_FLAGS,               NA,                                     NA,                     CONNECT_WILL_FLAG,                      false,  0,      NULL},
    {"will_qos",                    MR_BITS_DTYPE,  3,  0,              false,  2,      true,   CONNECT_MR_FLAGS,               NA,                                     NA,                     CONNECT_WILL_QOS,                       false,  0,      NULL},
    {"will_retain",                 MR_BITS_DTYPE,  5,  0,              false,  1,      true,   CONNECT_MR_FLAGS,               NA,                                     NA,                     CONNECT_WILL_RETAIN,                    false,  0,      NULL},
    {"password_flag",               MR_BITS_DTYPE,  6,  0,              false,  1,      true,   CONNECT_MR_FLAGS,               NA,                                     NA,                     CONNECT_PASSWORD_FLAG,                  false,  0,      NULL},
    {"username_flag",               MR_BITS_DTYPE,  7,  0,              false,  1,      true,   CONNECT_MR_FLAGS,               NA,                                     NA,                     CONNECT_USERNAME_FLAG,                  false,  0,      NULL},
    {"mr_flags",                    MR_U8_DTYPE,    NA, NA,             false,  1,      true,   NA,                             NA,                                     NA,                     CONNECT_MR_FLAGS,                       false,  0,      NULL},
    {"keep_alive",                  MR_U16_DTYPE,   NA, 0,              false,  2,      true,   NA,                             NA,                                     NA,                     CONNECT_KEEP_ALIVE,                     false,  0,      NULL},
    {"property_length",             MR_VBI_DTYPE,   NA, 0,              false,  0,      true,   CONNECT_AUTHENTICATION_DATA,    NA,                                     NA,                     CONNECT_PROPERTY_LENGTH,                false,  0,      NULL},
    {"mr_properties",               MR_PROPS_DTYPE, NA, (Word_t)MRCP,   false,  MRCPSZ, true,   NA,                             NA,                                     NA,                     CONNECT_MR_PROPERTIES,                  false,  0,      NULL},
    {"session_expiry_interval",     MR_U32_DTYPE,   NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_SESSION_EXPIRY_INTERVAL,      NA,                     CONNECT_SESSION_EXPIRY_INTERVAL,        false,  0,      NULL},
    {"receive_maximum",             MR_U16_DTYPE,   NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_RECEIVE_MAXIMUM,              NA,                     CONNECT_RECEIVE_MAXIMUM,                false,  0,      NULL},
    {"maximum_packet_size",         MR_U32_DTYPE,   NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_MAXIMUM_PACKET_SIZE,          NA,                     CONNECT_MAXIMUM_PACKET_SIZE,            false,  0,      NULL},
    {"topic_alias_maximum",         MR_U16_DTYPE,   NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_TOPIC_ALIAS_MAXIMUM,          NA,                     CONNECT_TOPIC_ALIAS_MAXIMUM,            false,  0,      NULL},
    {"request_response_information",MR_U8_DTYPE,    NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_REQUEST_RESPONSE_INFORMATION, NA,                     CONNECT_REQUEST_RESPONSE_INFORMATION,   false,  0,      NULL},
    {"request_problem_information", MR_U8_DTYPE,    NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_REQUEST_PROBLEM_INFORMATION,  NA,                     CONNECT_REQUEST_PROBLEM_INFORMATION,    false,  0,      NULL},
    {"user_properties",             MR_SPV_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_USER_PROPERTY,                NA,                     CONNECT_USER_PROPERTIES,                false,  0,      NULL},
    {"authentication_method",       MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_AUTHENTICATION_METHOD,        NA,                     CONNECT_AUTHENTICATION_METHOD,          false,  0,      NULL},
    {"authentication_data",         MR_U8V_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_AUTHENTICATION_DATA,          NA,                     CONNECT_AUTHENTICATION_DATA,            false,  0,      NULL},
    {"client_identifier",           MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      true,   NA,                             NA,                                     NA,                     CONNECT_CLIENT_IDENTIFIER,              false,  0,      NULL},
    {"will_property_length",        MR_VBI_DTYPE,   NA, 0,              false,  0,      false,  CONNECT_WILL_USER_PROPERTIES,   NA,                                     CONNECT_WILL_FLAG,      CONNECT_WILL_PROPERTY_LENGTH,           false,  0,      NULL},
    {"mr_will_properties",          MR_PROPS_DTYPE, NA, (Word_t)MRCWP,  false,  MRCWPSZ,false,  NA,                             NA,                                     CONNECT_WILL_FLAG,      CONNECT_MR_WILL_PROPERTIES,             false,  0,      NULL},
    {"will_delay_interval",         MR_U32_DTYPE,   NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_WILL_DELAY_INTERVAL,          NA,                     CONNECT_WILL_DELAY_INTERVAL,            false,  0,      NULL},
    {"payload_format_indicator",    MR_U8_DTYPE,    NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_PAYLOAD_FORMAT_INDICATOR,     NA,                     CONNECT_PAYLOAD_FORMAT_INDICATOR,       false,  0,      NULL},
    {"message_expiry_interval",     MR_U32_DTYPE,   NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,      NA,                     CONNECT_MESSAGE_EXPIRY_INTERVAL,        false,  0,      NULL},
    {"content_type",                MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_CONTENT_TYPE,                 NA,                     CONNECT_CONTENT_TYPE,                   false,  0,      NULL},
    {"response_topic",              MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_RESPONSE_TOPIC,               NA,                     CONNECT_RESPONSE_TOPIC,                 false,  0,      NULL},
    {"correlation_data",            MR_U8V_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_CORRELATION_DATA,             NA,                     CONNECT_CORRELATION_DATA,               false,  0,      NULL},
    {"will_user_properties",        MR_SPV_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_RESPONSE_INFORMATION,         NA,                     CONNECT_WILL_USER_PROPERTIES,           false,  0,      NULL},
    {"will_topic",                  MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             NA,                                     CONNECT_WILL_FLAG,      CONNECT_WILL_TOPIC,                     false,  0,      NULL},
    {"will_payload",                MR_U8V_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             NA,                                     CONNECT_WILL_FLAG,      CONNECT_WILL_PAYLOAD,                   false,  0,      NULL},
    {"user_name",                   MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             NA,                                     CONNECT_USERNAME_FLAG,  CONNECT_USER_NAME,                      false,  0,      NULL},
    {"password",                    MR_U8V_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             NA,                                     CONNECT_PASSWORD_FLAG,  CONNECT_PASSWORD,                       false,  0,      NULL}
//   name                           dtype           bp  value           valloc  vlen    vexists link                            propid                                  flagid                  idx                                     ualloc u8vlen  u8v0
};

int mr_init_connect_pctx(packet_ctx **ppctx) {
    size_t mdata_count = sizeof(CONNECT_MDATA_TEMPLATE) / sizeof(mr_mdata);
    return mr_init_packet_context(ppctx, CONNECT_MDATA_TEMPLATE, mdata_count);
}

static int mr_connect_packet_check(packet_ctx *pctx) {
    if (pctx->mqtt_packet_type == MQTT_CONNECT) {
        return 0;
    }
    else {
        dzlog_info("Packet Context is not a CONNECT packet");
        return -1;
    }
}

int mr_pack_connect_u8v0(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_pack_mdata_u8v0(pctx);
}

int mr_unpack_connect_u8v0(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_unpack_mdata_u8v0(pctx);
}

int mr_free_connect_pctx(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_free_packet_context(pctx);
}

//    const uint8_t packet_type;
int mr_get_connect_packet_type(packet_ctx *pctx, uint8_t *pu8) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_PACKET_TYPE, pu8);
}

//    uint32_t remaining_length;
int mr_get_connect_remaining_length(packet_ctx *pctx, uint32_t *pu32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_REMAINING_LENGTH, pu32);
}

//    const uint8_t *protocol_name;
int mr_get_connect_protocol_name(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_PROTOCOL_NAME, pu8v0, plen);
}

//    const uint8_t protocol_version;
int mr_get_connect_protocol_version(packet_ctx *pctx, uint8_t *pu8) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_PROTOCOL_VERSION, pu8);
}

//    const bool reserved;
int mr_get_connect_reserved(packet_ctx *pctx, bool *pboolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_RESERVED, pboolean);
}

//    bool clean_start;
int mr_set_connect_clean_start(packet_ctx *pctx, bool boolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_CLEAN_START, boolean);
}

int mr_get_connect_clean_start(packet_ctx *pctx, bool *pboolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_CLEAN_START, pboolean);
}

int mr_reset_connect_clean_start(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_value(pctx, CONNECT_CLEAN_START);
}

//    bool will_flag;
int mr_get_connect_will_flag(packet_ctx *pctx, bool *pboolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_WILL_FLAG, pboolean);
}

//    uint8_t will_qos;
int mr_get_connect_will_qos(packet_ctx *pctx, uint8_t *pu8) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_WILL_QOS, pu8);
}

//    bool will_retain;
int mr_get_connect_will_retain(packet_ctx *pctx, bool *pboolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_WILL_RETAIN, pboolean);
}

//    bool password_flag;

int mr_get_connect_password_flag(packet_ctx *pctx, bool *pboolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_PASSWORD_FLAG, pboolean);
}

//    bool username_flag;

int mr_get_connect_username_flag(packet_ctx *pctx, bool *pboolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_USERNAME_FLAG, pboolean);
}

//    uint16_t keep_alive;
int mr_set_connect_keep_alive(packet_ctx *pctx, uint16_t u16) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_KEEP_ALIVE, u16);
}

int mr_get_connect_keep_alive(packet_ctx *pctx, uint16_t *pu16) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u16(pctx, CONNECT_KEEP_ALIVE, pu16);
}

int mr_reset_connect_keep_alive(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_value(pctx, CONNECT_KEEP_ALIVE);
}

//    uint32_t property_length;
int mr_get_connect_property_length(packet_ctx *pctx, uint32_t *pu32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_PROPERTY_LENGTH, pu32);
}

//    uint32_t session_expiry;
int mr_set_connect_session_expiry_interval(packet_ctx *pctx, uint32_t u32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_SESSION_EXPIRY_INTERVAL, u32);
}

int mr_get_connect_session_expiry_interval(packet_ctx *pctx, uint32_t *pu32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_SESSION_EXPIRY_INTERVAL, pu32);
}

int mr_reset_connect_session_expiry_interval(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_value(pctx, CONNECT_SESSION_EXPIRY_INTERVAL);
}

//    uint16_t receive_maximum;
int mr_set_connect_receive_maximum(packet_ctx *pctx, uint16_t u16) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_RECEIVE_MAXIMUM, u16);
}

int mr_get_connect_receive_maximum(packet_ctx *pctx, uint16_t *pu16) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u16(pctx, CONNECT_RECEIVE_MAXIMUM, pu16);
}

int mr_reset_connect_receive_maximum(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_value(pctx, CONNECT_RECEIVE_MAXIMUM);
}

//    uint32_t maximum_packet_size;
int mr_set_connect_maximum_packet_size(packet_ctx *pctx, uint32_t u32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_MAXIMUM_PACKET_SIZE, u32);
}

int mr_get_connect_maximum_packet_size(packet_ctx *pctx, uint32_t *pu32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_MAXIMUM_PACKET_SIZE, pu32);
}

int mr_reset_connect_maximum_packet_size(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_value(pctx, CONNECT_MAXIMUM_PACKET_SIZE);
}

//    uint16_t topic_alias_maximum;
int mr_set_connect_topic_alias_maximum(packet_ctx *pctx, uint16_t u16) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM, u16);
}

int mr_get_connect_topic_alias_maximum(packet_ctx *pctx, uint16_t *pu16) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u16(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM, pu16);
}

int mr_reset_connect_topic_alias_maximum(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_value(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM);
}

//    uint8_t request_response_information;
int mr_set_connect_request_response_information(packet_ctx *pctx, uint8_t u8) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION, u8);
}

int mr_get_connect_request_response_information(packet_ctx *pctx, uint8_t *pu8) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION, pu8);
}

int mr_reset_connect_request_response_information(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_value(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION);
}

//    uint8_t request_problem_information;
int mr_set_connect_request_problem_information(packet_ctx *pctx, uint8_t u8) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION, u8);
}

int mr_get_connect_request_problem_information(packet_ctx *pctx, uint8_t *pu8) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION, pu8);
}

int mr_reset_connect_request_problem_information(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_value(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION);
}

//    string_pair *user_properties;
int mr_set_connect_user_properties(packet_ctx *pctx, string_pair *spv0, size_t len) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_USER_PROPERTIES, spv0, len);
}

int mr_get_connect_user_properties(packet_ctx *pctx, string_pair **pspv0, size_t *plen) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_spv(pctx, CONNECT_USER_PROPERTIES, pspv0, plen);
}

int mr_reset_connect_user_properties(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_value(pctx, CONNECT_USER_PROPERTIES);
}

//    uint8_t *authentication_method;
int mr_set_connect_authentication_method(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_AUTHENTICATION_METHOD, u8v0, len);
}

int mr_get_connect_authentication_method(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_AUTHENTICATION_METHOD, pu8v0, plen);
}

int mr_reset_connect_authentication_method(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_value(pctx, CONNECT_AUTHENTICATION_METHOD);
}

//    uint8_t *authentication_data;
int mr_set_connect_authentication_data(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_AUTHENTICATION_DATA, u8v0, len);
}

int mr_get_connect_authentication_data(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_AUTHENTICATION_DATA, pu8v0, plen);
}

int mr_reset_connect_authentication_data(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_value(pctx, CONNECT_AUTHENTICATION_DATA);
}

//    uint8_t *client_identifier;
int mr_set_connect_client_identifier(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_CLIENT_IDENTIFIER, u8v0, len);
}

int mr_get_connect_client_identifier(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_CLIENT_IDENTIFIER, pu8v0, plen);
}

int mr_reset_connect_client_identifier(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_value(pctx, CONNECT_CLIENT_IDENTIFIER);
}

//    uint32_t will_property_length;
int mr_get_connect_will_property_length(packet_ctx *pctx, uint32_t *pu32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_WILL_PROPERTY_LENGTH, pu32);
}

//    uint32_t will_delay_interval;
int mr_get_connect_will_delay_interval(packet_ctx *pctx, uint32_t *pu32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_WILL_DELAY_INTERVAL, pu32);
}

//    uint8_t payload_format_indicator;
int mr_get_connect_payload_format_indicator(packet_ctx *pctx, uint8_t *pu8) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, pu8);
}

//    uint32_t message_expiry_interval;
int mr_get_connect_message_expiry_interval(packet_ctx *pctx, uint32_t *pu32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, pu32);
}

//    uint8_t *content_type;
int mr_get_connect_content_type(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_CONTENT_TYPE, pu8v0, plen);
}

//    uint8_t *response_topic;
int mr_get_connect_response_topic(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_RESPONSE_TOPIC, pu8v0, plen);
}

//    uint8_t *correlation_data;
int mr_get_connect_correlation_data(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_CORRELATION_DATA, pu8v0, plen);
}

//    string_pair *will_user_properties;
int mr_get_connect_will_user_properties(packet_ctx *pctx, string_pair **pspv0, size_t *plen) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_spv(pctx, CONNECT_WILL_USER_PROPERTIES, pspv0, plen);
}

//    uint8_t *will_topic;
int mr_get_connect_will_topic(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_WILL_TOPIC, pu8v0, plen);
}

//    uint8_t *will_payload;
int mr_get_connect_will_payload(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_WILL_PAYLOAD, pu8v0, plen);
}

//    uint8_t *user_name;
int mr_set_connect_user_name(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    if (mr_connect_packet_check(pctx)) return -1;
    int rc = mr_set_vector(pctx, CONNECT_USER_NAME, u8v0, len);
    if (!rc) rc = mr_set_scalar(pctx, CONNECT_USERNAME_FLAG, true);
    if (rc) mr_reset_connect_user_name(pctx);
    return rc;
}

int mr_get_connect_user_name(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_USER_NAME, pu8v0, plen);
}

int mr_reset_connect_user_name(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    int rc = mr_reset_value(pctx, CONNECT_USER_NAME);
    if (!rc) rc = mr_reset_value(pctx, CONNECT_USERNAME_FLAG);
    return rc;
}

//    uint8_t *password;
int mr_set_connect_password(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    if (mr_connect_packet_check(pctx)) return -1;
    int rc = mr_set_vector(pctx, CONNECT_PASSWORD, u8v0, len);
    if (!rc) rc = mr_set_scalar(pctx, CONNECT_PASSWORD_FLAG, true);
    if (rc) mr_reset_connect_password(pctx);
    return rc;
}

int mr_get_connect_password(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_PASSWORD, pu8v0, plen);
}

int mr_reset_connect_password(packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    int rc = mr_reset_value(pctx, CONNECT_PASSWORD);
    if (!rc) rc = mr_reset_value(pctx, CONNECT_PASSWORD_FLAG);
    return rc;
}
