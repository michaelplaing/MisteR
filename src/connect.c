/* connect.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mister/mister.h"
#include "mister/mrzlog.h"
#include "connect_internal.h"
#include "packet_internal.h"

static const uint8_t _PS[] = {'M', 'Q', 'T', 'T'};  // protocol signature
#define _PSSZ 4
#define _PV 5
#define _NA 0

static const uint8_t _CP[] = { // connect property values
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
#define _CPSZ 9

static const uint8_t _CWP[] = { // connect will property values
    MQTT_PROP_WILL_DELAY_INTERVAL,
    MQTT_PROP_PAYLOAD_FORMAT_INDICATOR,
    MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,
    MQTT_PROP_CONTENT_TYPE,
    MQTT_PROP_RESPONSE_TOPIC,
    MQTT_PROP_CORRELATION_DATA,
    MQTT_PROP_USER_PROPERTY
};
#define _CWPSZ 7

static const mr_mdata _CONNECT_MDATA_TEMPLATE[] = { // Same order as enum CONNECT_MDATA_FIELDS (see column idx)
//   name                           dtype           bpos    value               valloc  vlen    u8vlen  vexists link                            propid                                  flagid                  idx                                     pvalloc     pvalue
    {"packet_type",                 MR_U8_DTYPE,    _NA,    MQTT_CONNECT,       false,  1,      1,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_PACKET_TYPE,                    false,      NULL},
    {"remaining_length",            MR_VBI_DTYPE,   _NA,    0,                  false,  0,      0,      true,   CONNECT_PASSWORD,               _NA,                                    _NA,                    CONNECT_REMAINING_LENGTH,               false,      NULL},
    {"protocol_name",               MR_U8V_DTYPE,   _NA,    (mr_mvalue_t)_PS,   false,  _PSSZ,  _PSSZ+2,true,   _NA,                            _NA,                                    _NA,                    CONNECT_PROTOCOL_NAME,                  false,      NULL},
    {"protocol_version",            MR_U8_DTYPE,    _NA,    _PV,                false,  1,      1,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_PROTOCOL_VERSION,               false,      NULL},
    {"reserved",                    MR_BITS_DTYPE,  0,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_RESERVED,                       false,      NULL},
    {"clean_start",                 MR_BITS_DTYPE,  1,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_CLEAN_START,                    false,      NULL},
    {"will_flag",                   MR_BITS_DTYPE,  2,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_WILL_FLAG,                      false,      NULL},
    {"will_qos",                    MR_BITS_DTYPE,  3,      0,                  false,  2,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_QOS,                       false,      NULL},
    {"will_retain",                 MR_BITS_DTYPE,  5,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_RETAIN,                    false,      NULL},
    {"password_flag",               MR_BITS_DTYPE,  6,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_PASSWORD_FLAG,                  false,      NULL},
    {"username_flag",               MR_BITS_DTYPE,  7,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_USERNAME_FLAG,                  false,      NULL},
    {"mr_flags",                    MR_FLAGS_DTYPE, _NA,    0,                  false,  1,      1,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_MR_FLAGS,                       false,      NULL},
    {"keep_alive",                  MR_U16_DTYPE,   _NA,    0,                  false,  2,      2,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_KEEP_ALIVE,                     false,      NULL},
    {"property_length",             MR_VBI_DTYPE,   _NA,    0,                  false,  0,      0,      true,   CONNECT_AUTHENTICATION_DATA,    _NA,                                    _NA,                    CONNECT_PROPERTY_LENGTH,                false,      NULL},
    {"mr_properties",               MR_PROPS_DTYPE, _NA,    (mr_mvalue_t)_CP,   false,  _CPSZ, _NA,     true,   _NA,                            _NA,                                    _NA,                    CONNECT_MR_PROPERTIES,                  false,      NULL},
    {"session_expiry_interval",     MR_U32_DTYPE,   _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_SESSION_EXPIRY_INTERVAL,      _NA,                    CONNECT_SESSION_EXPIRY_INTERVAL,        false,      NULL},
    {"receive_maximum",             MR_U16_DTYPE,   _NA,    0,                  false,  2,      3,      false,  _NA,                            MQTT_PROP_RECEIVE_MAXIMUM,              _NA,                    CONNECT_RECEIVE_MAXIMUM,                false,      NULL},
    {"maximum_packet_size",         MR_U32_DTYPE,   _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_MAXIMUM_PACKET_SIZE,          _NA,                    CONNECT_MAXIMUM_PACKET_SIZE,            false,      NULL},
    {"topic_alias_maximum",         MR_U16_DTYPE,   _NA,    0,                  false,  2,      3,      false,  _NA,                            MQTT_PROP_TOPIC_ALIAS_MAXIMUM,          _NA,                    CONNECT_TOPIC_ALIAS_MAXIMUM,            false,      NULL},
    {"request_response_information",MR_U8_DTYPE,    _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_REQUEST_RESPONSE_INFORMATION, _NA,                    CONNECT_REQUEST_RESPONSE_INFORMATION,   false,      NULL},
    {"request_problem_information", MR_U8_DTYPE,    _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_REQUEST_PROBLEM_INFORMATION,  _NA,                    CONNECT_REQUEST_PROBLEM_INFORMATION,    false,      NULL},
    {"user_properties",             MR_SPV_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_USER_PROPERTY,                _NA,                    CONNECT_USER_PROPERTIES,                false,      NULL},
    {"authentication_method",       MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_AUTHENTICATION_METHOD,        _NA,                    CONNECT_AUTHENTICATION_METHOD,          false,      NULL},
    {"authentication_data",         MR_U8V_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_AUTHENTICATION_DATA,          _NA,                    CONNECT_AUTHENTICATION_DATA,            false,      NULL},
    {"client_identifier",           MR_STR_DTYPE,   _NA,    (mr_mvalue_t)"",    false,  1,      2,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_CLIENT_IDENTIFIER,              false,      NULL},
    {"will_property_length",        MR_VBI_DTYPE,   _NA,    0,                  false,  0,      0,      false,  CONNECT_WILL_USER_PROPERTIES,   _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_PROPERTY_LENGTH,           false,      NULL},
    {"mr_will_properties",          MR_PROPS_DTYPE, _NA,    (mr_mvalue_t)_CWP,  false,  _CWPSZ, _NA,    false,  _NA,                            _NA,                                    CONNECT_WILL_FLAG,      CONNECT_MR_WILL_PROPERTIES,             false,      NULL},
    {"will_delay_interval",         MR_U32_DTYPE,   _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_WILL_DELAY_INTERVAL,          CONNECT_WILL_FLAG,      CONNECT_WILL_DELAY_INTERVAL,            false,      NULL},
    {"payload_format_indicator",    MR_U8_DTYPE,    _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_PAYLOAD_FORMAT_INDICATOR,     CONNECT_WILL_FLAG,      CONNECT_PAYLOAD_FORMAT_INDICATOR,       false,      NULL},
    {"message_expiry_interval",     MR_U32_DTYPE,   _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,      CONNECT_WILL_FLAG,      CONNECT_MESSAGE_EXPIRY_INTERVAL,        false,      NULL},
    {"content_type",                MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_CONTENT_TYPE,                 CONNECT_WILL_FLAG,      CONNECT_CONTENT_TYPE,                   false,      NULL},
    {"response_topic",              MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_RESPONSE_TOPIC,               CONNECT_WILL_FLAG,      CONNECT_RESPONSE_TOPIC,                 false,      NULL},
    {"correlation_data",            MR_U8V_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_CORRELATION_DATA,             CONNECT_WILL_FLAG,      CONNECT_CORRELATION_DATA,               false,      NULL},
    {"will_user_properties",        MR_SPV_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_USER_PROPERTY,                CONNECT_WILL_FLAG,      CONNECT_WILL_USER_PROPERTIES,           false,      NULL},
    {"will_topic",                  MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_TOPIC,                     false,      NULL},
    {"will_payload",                MR_U8V_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_PAYLOAD,                   false,      NULL},
    {"user_name",                   MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            _NA,                                    CONNECT_USERNAME_FLAG,  CONNECT_USER_NAME,                      false,      NULL},
    {"password",                    MR_U8V_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            _NA,                                    CONNECT_PASSWORD_FLAG,  CONNECT_PASSWORD,                       false,      NULL}
//   name                           dtype           bpos    value               valloc  vlen    u8vlen  vexists link                            propid                                  flagid                  idx                                     spvalloc    spv
};

static const size_t _CONNECT_MDATA_COUNT = sizeof(_CONNECT_MDATA_TEMPLATE) / sizeof(mr_mdata);

int mr_init_connect_pctx(mr_packet_ctx **ppctx) {
    return mr_init_packet(ppctx, _CONNECT_MDATA_TEMPLATE, _CONNECT_MDATA_COUNT);
}

int mr_init_unpack_connect_packet(mr_packet_ctx **ppctx, uint8_t *u8v0, size_t u8vlen) {
    return mr_init_unpack_packet(ppctx, _CONNECT_MDATA_TEMPLATE, _CONNECT_MDATA_COUNT, u8v0, u8vlen);
}

static int mr_connect_packet_check(mr_packet_ctx *pctx) {
    if (pctx->mqtt_packet_type == MQTT_CONNECT) {
        return 0;
    }
    else {
        dzlog_error("Packet Context is not a CONNECT packet");
        return -1;
    }
}

int mr_pack_connect_packet(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_pack_packet(pctx);
}

int mr_free_connect_pctx(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_free_packet_context(pctx);
}

int mr_connect_mdata_dump(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_mdata_dump(pctx);
}

// const uint8_t packet_type;
int mr_get_connect_packet_type(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_PACKET_TYPE, pu8, pexists);
}

// uint32_t remaining_length;
int mr_get_connect_remaining_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_REMAINING_LENGTH, pu32, pexists);
}

// const uint8_t *protocol_name;
int mr_get_connect_protocol_name(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_PROTOCOL_NAME, pu8v0, plen, pexists);
}

// const uint8_t protocol_version;
int mr_get_connect_protocol_version(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_PROTOCOL_VERSION, pu8, pexists);
}

// const bool reserved;
int mr_get_connect_reserved(mr_packet_ctx *pctx, bool *pboolean, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_RESERVED, pboolean, pexists);
}

// bool clean_start;
int mr_get_connect_clean_start(mr_packet_ctx *pctx, bool *pboolean, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_CLEAN_START, pboolean, pexists);
}

int mr_set_connect_clean_start(mr_packet_ctx *pctx, bool boolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_CLEAN_START, boolean);
}

int mr_reset_connect_clean_start(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_CLEAN_START);
}

// bool will_flag;
int mr_get_connect_will_flag(mr_packet_ctx *pctx, bool *pboolean, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_WILL_FLAG, pboolean, pexists);
}

int mr_set_connect_will_flag(mr_packet_ctx *pctx, bool boolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_WILL_FLAG, boolean);
}

int mr_reset_connect_will_flag(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_WILL_FLAG);
}

// uint8_t will_qos;
int mr_get_connect_will_qos(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_WILL_QOS, pu8, pexists);
}

int mr_set_connect_will_qos(mr_packet_ctx *pctx, uint8_t u8) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_WILL_QOS, u8);
}

int mr_reset_connect_will_qos(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_WILL_QOS);
}

// bool will_retain;
int mr_get_connect_will_retain(mr_packet_ctx *pctx, bool *pboolean, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_WILL_RETAIN, pboolean, pexists);
}

int mr_set_connect_will_retain(mr_packet_ctx *pctx, bool boolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_WILL_RETAIN, boolean);
}

int mr_reset_connect_will_retain(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_WILL_RETAIN);
}

// bool password_flag;
int mr_get_connect_password_flag(mr_packet_ctx *pctx, bool *pboolean, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_PASSWORD_FLAG, pboolean, pexists);
}

int mr_set_connect_password_flag(mr_packet_ctx *pctx, bool boolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_PASSWORD_FLAG, boolean);
}

int mr_reset_connect_password_flag(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_PASSWORD_FLAG);
}

// bool username_flag;
int mr_get_connect_username_flag(mr_packet_ctx *pctx, bool *pboolean, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_USERNAME_FLAG, pboolean, pexists);
}

int mr_set_connect_username_flag(mr_packet_ctx *pctx, bool boolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_USERNAME_FLAG, boolean);
}

int mr_reset_connect_username_flag(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_USERNAME_FLAG);
}

// uint16_t keep_alive;
int mr_get_connect_keep_alive(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u16(pctx, CONNECT_KEEP_ALIVE, pu16, pexists);
}

int mr_set_connect_keep_alive(mr_packet_ctx *pctx, uint16_t u16) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_KEEP_ALIVE, u16);
}

int mr_reset_connect_keep_alive(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_KEEP_ALIVE);
}

// uint32_t property_length;
int mr_get_connect_property_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_PROPERTY_LENGTH, pu32, pexists);
}

// uint32_t session_expiry_interval;
int mr_get_connect_session_expiry_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_SESSION_EXPIRY_INTERVAL, pu32, pexists);
}

int mr_set_connect_session_expiry_interval(mr_packet_ctx *pctx, uint32_t u32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_SESSION_EXPIRY_INTERVAL, u32);
}

int mr_reset_connect_session_expiry_interval(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_SESSION_EXPIRY_INTERVAL);
}

// uint16_t receive_maximum;
int mr_get_connect_receive_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u16(pctx, CONNECT_RECEIVE_MAXIMUM, pu16, pexists);
}

int mr_set_connect_receive_maximum(mr_packet_ctx *pctx, uint16_t u16) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_RECEIVE_MAXIMUM, u16);
}

int mr_reset_connect_receive_maximum(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_RECEIVE_MAXIMUM);
}

// uint32_t maximum_packet_size;
int mr_get_connect_maximum_packet_size(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_MAXIMUM_PACKET_SIZE, pu32, pexists);
}

int mr_set_connect_maximum_packet_size(mr_packet_ctx *pctx, uint32_t u32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_MAXIMUM_PACKET_SIZE, u32);
}

int mr_reset_connect_maximum_packet_size(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_MAXIMUM_PACKET_SIZE);
}

// uint16_t topic_alias_maximum;
int mr_get_connect_topic_alias_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u16(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM, pu16, pexists);
}

int mr_set_connect_topic_alias_maximum(mr_packet_ctx *pctx, uint16_t u16) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM, u16);
}

int mr_reset_connect_topic_alias_maximum(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM);
}

// uint8_t request_response_information;
int mr_get_connect_request_response_information(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION, pu8, pexists);
}

int mr_set_connect_request_response_information(mr_packet_ctx *pctx, uint8_t u8) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION, u8);
}

int mr_reset_connect_request_response_information(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION);
}

// uint8_t request_problem_information;
int mr_get_connect_request_problem_information(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION, pu8, pexists);
}

int mr_set_connect_request_problem_information(mr_packet_ctx *pctx, uint8_t u8) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION, u8);
}

int mr_reset_connect_request_problem_information(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION);
}

// mr_string_pair *user_properties;
int mr_get_connect_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_spv(pctx, CONNECT_USER_PROPERTIES, pspv0, plen, pexists);
}

int mr_set_connect_user_properties(mr_packet_ctx *pctx, mr_string_pair *spv0, size_t len) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_USER_PROPERTIES, spv0, len);
}

int mr_reset_connect_user_properties(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_USER_PROPERTIES);
}

// char *authentication_method;
int mr_get_connect_authentication_method(mr_packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNECT_AUTHENTICATION_METHOD, pcv0, pexists);
}

int mr_set_connect_authentication_method(mr_packet_ctx *pctx, char *cv0) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_AUTHENTICATION_METHOD, cv0, strlen(cv0) + 1);
}

int mr_reset_connect_authentication_method(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_AUTHENTICATION_METHOD);
}

// uint8_t *authentication_data;
int mr_get_connect_authentication_data(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_AUTHENTICATION_DATA, pu8v0, plen, pexists);
}

int mr_set_connect_authentication_data(mr_packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_AUTHENTICATION_DATA, u8v0, len);
}

int mr_reset_connect_authentication_data(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_AUTHENTICATION_DATA);
}

// char *client_identifier;
int mr_get_connect_client_identifier(mr_packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNECT_CLIENT_IDENTIFIER, pcv0, pexists);
}

int mr_set_connect_client_identifier(mr_packet_ctx *pctx, char *cv0) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_CLIENT_IDENTIFIER, cv0, strlen(cv0) + 1);
}

int mr_reset_connect_client_identifier(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_CLIENT_IDENTIFIER);
}

// uint32_t will_property_length;
int mr_get_connect_will_property_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_WILL_PROPERTY_LENGTH, pu32, pexists);
}

int mr_set_connect_will_property_length(mr_packet_ctx *pctx, uint32_t u32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_WILL_PROPERTY_LENGTH, 0); // sets vexists; value always calculated
}

int mr_reset_connect_will_property_length(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_WILL_PROPERTY_LENGTH);
}

// uint32_t will_delay_interval;
int mr_get_connect_will_delay_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_WILL_DELAY_INTERVAL, pu32, pexists);
}

int mr_set_connect_will_delay_interval(mr_packet_ctx *pctx, uint32_t u32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_WILL_DELAY_INTERVAL, u32);
}

int mr_reset_connect_will_delay_interval(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_WILL_DELAY_INTERVAL);
}

// uint8_t payload_format_indicator;
int mr_get_connect_payload_format_indicator(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, pu8, pexists);
}

int mr_set_connect_payload_format_indicator(mr_packet_ctx *pctx, uint8_t u8) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, u8);
}

int mr_reset_connect_payload_format_indicator(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR);
}

// uint32_t message_expiry_interval;
int mr_get_connect_message_expiry_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, pu32, pexists);
}

int mr_set_connect_message_expiry_interval(mr_packet_ctx *pctx, uint32_t u32) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, u32);
}

int mr_reset_connect_message_expiry_interval(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL);
}

// char *content_type;
int mr_get_connect_content_type(mr_packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNECT_CONTENT_TYPE, pcv0, pexists);
}

int mr_set_connect_content_type(mr_packet_ctx *pctx, char *cv0) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_CONTENT_TYPE, cv0, strlen(cv0) + 1);
}

int mr_reset_connect_content_type(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_CONTENT_TYPE);
}

// char *response_topic;
int mr_get_connect_response_topic(mr_packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNECT_RESPONSE_TOPIC, pcv0, pexists);
}

int mr_set_connect_response_topic(mr_packet_ctx *pctx, char *cv0) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_RESPONSE_TOPIC, cv0, strlen(cv0) + 1);
}

int mr_reset_connect_response_topic(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_RESPONSE_TOPIC);
}

// uint8_t *correlation_data;
int mr_get_connect_correlation_data(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_CORRELATION_DATA, pu8v0, plen, pexists);
}

int mr_set_connect_correlation_data(mr_packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_CORRELATION_DATA, u8v0, len);
}

int mr_reset_connect_correlation_data(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_CORRELATION_DATA);
}

// mr_string_pair *will_user_properties;
int mr_get_connect_will_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_spv(pctx, CONNECT_WILL_USER_PROPERTIES, pspv0, plen, pexists);
}

int mr_set_connect_will_user_properties(mr_packet_ctx *pctx, mr_string_pair *spv0, size_t len) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_WILL_USER_PROPERTIES, spv0, len);
}

int mr_reset_connect_will_user_properties(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_WILL_USER_PROPERTIES);
}

// char *will_topic;
int mr_get_connect_will_topic(mr_packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNECT_WILL_TOPIC, pcv0, pexists);
}

int mr_set_connect_will_topic(mr_packet_ctx *pctx, char *cv0) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_WILL_TOPIC, cv0, strlen(cv0) + 1);
}

int mr_reset_connect_will_topic(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_WILL_TOPIC);
}

// uint8_t *will_payload;
int mr_get_connect_will_payload(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_WILL_PAYLOAD, pu8v0, plen, pexists);
}

int mr_set_connect_will_payload(mr_packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_WILL_PAYLOAD, u8v0, len);
}

int mr_reset_connect_will_payload(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_WILL_PAYLOAD);
}

// char *user_name;
int mr_get_connect_user_name(mr_packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNECT_USER_NAME, pcv0, pexists);
}

int mr_set_connect_user_name(mr_packet_ctx *pctx, char *cv0) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_USER_NAME, cv0, strlen(cv0) + 1);
}

int mr_reset_connect_user_name(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_USER_NAME);
}

static int mr_validate_cross_connect_user_name(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    bool bflag_value;
    bool bflag_exists;
    if (mr_get_boolean(pctx, CONNECT_USERNAME_FLAG, &bflag_value, &bflag_exists)) return -1;
    uint8_t *u8v0;
    bool bubv0_exists;
    size_t len;
    if (mr_get_u8v(pctx, CONNECT_USER_NAME, &u8v0, &len, &bubv0_exists)) return -1;
    if (!bflag_exists && !bubv0_exists) return 0;
    if (bflag_exists && bflag_value && bubv0_exists) return 0;

    dzlog_error(
        "CONNECT_USERNAME_FLAG / CONNECT_USER_NAME are inconsistent:: "
        "username flag exists/value: %u / %u; user name exists: %u",
        bflag_exists, bflag_value, bubv0_exists
    );

    return -1;
}

// uint8_t *password;
int mr_get_connect_password(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_PASSWORD, pu8v0, plen, pexists);
}

int mr_set_connect_password(mr_packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_PASSWORD, u8v0, len);
}

int mr_reset_connect_password(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_PASSWORD);
}

static int mr_validate_cross_connect_password(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    bool bflag_value;
    bool bflag_exists;
    if (mr_get_boolean(pctx, CONNECT_PASSWORD_FLAG, &bflag_value, &bflag_exists)) return -1;
    uint8_t *u8v0;
    bool bubv0_exists;
    size_t len;
    if (mr_get_u8v(pctx, CONNECT_PASSWORD, &u8v0, &len, &bubv0_exists)) return -1;
    if (!bflag_exists && !bubv0_exists) return 0;
    if (bflag_exists && bflag_value && bubv0_exists) return 0;

    dzlog_error(
        "CONNECT_PASSWORD_FLAG / CONNECT_PASSWORD are inconsistent::\n"
        "  password flag exists/value: %u / %u; password exists: %u",
        bflag_exists, bflag_value, bubv0_exists
    );

    return -1;
}

int mr_validate_connect_values(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    if (mr_validate_connect_cross(pctx)) return -1;
    if (mr_validate_utf8_values(pctx)) return -1;
    if (mr_validate_connect_extra(pctx)) return -1;

    return 0;
}

static int mr_validate_connect_cross(mr_packet_ctx *pctx) {
    bool bexists;

    bool will_flag;
    if (mr_get_boolean(pctx, CONNECT_WILL_FLAG, &will_flag, &bexists)) return -1;
    bool username_flag;
    if (mr_get_boolean(pctx, CONNECT_USERNAME_FLAG, &username_flag, &bexists)) return -1;
    bool password_flag;
    if (mr_get_boolean(pctx, CONNECT_PASSWORD_FLAG, &password_flag, &bexists)) return -1;

    mr_mdata *mdata = pctx->mdata0;
    for (int i = 0; i < _CONNECT_MDATA_COUNT; i++, mdata++) {
        if (mdata->flagid == CONNECT_WILL_FLAG && !will_flag) { // governed by will_flag AND it is false
            if (mdata->dtype == MR_BITS_DTYPE && mdata->value) { // MR_BITS_DTYPE always exist so check value
                dzlog_error("will_flag false but '%s' is set", mdata->name);
                return -1;
            }
            else if (mdata->vexists) {
                dzlog_error("will_flag false but '%s' exists", mdata->name);
                return -1;
            }
        }
        else if (mdata->flagid == CONNECT_USERNAME_FLAG && !username_flag) {
            if (mdata->vexists) {
                dzlog_error("username_flag false but '%s' exists", mdata->name);
                return -1;
            }
        }
        else if (mdata->flagid == CONNECT_PASSWORD_FLAG && !password_flag) {
            if (mdata->vexists) {
                dzlog_error("password_flag false but '%s' exists", mdata->name);
                return -1;
            }
        }
        else {
            ; // noop
        }
    }

    return 0;
}

int mr_validate_connect_extra(mr_packet_ctx *pctx) {
    uint8_t u8;
    bool bexists;

    // payload_format_indicator & will_payload
    if (mr_get_connect_payload_format_indicator(pctx, &u8, &bexists)) return -1;

    if (bexists) {
        if (u8 > 1) {
            dzlog_error("payload_format_indicator out of range (0..1): %u", u8);
            return -1;
        }
        else { // previous cross check of will_flag insures existence
            if (u8 > 0 && mr_validate_u8vutf8(pctx, CONNECT_WILL_PAYLOAD)) return -1;
        }
    }

    // will_qos
    if (mr_get_connect_will_qos(pctx, &u8, &bexists)) return -1;

    if (bexists && u8 && u8 > 2) {
        dzlog_error("will_qos out of range (0..2): %u", u8);
        return -1;
    }

    return 0;
}
