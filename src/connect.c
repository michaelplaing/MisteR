// connect.c

/**
 * @file
 * @brief The CONNECT packet metadata (mr_mdata vector) and functions.
 *
 * This packet specifies the vector of mr_mdata instances that characterize the CONNECT
 * values. It provides functions to get, set & reset them as well as validate. There are
 * additional functions for cross-validation.
*/

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

static const char _S0L[] = "";

static const mr_mdata _CONNECT_MDATA_TEMPLATE[] = { // Same order as enum CONNECT_MDATA_FIELDS (see column idx)
//   name                           dtype           bpos    value               valloc  vlen    u8vlen  vexists link                            propid                                  flagid                  idx                                     ovalue
    {"packet_type",                 MR_U8_DTYPE,    _NA,    MQTT_CONNECT,       false,  1,      1,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_PACKET_TYPE,                    NULL},
    {"remaining_length",            MR_VBI_DTYPE,   _NA,    0,                  false,  0,      0,      true,   CONNECT_PASSWORD,               _NA,                                    _NA,                    CONNECT_REMAINING_LENGTH,               NULL},
    {"protocol_name",               MR_U8V_DTYPE,   _NA,    (mr_mvalue_t)_PS,   false,  _PSSZ,  _PSSZ+2,true,   _NA,                            _NA,                                    _NA,                    CONNECT_PROTOCOL_NAME,                  NULL},
    {"protocol_version",            MR_U8_DTYPE,    _NA,    _PV,                false,  1,      1,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_PROTOCOL_VERSION,               NULL},
    {"reserved",                    MR_BITS_DTYPE,  0,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_RESERVED,                       NULL},
    {"clean_start",                 MR_BITS_DTYPE,  1,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_CLEAN_START,                    NULL},
    {"will_flag",                   MR_BITS_DTYPE,  2,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_WILL_FLAG,                      NULL},
    {"will_qos",                    MR_BITS_DTYPE,  3,      0,                  false,  2,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_QOS,                       NULL},
    {"will_retain",                 MR_BITS_DTYPE,  5,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_RETAIN,                    NULL},
    {"password_flag",               MR_BITS_DTYPE,  6,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_PASSWORD_FLAG,                  NULL},
    {"username_flag",               MR_BITS_DTYPE,  7,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_USERNAME_FLAG,                  NULL},
    {"mr_flags",                    MR_FLAGS_DTYPE, _NA,    0,                  false,  1,      1,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_MR_FLAGS,                       NULL},
    {"keep_alive",                  MR_U16_DTYPE,   _NA,    0,                  false,  2,      2,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_KEEP_ALIVE,                     NULL},
    {"property_length",             MR_VBI_DTYPE,   _NA,    0,                  false,  0,      0,      true,   CONNECT_AUTHENTICATION_DATA,    _NA,                                    _NA,                    CONNECT_PROPERTY_LENGTH,                NULL},
    {"mr_properties",               MR_PROPS_DTYPE, _NA,    (mr_mvalue_t)_CP,   _NA,    _CPSZ,  _NA,    true,    _NA,                           _NA,                                    _NA,                    CONNECT_MR_PROPERTIES,                  NULL},
    {"session_expiry_interval",     MR_U32_DTYPE,   _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_SESSION_EXPIRY_INTERVAL,      _NA,                    CONNECT_SESSION_EXPIRY_INTERVAL,        NULL},
    {"receive_maximum",             MR_U16_DTYPE,   _NA,    0,                  false,  2,      3,      false,  _NA,                            MQTT_PROP_RECEIVE_MAXIMUM,              _NA,                    CONNECT_RECEIVE_MAXIMUM,                NULL},
    {"maximum_packet_size",         MR_U32_DTYPE,   _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_MAXIMUM_PACKET_SIZE,          _NA,                    CONNECT_MAXIMUM_PACKET_SIZE,            NULL},
    {"topic_alias_maximum",         MR_U16_DTYPE,   _NA,    0,                  false,  2,      3,      false,  _NA,                            MQTT_PROP_TOPIC_ALIAS_MAXIMUM,          _NA,                    CONNECT_TOPIC_ALIAS_MAXIMUM,            NULL},
    {"request_response_information",MR_U8_DTYPE,    _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_REQUEST_RESPONSE_INFORMATION, _NA,                    CONNECT_REQUEST_RESPONSE_INFORMATION,   NULL},
    {"request_problem_information", MR_U8_DTYPE,    _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_REQUEST_PROBLEM_INFORMATION,  _NA,                    CONNECT_REQUEST_PROBLEM_INFORMATION,    NULL},
    {"user_properties",             MR_SPV_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_USER_PROPERTY,                _NA,                    CONNECT_USER_PROPERTIES,                NULL},
    {"authentication_method",       MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_AUTHENTICATION_METHOD,        _NA,                    CONNECT_AUTHENTICATION_METHOD,          NULL},
    {"authentication_data",         MR_U8V_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_AUTHENTICATION_DATA,          _NA,                    CONNECT_AUTHENTICATION_DATA,            NULL},
    {"client_identifier",           MR_STR_DTYPE,   _NA,    (mr_mvalue_t)_S0L,  false,  1,      2,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_CLIENT_IDENTIFIER,              NULL},
    {"will_property_length",        MR_VBI_DTYPE,   _NA,    0,                  false,  0,      0,      false,  CONNECT_WILL_USER_PROPERTIES,   _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_PROPERTY_LENGTH,           NULL},
    {"mr_will_properties",          MR_PROPS_DTYPE, _NA,    (mr_mvalue_t)_CWP,  _NA,    _CWPSZ, _NA,    _NA,    _NA,                            _NA,                                    _NA,                    CONNECT_MR_WILL_PROPERTIES,             NULL},
    {"will_delay_interval",         MR_U32_DTYPE,   _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_WILL_DELAY_INTERVAL,          CONNECT_WILL_FLAG,      CONNECT_WILL_DELAY_INTERVAL,            NULL},
    {"payload_format_indicator",    MR_U8_DTYPE,    _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_PAYLOAD_FORMAT_INDICATOR,     CONNECT_WILL_FLAG,      CONNECT_PAYLOAD_FORMAT_INDICATOR,       NULL},
    {"message_expiry_interval",     MR_U32_DTYPE,   _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,      CONNECT_WILL_FLAG,      CONNECT_MESSAGE_EXPIRY_INTERVAL,        NULL},
    {"content_type",                MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_CONTENT_TYPE,                 CONNECT_WILL_FLAG,      CONNECT_CONTENT_TYPE,                   NULL},
    {"response_topic",              MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_RESPONSE_TOPIC,               CONNECT_WILL_FLAG,      CONNECT_RESPONSE_TOPIC,                 NULL},
    {"correlation_data",            MR_U8V_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_CORRELATION_DATA,             CONNECT_WILL_FLAG,      CONNECT_CORRELATION_DATA,               NULL},
    {"will_user_properties",        MR_SPV_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_USER_PROPERTY,                CONNECT_WILL_FLAG,      CONNECT_WILL_USER_PROPERTIES,           NULL},
    {"will_topic",                  MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_TOPIC,                     NULL},
    {"will_payload",                MR_U8V_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_PAYLOAD,                   NULL},
    {"user_name",                   MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            _NA,                                    CONNECT_USERNAME_FLAG,  CONNECT_USER_NAME,                      NULL},
    {"password",                    MR_U8V_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            _NA,                                    CONNECT_PASSWORD_FLAG,  CONNECT_PASSWORD,                       NULL}
//   name                           dtype           bpos    value               valloc  vlen    u8vlen  vexists link                            propid                                  flagid                  idx                                     ovalue
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
    if (mr_validate_connect_values(pctx)) return -1;
    return mr_pack_packet(pctx);
}

int mr_free_connect_pctx(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_free_packet_context(pctx);
}

int mr_connect_printable_mdata(mr_packet_ctx *pctx, bool allflag) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_printable_mdata(pctx, allflag);
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

// bool will_flag;
int mr_get_connect_will_flag(mr_packet_ctx *pctx, bool *pboolean, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_WILL_FLAG, pboolean, pexists);
}

int mr_set_connect_will_flag(mr_packet_ctx *pctx, bool boolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    if (mr_set_scalar(pctx, CONNECT_WILL_FLAG, boolean)) return -1;

    if (boolean) {
        if (mr_set_scalar(pctx, CONNECT_WILL_PROPERTY_LENGTH, 0)) return -1; // sets vexists
    }
    else {
        if (mr_set_connect_will_qos(pctx, 0)) return -1;
        if (mr_set_connect_will_retain(pctx, 0)) return -1;

        mr_mdata *mdata = pctx->mdata0;
        for (int i = 0; i < _CONNECT_MDATA_COUNT; i++, mdata++) {
            if (mdata->flagid == CONNECT_WILL_FLAG) {
                if (mdata->dtype == MR_BITS_DTYPE) {
                    if (mr_set_scalar(pctx, mdata->idx, 0)) return -1;
                }
                else if (mdata->dtype == MR_U8V_DTYPE || mdata->dtype == MR_STR_DTYPE || mdata->dtype == MR_SPV_DTYPE) {
                    if (mr_reset_vector(pctx, mdata->idx)) return -1;
                }
                else { // some scalar dtype
                    if (mr_reset_scalar(pctx, mdata->idx)) return -1;
                }
            }
        }
    }

    return 0;
}

// uint8_t will_qos;
int mr_get_connect_will_qos(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_WILL_QOS, pu8, pexists);
}

int mr_set_connect_will_qos(mr_packet_ctx *pctx, uint8_t u8) {
    if (mr_connect_packet_check(pctx)) return -1;

    if (u8 > 0) {
        if (u8 > 2) {
            dzlog_error("will_qos out of range (0..2): %u", u8);
            return -1;
        }
        else {
            if (mr_set_connect_will_flag(pctx, true)) return -1;
        } // don't set false due to looping
    }

    return mr_set_scalar(pctx, CONNECT_WILL_QOS, u8);
}

// bool will_retain;
int mr_get_connect_will_retain(mr_packet_ctx *pctx, bool *pboolean, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_WILL_RETAIN, pboolean, pexists);
}

int mr_set_connect_will_retain(mr_packet_ctx *pctx, bool boolean) {
    if (mr_connect_packet_check(pctx)) return -1;
    if (boolean && mr_set_connect_will_flag(pctx, true)) return -1;
    return mr_set_scalar(pctx, CONNECT_WILL_RETAIN, boolean);
}

// bool password_flag;
int mr_get_connect_password_flag(mr_packet_ctx *pctx, bool *pboolean, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_PASSWORD_FLAG, pboolean, pexists);
}

// set by setting password

// bool username_flag; - set by setting username
int mr_get_connect_username_flag(mr_packet_ctx *pctx, bool *pboolean, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_USERNAME_FLAG, pboolean, pexists);
}

// uint16_t keep_alive; always exists hence never reset
int mr_get_connect_keep_alive(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u16(pctx, CONNECT_KEEP_ALIVE, pu16, pexists);
}

int mr_set_connect_keep_alive(mr_packet_ctx *pctx, uint16_t u16) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_KEEP_ALIVE, u16);
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

    if (u16 == 0) {
        dzlog_error("if present, receive_maximum must be > 0");
        return -1;
    }

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

    if (u32 == 0) {
        dzlog_error("if present, receive_maximum must be > 0");
        return -1;
    }

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

    if (u8 > 1) {
        dzlog_error("request_response_information out of range (0..1): %u", u8);
        return -1;
    }

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

    if (u8 > 1) {
        dzlog_error("request_problem_information out of range (0..1): %u", u8);
        return -1;
    }

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

// uint32_t will_property_length; - linked to will_flag
int mr_get_connect_will_property_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_WILL_PROPERTY_LENGTH, pu32, pexists);
}

 // uint32_t will_delay_interval;
int mr_get_connect_will_delay_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_WILL_DELAY_INTERVAL, pu32, pexists);
}

int mr_set_connect_will_delay_interval(mr_packet_ctx *pctx, uint32_t u32) {
    if (mr_connect_packet_check(pctx)) return -1;
    if (mr_set_connect_will_flag(pctx, true)) return -1;
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
    if (mr_set_connect_will_flag(pctx, true)) return -1;
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
    if (mr_set_connect_will_flag(pctx, true)) return -1;
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
    if (mr_set_connect_will_flag(pctx, true)) return -1;
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
    if (mr_set_connect_will_flag(pctx, true)) return -1;
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
    if (mr_set_connect_will_flag(pctx, true)) return -1;
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
    if (mr_set_connect_will_flag(pctx, true)) return -1;
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
    if (mr_set_connect_will_flag(pctx, true)) return -1;
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
    if (mr_set_connect_will_flag(pctx, true)) return -1;
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
    if (mr_set_scalar(pctx, CONNECT_USERNAME_FLAG, true)) return -1;
    return mr_set_vector(pctx, CONNECT_USER_NAME, cv0, strlen(cv0) + 1);
}

int mr_reset_connect_user_name(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    if (mr_set_scalar(pctx, CONNECT_USERNAME_FLAG, false)) return -1;
    return mr_reset_vector(pctx, CONNECT_USER_NAME);
}

// uint8_t *password;
int mr_get_connect_password(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists) {
    if (mr_connect_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_PASSWORD, pu8v0, plen, pexists);
}

int mr_set_connect_password(mr_packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    if (mr_connect_packet_check(pctx)) return -1;
    if (mr_set_scalar(pctx, CONNECT_PASSWORD_FLAG, true)) return -1;
    return mr_set_vector(pctx, CONNECT_PASSWORD, u8v0, len);
}

int mr_reset_connect_password(mr_packet_ctx *pctx) {
    if (mr_connect_packet_check(pctx)) return -1;
    if (mr_set_scalar(pctx, CONNECT_PASSWORD_FLAG, false)) return -1;
    return mr_reset_vector(pctx, CONNECT_PASSWORD);
}

// validation

int mr_validate_connect_values(mr_packet_ctx *pctx) {
    // dzlog_debug("");
    if (mr_connect_packet_check(pctx)) return -1;
    if (mr_validate_connect_cross(pctx)) return -1;
    if (mr_validate_utf8_values(pctx)) return -1;
    if (mr_validate_connect_extra(pctx)) return -1;

    return 0;
}

static int mr_validate_connect_cross(mr_packet_ctx *pctx) {
    // dzlog_debug("");
    bool bexists;
    bool will_flag;
    if (mr_get_boolean(pctx, CONNECT_WILL_FLAG, &will_flag, &bexists)) return -1;

    mr_mdata *mdata = pctx->mdata0;
    for (int i = 0; i < _CONNECT_MDATA_COUNT; i++, mdata++) {
        if (mdata->flagid == CONNECT_WILL_FLAG && !will_flag) { // governed by will_flag AND will_flag is false
            if (mdata->dtype == MR_BITS_DTYPE) { // MR_BITS_DTYPE always exist so check value
                if (mdata->value) {
                    dzlog_error("will_flag is false but '%s' has a value", mdata->name);
                    return -1;
                }
                else {
                    ; // noop
                }
            }
            else if (mdata->vexists) { // check existence for other dtypes
                dzlog_error("will_flag is false but '%s' exists", mdata->name);
                return -1;
            }
            else {
                ; // noop
            }
        }
    }

    return 0;
}

// CONNECT ptype_fn invoked from packet.c during unpack in addition to above
int mr_validate_connect_extra(mr_packet_ctx *pctx) {
    // dzlog_debug("");
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint8_t *u8v0;
    size_t len;
    char *cv0;
    bool bexists;

    // payload_format_indicator & will_payload
    if (mr_get_connect_payload_format_indicator(pctx, &u8, &bexists)) return -1;

    if (bexists) {
        if (u8 > 1) {
            dzlog_error("payload_format_indicator out of range (0..1): %u", u8);
            return -1;
        }
        else { // previous cross check of will_flag insures existence of will_payload
            if (u8 > 0 && mr_validate_u8vutf8(pctx, CONNECT_WILL_PAYLOAD)) return -1;
        }
    }

    // will_qos
    if (mr_get_connect_will_qos(pctx, &u8, &bexists)) return -1;

    if (bexists && u8 && u8 > 2) {
        dzlog_error("will_qos out of range (0..2): %u", u8);
        return -1;
    }

    // receive_maximum
    if (mr_get_connect_receive_maximum(pctx, &u16, &bexists)) return -1;

    if (bexists && !u16) {
        dzlog_error("if present, receive_maximum must be > 0");
        return -1;
    }

    // maximum_packet_size
    if (mr_get_connect_maximum_packet_size(pctx, &u32, &bexists)) return -1;

    if (bexists && !u32) {
        dzlog_error("if present, maximum_packet_size must be > 0");
        return -1;
    }

    // request_response_information
    if (mr_get_connect_request_response_information(pctx, &u8, &bexists)) return -1;

    if (bexists && u8 && u8 > 1) {
        dzlog_error("request_response_information out of range (0..1): %u", u8);
        return -1;
    }

    // request_problem_information
    if (mr_get_connect_request_problem_information(pctx, &u8, &bexists)) return -1;

    if (bexists && u8 && u8 > 1) {
        dzlog_error("request_problem_information out of range (0..1): %u", u8);
        return -1;
    }

    // authentication_data
    if (mr_get_connect_authentication_data(pctx, &u8v0, &len, &bexists)) return -1;

    if (bexists) {
        if (mr_get_connect_authentication_method(pctx, &cv0, &bexists)) return -1;

        if (!bexists) {
            dzlog_error("authentication_method must exist since authentication_data exists");
            return -1;
        }
    }

    return 0;
}
