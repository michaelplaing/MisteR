/* connack.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mister/mister.h"
#include "mister/mrzlog.h"
#include "connack_internal.h"
#include "packet_internal.h"

static const uint8_t _CONNACK_CONNECT_REASON_CODES[] = {
    MQTT_RC_SUCCESS,
    MQTT_RC_UNSPECIFIED,
    MQTT_RC_MALFORMED_PACKET,
    MQTT_RC_PROTOCOL_ERROR,
    MQTT_RC_IMPLEMENTATION_SPECIFIC,
    MQTT_RC_UNSUPPORTED_PROTOCOL_VERSION,
    MQTT_RC_CLIENTID_NOT_VALID,
    MQTT_RC_BAD_USERNAME_OR_PASSWORD,
    MQTT_RC_NOT_AUTHORIZED,
    MQTT_RC_SERVER_UNAVAILABLE,
    MQTT_RC_SERVER_BUSY,
    MQTT_RC_BANNED,
    MQTT_RC_BAD_AUTHENTICATION_METHOD,
    MQTT_RC_TOPIC_NAME_INVALID,
    MQTT_RC_PACKET_TOO_LARGE,
    MQTT_RC_QUOTA_EXCEEDED,
    MQTT_RC_PAYLOAD_FORMAT_INVALID,
    MQTT_RC_RETAIN_NOT_SUPPORTED,
    MQTT_RC_QOS_NOT_SUPPORTED,
    MQTT_RC_USE_ANOTHER_SERVER,
    MQTT_RC_SERVER_MOVED,
    MQTT_RC_CONNECTION_RATE_EXCEEDED
};
#define _CCRCSZ 22

static const uint8_t _CP[] = {
    MQTT_PROP_SESSION_EXPIRY_INTERVAL,
    MQTT_PROP_RECEIVE_MAXIMUM,
    MQTT_PROP_MAXIMUM_QOS,
    MQTT_PROP_RETAIN_AVAILABLE,
    MQTT_PROP_MAXIMUM_PACKET_SIZE,
    MQTT_PROP_ASSIGNED_CLIENT_IDENTIFIER,
    MQTT_PROP_TOPIC_ALIAS_MAXIMUM,
    MQTT_PROP_REASON_STRING,
    MQTT_PROP_USER_PROPERTY,
    MQTT_PROP_WILDCARD_SUBSCRIPTION_AVAILABLE,
    MQTT_PROP_SUBSCRIPTION_IDENTIFIERS_AVAILABLE,
    MQTT_PROP_SHARED_SUBSCRIPTION_AVAILABLE,
    MQTT_PROP_SERVER_KEEP_ALIVE,
    MQTT_PROP_RESPONSE_INFORMATION,
    MQTT_PROP_SERVER_REFERENCE,
    MQTT_PROP_AUTHENTICATION_METHOD,
    MQTT_PROP_AUTHENTICATION_DATA
};
#define _CPSZ 17

#define _NA 0

static const mr_mdata _CONNACK_MDATA_TEMPLATE[] = {
//   name                                   dtype           bpos    value               valloc  vlen    u8vlen  vexists link                            propid                                          flagid  idx                                         ovalloc     ovalue
    {"packet_type",                         MR_U8_DTYPE,    _NA,    MQTT_CONNACK,       false,  1,      1,      true,   _NA,                            _NA,                                            _NA,    CONNACK_PACKET_TYPE,                        false,      NULL},
    {"remaining_length",                    MR_VBI_DTYPE,   _NA,    0,                  false,  0,      0,      true,   CONNACK_AUTHENTICATION_DATA,    _NA,                                            _NA,    CONNACK_REMAINING_LENGTH,                   false,      NULL},
    {"session_present",                     MR_BITS_DTYPE,  0,      0,                  false,  1,      0,      true,   CONNACK_MR_FLAGS,               _NA,                                            _NA,    CONNACK_SESSION_PRESENT,                    false,      NULL},
    {"reserved",                            MR_BITS_DTYPE,  1,      0,                  false,  7,      0,      true,   CONNACK_MR_FLAGS,               _NA,                                            _NA,    CONNACK_RESERVED,                           false,      NULL},
    {"mr_flags",                            MR_FLAGS_DTYPE, _NA,    0,                  false,  1,      1,      true,   _NA,                            _NA,                                            _NA,    CONNACK_MR_FLAGS,                           false,      NULL},
    {"connect_reason_code",                 MR_U8_DTYPE,    _NA,    0,                  false,  1,      1,      true,   _NA,                            _NA,                                            _NA,    CONNACK_CONNECT_REASON_CODE,                false,      NULL},
    {"property_length",                     MR_VBI_DTYPE,   _NA,    0,                  false,  0,      0,      true,   CONNACK_AUTHENTICATION_DATA,    _NA,                                            _NA,    CONNACK_PROPERTY_LENGTH,                    false,      NULL},
    {"mr_properties",                       MR_PROPS_DTYPE, _NA,    (mr_mvalue_t)_CP,   _NA,    _CPSZ,  _NA,    true,   _NA,                            _NA,                                            _NA,    CONNACK_MR_PROPERTIES,                      false,      NULL},
    {"session_expiry_interval",             MR_U32_DTYPE,   _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_SESSION_EXPIRY_INTERVAL,              _NA,    CONNACK_SESSION_EXPIRY_INTERVAL,            false,      NULL},
    {"receive_maximum",                     MR_U16_DTYPE,   _NA,    0,                  false,  2,      3,      false,  _NA,                            MQTT_PROP_RECEIVE_MAXIMUM,                      _NA,    CONNACK_RECEIVE_MAXIMUM,                    false,      NULL},
    {"maximum_qos",                         MR_U8_DTYPE,    _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_MAXIMUM_QOS,                          _NA,    CONNACK_MAXIMUM_QOS,                        false,      NULL},
    {"retain_available",                    MR_U8_DTYPE,    _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_RETAIN_AVAILABLE,                     _NA,    CONNACK_RETAIN_AVAILABLE,                   false,      NULL},
    {"maximum_packet_size",                 MR_U32_DTYPE,   _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_MAXIMUM_PACKET_SIZE,                  _NA,    CONNACK_MAXIMUM_PACKET_SIZE,                false,      NULL},
    {"assigned_client_identifier",          MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_ASSIGNED_CLIENT_IDENTIFIER,           _NA,    CONNACK_ASSIGNED_CLIENT_IDENTIFIER,         false,      NULL},
    {"topic_alias_maximum",                 MR_U16_DTYPE,   _NA,    0,                  false,  2,      3,      false,  _NA,                            MQTT_PROP_TOPIC_ALIAS_MAXIMUM,                  _NA,    CONNACK_TOPIC_ALIAS_MAXIMUM,                false,      NULL},
    {"reason_string",                       MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_REASON_STRING,                        _NA,    CONNACK_REASON_STRING,                      false,      NULL},
    {"user_properties",                     MR_SPV_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_USER_PROPERTY,                        _NA,    CONNACK_USER_PROPERTIES,                    false,      NULL},
    {"wildcard_subscription_available",     MR_U8_DTYPE,    _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_WILDCARD_SUBSCRIPTION_AVAILABLE,      _NA,    CONNACK_WILDCARD_SUBSCRIPTION_AVAILABLE,    false,      NULL},
    {"subscription_identifiers_available",  MR_U8_DTYPE,    _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_SUBSCRIPTION_IDENTIFIERS_AVAILABLE,   _NA,    CONNACK_WILDCARD_SUBSCRIPTION_AVAILABLE,    false,      NULL},
    {"shared_subscription_available",       MR_U8_DTYPE,    _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_SHARED_SUBSCRIPTION_AVAILABLE,        _NA,    CONNACK_SHARED_SUBSCRIPTION_AVAILABLE,      false,      NULL},
    {"server_keep_alive",                   MR_U16_DTYPE,   _NA,    0,                  false,  2,      3,      false,  _NA,                            MQTT_PROP_SERVER_KEEP_ALIVE,                    _NA,    CONNACK_SERVER_KEEP_ALIVE,                  false,      NULL},
    {"response_information",                MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_RESPONSE_INFORMATION,                 _NA,    CONNACK_RESPONSE_INFORMATION,               false,      NULL},
    {"server_reference",                    MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_SERVER_REFERENCE,                     _NA,    CONNACK_SERVER_REFERENCE,                   false,      NULL},
    {"authentication_method",               MR_STR_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_AUTHENTICATION_METHOD,                _NA,    CONNACK_AUTHENTICATION_METHOD,              false,      NULL},
    {"authentication_data",                 MR_U8V_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_AUTHENTICATION_DATA,                  _NA,    CONNACK_AUTHENTICATION_DATA,                false,      NULL}
//   name                                   dtype           bpos    value               valloc  vlen    u8vlen  vexists link                            propid                                          flagid  idx                                         ovalloc     ovalue
};

static const size_t _CONNACK_MDATA_COUNT = sizeof(_CONNACK_MDATA_TEMPLATE) / sizeof(mr_mdata);

int mr_init_connack_packet(mr_packet_ctx **ppctx) {
    return mr_init_packet(ppctx, _CONNACK_MDATA_TEMPLATE, _CONNACK_MDATA_COUNT);
}

int mr_init_unpack_connack_packet(mr_packet_ctx **ppctx, uint8_t *u8v0, size_t ulen) {
    return mr_init_unpack_packet(ppctx, _CONNACK_MDATA_TEMPLATE, _CONNACK_MDATA_COUNT, u8v0, ulen);
}

static int mr_connack_packet_check(mr_packet_ctx *pctx) {
    if (pctx->mqtt_packet_type == MQTT_CONNACK) {
        return 0;
    }
    else {
        dzlog_info("Packet Context is not a CONNACK packet");
        return -1;
    }
}

int mr_pack_connack_packet(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    if (mr_validate_connack_values(pctx)) return -1;
    return mr_pack_packet(pctx);
}

int mr_free_connack_packet(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_free_packet_context(pctx);
}

int mr_connack_mdata_dump(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_printable_mdata(pctx);
}

// const uint8_t packet_type;
int mr_get_connack_packet_type(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_PACKET_TYPE, pu8, pexists);
}

// uint32_t remaining_length;
int mr_get_connack_remaining_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNACK_REMAINING_LENGTH, pu32, pexists);
}

// bool session_present;
int mr_get_connack_session_present(mr_packet_ctx *pctx, bool *pboolean, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNACK_SESSION_PRESENT, pboolean, pexists);
}

int mr_set_connack_session_present(mr_packet_ctx *pctx, bool boolean) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_SESSION_PRESENT, boolean);
}

// uint8_t reserved; // bits 1-7
int mr_get_connack_reserved(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_RESERVED, pu8, pexists);
}

// uint8_t connect_reason_code;
int mr_get_connack_connect_reason_code(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_CONNECT_REASON_CODE, pu8, pexists);
}

int mr_set_connack_connect_reason_code(mr_packet_ctx *pctx, uint8_t u8) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_CONNECT_REASON_CODE, u8);
}

// uint32_t property_length;
int mr_get_connack_property_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNACK_PROPERTY_LENGTH, pu32, pexists);
}

// uint32_t session_expiry_interval;
int mr_get_connack_session_expiry_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNACK_SESSION_EXPIRY_INTERVAL, pu32, pexists);
}

int mr_set_connack_session_expiry_interval(mr_packet_ctx *pctx, uint32_t u32) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_SESSION_EXPIRY_INTERVAL, u32);
}

int mr_reset_connack_session_expiry_interval(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_SESSION_EXPIRY_INTERVAL);
}

// uint16_t receive_maximum;
int mr_get_connack_receive_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u16(pctx, CONNACK_RECEIVE_MAXIMUM, pu16, pexists);
}

int mr_set_connack_receive_maximum(mr_packet_ctx *pctx, uint16_t u16) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_RECEIVE_MAXIMUM, u16);
}

int mr_reset_connack_receive_maximum(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_RECEIVE_MAXIMUM);
}

// uint8_t maximum_qos;
int mr_get_connack_maximum_qos(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_MAXIMUM_QOS, pu8, pexists);
}

int mr_set_connack_maximum_qos(mr_packet_ctx *pctx, uint8_t u8) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_MAXIMUM_QOS, u8);
}

int mr_reset_connack_maximum_qos(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_MAXIMUM_QOS);
}

// uint8_t retain_available;
int mr_get_connack_retain_available(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_RETAIN_AVAILABLE, pu8, pexists);
}

int mr_set_connack_retain_available(mr_packet_ctx *pctx, uint8_t u8) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_RETAIN_AVAILABLE, u8);
}

int mr_reset_connack_retain_available(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_RETAIN_AVAILABLE);
}

// uint32_t maximum_packet_size;
int mr_get_connack_maximum_packet_size(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNACK_MAXIMUM_PACKET_SIZE, pu32, pexists);
}

int mr_set_connack_maximum_packet_size(mr_packet_ctx *pctx, uint32_t u32) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_MAXIMUM_PACKET_SIZE, u32);
}

int mr_reset_connack_maximum_packet_size(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_MAXIMUM_PACKET_SIZE);
}

// uint8_t *assigned_client_identifier;
int mr_get_connack_assigned_client_identifier(mr_packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNACK_ASSIGNED_CLIENT_IDENTIFIER, pcv0, pexists);
}

int mr_set_connack_assigned_client_identifier(mr_packet_ctx *pctx, char *cv0) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_ASSIGNED_CLIENT_IDENTIFIER, cv0, strlen(cv0) + 1);
}

int mr_reset_connack_assigned_client_identifier(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_ASSIGNED_CLIENT_IDENTIFIER);
}

// uint16_t topic_alias_maximum;
int mr_get_connack_topic_alias_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u16(pctx, CONNACK_TOPIC_ALIAS_MAXIMUM, pu16, pexists);
}

int mr_set_connack_topic_alias_maximum(mr_packet_ctx *pctx, uint16_t u16) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_TOPIC_ALIAS_MAXIMUM, u16);
}

int mr_reset_connack_topic_alias_maximum(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_TOPIC_ALIAS_MAXIMUM);
}

// uint8_t *reason_string;
int mr_get_connack_reason_string(mr_packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNACK_REASON_STRING, pcv0, pexists);
}

int mr_set_connack_reason_string(mr_packet_ctx *pctx, char *cv0) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_REASON_STRING, cv0, strlen(cv0) + 1);
}

int mr_reset_connack_reason_string(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_REASON_STRING);
}

// mr_string_pair *user_properties;
int mr_get_connack_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_spv(pctx, CONNACK_USER_PROPERTIES, pspv0, plen, pexists);
}

int mr_set_connack_user_properties(mr_packet_ctx *pctx, mr_string_pair *spv0, size_t len) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_USER_PROPERTIES, spv0, len);
}

int mr_reset_connack_user_properties(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_USER_PROPERTIES);
}

// uint8_t wildcard_subscription_available;
int mr_get_connack_wildcard_subscription_available(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_WILDCARD_SUBSCRIPTION_AVAILABLE, pu8, pexists);
}

int mr_set_connack_wildcard_subscription_available(mr_packet_ctx *pctx, uint8_t u8) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_WILDCARD_SUBSCRIPTION_AVAILABLE, u8);
}

int mr_reset_connack_wildcard_subscription_available(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_WILDCARD_SUBSCRIPTION_AVAILABLE);
}

// uint8_t subscription_identifiers_available;
int mr_get_connack_subscription_identifiers_available(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_SUBSCRIPTION_IDENTIFIERS_AVAILABLE, pu8, pexists);
}

int mr_set_connack_subscription_identifiers_available(mr_packet_ctx *pctx, uint8_t u8) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_SUBSCRIPTION_IDENTIFIERS_AVAILABLE, u8);
}

int mr_reset_connack_subscription_identifiers_available(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_SUBSCRIPTION_IDENTIFIERS_AVAILABLE);
}

// uint8_t shared_subscription_available;
int mr_get_connack_shared_subscription_available(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_SHARED_SUBSCRIPTION_AVAILABLE, pu8, pexists);
}

int mr_set_connack_shared_subscription_available(mr_packet_ctx *pctx, uint8_t u8) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_SHARED_SUBSCRIPTION_AVAILABLE, u8);
}

int mr_reset_connack_shared_subscription_available(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_SHARED_SUBSCRIPTION_AVAILABLE);
}

// uint16_t server_keep_alive;
int mr_get_connack_server_keep_alive(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u16(pctx, CONNACK_SERVER_KEEP_ALIVE, pu16, pexists);
}

int mr_set_connack_server_keep_alive(mr_packet_ctx *pctx, uint16_t u16) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_SERVER_KEEP_ALIVE, u16);
}

int mr_reset_connack_server_keep_alive(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_SERVER_KEEP_ALIVE);
}
// uint8_t *response_information;
int mr_get_connack_response_information(mr_packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNACK_RESPONSE_INFORMATION, pcv0, pexists);
}

int mr_set_connack_response_information(mr_packet_ctx *pctx, char *cv0) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_RESPONSE_INFORMATION, cv0, strlen(cv0) + 1);
}

int mr_reset_connack_response_information(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_RESPONSE_INFORMATION);
}

// uint8_t *server_reference;
int mr_get_connack_server_reference(mr_packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNACK_SERVER_REFERENCE, pcv0, pexists);
}

int mr_set_connack_server_reference(mr_packet_ctx *pctx, char *cv0) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_SERVER_REFERENCE, cv0, strlen(cv0) + 1);
}

int mr_reset_connack_server_reference(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_SERVER_REFERENCE);
}

// uint8_t *authentication_method;
int mr_get_connack_authentication_method(mr_packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNACK_AUTHENTICATION_METHOD, pcv0, pexists);
}

int mr_set_connack_authentication_method(mr_packet_ctx *pctx, char *cv0) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_AUTHENTICATION_METHOD, cv0, strlen(cv0) + 1);
}

int mr_reset_connack_authentication_method(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_AUTHENTICATION_METHOD);
}

// uint8_t *authentication_data;
int mr_get_connack_authentication_data(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNACK_AUTHENTICATION_DATA, pu8v0, plen, pexists);
}

int mr_set_connack_authentication_data(mr_packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_AUTHENTICATION_DATA, u8v0, len);
}

int mr_reset_connack_authentication_data(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_AUTHENTICATION_DATA);
}

// validation

int mr_validate_connack_values(mr_packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    // if (mr_validate_connect_cross(pctx)) return -1;
    if (mr_validate_utf8_values(pctx)) return -1;
    if (mr_validate_connack_extra(pctx)) return -1;

    return 0;
}

int mr_validate_connack_extra(mr_packet_ctx *pctx) {
    uint8_t u8;
    bool bexists;
    // connect_reason_code
    if (mr_get_connack_connect_reason_code(pctx, &u8, &bexists)) return -1;

    if (!memchr(_CONNACK_CONNECT_REASON_CODES, u8, _CCRCSZ)) {
        dzlog_error("connect_reason_code not found");
        return -1;
    }

    return 0;
}