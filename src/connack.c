/* connack.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mister/mister.h"
#include "mister/mrzlog.h"
#include "connack_internal.h"
#include "packet_internal.h"

static const uint8_t MRCP[] = {
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
#define MRCPSZ 17

#define NA 0

static const mr_mdata CONNACK_MDATA_TEMPLATE[] = {
//   name                                   dtype           bp  value           valloc  vlen    u8vlen  vexists link                            propid                                          flagid  idx                                         pvalloc     pvalue
    {"packet_type",                         MR_U8_DTYPE,    NA, MQTT_CONNACK,   false,  1,      1,      true,   NA,                             NA,                                             NA,     CONNACK_PACKET_TYPE,                        false,      NULL},
    {"remaining_length",                    MR_VBI_DTYPE,   NA, 0,              false,  0,      0,      true,   CONNACK_AUTHENTICATION_DATA,    NA,                                             NA,     CONNACK_REMAINING_LENGTH,                   false,      NULL},
    {"session_present",                     MR_BITS_DTYPE,  0,  0,              false,  1,      0,      true,   CONNACK_MR_FLAGS,               NA,                                             NA,     CONNACK_SESSION_PRESENT,                    false,      NULL},
    {"reserved",                            MR_BITS_DTYPE,  1,  0,              false,  7,      0,      true,   CONNACK_MR_FLAGS,               NA,                                             NA,     CONNACK_RESERVED,                           false,      NULL},
    {"mr_flags",                            MR_FLAGS_DTYPE, NA, 0,              false,  1,      1,      true,   NA,                             NA,                                             NA,     CONNACK_MR_FLAGS,                           false,      NULL},
    {"connect_reason_code",                 MR_U8_DTYPE,    NA, 0,              false,  1,      1,      true,   NA,                             NA,                                             NA,     CONNACK_CONNECT_REASON_CODE,                false,      NULL},
    {"property_length",                     MR_VBI_DTYPE,   NA, 0,              false,  0,      0,      true,   CONNACK_AUTHENTICATION_DATA,    NA,                                             NA,     CONNACK_PROPERTY_LENGTH,                    false,      NULL},
    {"mr_properties",                       MR_PROPS_DTYPE, NA, (Word_t)MRCP,   false,  MRCPSZ, NA,     true,   NA,                             NA,                                             NA,     CONNACK_MR_PROPERTIES,                      false,      NULL},
    {"session_expiry_interval",             MR_U32_DTYPE,   NA, 0,              false,  4,      5,      false,  NA,                             MQTT_PROP_SESSION_EXPIRY_INTERVAL,              NA,     CONNACK_SESSION_EXPIRY_INTERVAL,            false,      NULL},
    {"receive_maximum",                     MR_U16_DTYPE,   NA, 0,              false,  2,      3,      false,  NA,                             MQTT_PROP_RECEIVE_MAXIMUM,                      NA,     CONNACK_RECEIVE_MAXIMUM,                    false,      NULL},
    {"maximum_qos",                         MR_U8_DTYPE,    NA, 0,              false,  1,      2,      false,  NA,                             MQTT_PROP_MAXIMUM_QOS,                          NA,     CONNACK_MAXIMUM_QOS,                        false,      NULL},
    {"retain_available",                    MR_U8_DTYPE,    NA, 0,              false,  1,      2,      false,  NA,                             MQTT_PROP_RETAIN_AVAILABLE,                     NA,     CONNACK_RETAIN_AVAILABLE,                   false,      NULL},
    {"maximum_packet_size",                 MR_U32_DTYPE,   NA, 0,              false,  4,      5,      false,  NA,                             MQTT_PROP_MAXIMUM_PACKET_SIZE,                  NA,     CONNACK_MAXIMUM_PACKET_SIZE,                false,      NULL},
    {"assigned_client_identifier",          MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      0,      false,  NA,                             MQTT_PROP_ASSIGNED_CLIENT_IDENTIFIER,           NA,     CONNACK_ASSIGNED_CLIENT_IDENTIFIER,         false,      NULL},
    {"topic_alias_maximum",                 MR_U16_DTYPE,   NA, 0,              false,  2,      3,      false,  NA,                             MQTT_PROP_TOPIC_ALIAS_MAXIMUM,                  NA,     CONNACK_TOPIC_ALIAS_MAXIMUM,                false,      NULL},
    {"reason_string",                       MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      0,      false,  NA,                             MQTT_PROP_REASON_STRING,                        NA,     CONNACK_REASON_STRING,                      false,      NULL},
    {"user_properties",                     MR_SPV_DTYPE,   NA, (Word_t)NULL,   false,  0,      0,      false,  NA,                             MQTT_PROP_USER_PROPERTY,                        NA,     CONNACK_USER_PROPERTIES,                    false,      NULL},
    {"wildcard_subscription_available",     MR_U8_DTYPE,    NA, 0,              false,  1,      2,      false,  NA,                             MQTT_PROP_WILDCARD_SUBSCRIPTION_AVAILABLE,      NA,     CONNACK_WILDCARD_SUBSCRIPTION_AVAILABLE,    false,      NULL},
    {"subscription_identifiers_available",  MR_U8_DTYPE,    NA, 0,              false,  1,      2,      false,  NA,                             MQTT_PROP_SUBSCRIPTION_IDENTIFIERS_AVAILABLE,   NA,     CONNACK_WILDCARD_SUBSCRIPTION_AVAILABLE,    false,      NULL},
    {"shared_subscription_available",       MR_U8_DTYPE,    NA, 0,              false,  1,      2,      false,  NA,                             MQTT_PROP_SHARED_SUBSCRIPTION_AVAILABLE,        NA,     CONNACK_SHARED_SUBSCRIPTION_AVAILABLE,      false,      NULL},
    {"server_keep_alive",                   MR_U16_DTYPE,   NA, 0,              false,  2,      3,      false,  NA,                             MQTT_PROP_SERVER_KEEP_ALIVE,                    NA,     CONNACK_SERVER_KEEP_ALIVE,                  false,      NULL},
    {"response_information",                MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      0,      false,  NA,                             MQTT_PROP_RESPONSE_INFORMATION,                 NA,     CONNACK_RESPONSE_INFORMATION,               false,      NULL},
    {"server_reference",                    MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      0,      false,  NA,                             MQTT_PROP_SERVER_REFERENCE,                     NA,     CONNACK_SERVER_REFERENCE,                   false,      NULL},
    {"authentication_method",               MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      0,      false,  NA,                             MQTT_PROP_AUTHENTICATION_METHOD,                NA,     CONNACK_AUTHENTICATION_METHOD,              false,      NULL},
    {"authentication_data",                 MR_U8V_DTYPE,   NA, (Word_t)NULL,   false,  0,      0,      false,  NA,                             MQTT_PROP_AUTHENTICATION_DATA,                  NA,     CONNACK_AUTHENTICATION_DATA,                false,      NULL}
//   name                                   dtype           bp  value           valloc  vlen    u8vlen  vexists link                            propid                                          flagid  idx                                         spvalloc    spv
};

int mr_init_connack_packet(packet_ctx **ppctx) {
    size_t mdata_count = sizeof(CONNACK_MDATA_TEMPLATE) / sizeof(mr_mdata);
    return mr_init_packet(ppctx, CONNACK_MDATA_TEMPLATE, mdata_count);
}

int mr_init_unpack_connack_packet(packet_ctx **ppctx, uint8_t *u8v0, size_t ulen) {
    size_t mdata_count = sizeof(CONNACK_MDATA_TEMPLATE) / sizeof(mr_mdata);
    return mr_init_unpack_packet(ppctx, CONNACK_MDATA_TEMPLATE, mdata_count, u8v0, ulen);
}

static int mr_connack_packet_check(packet_ctx *pctx) {
    if (pctx->mqtt_packet_type == MQTT_CONNACK) {
        return 0;
    }
    else {
        dzlog_info("Packet Context is not a CONNACK packet");
        return -1;
    }
}

int mr_pack_connack_packet(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_pack_packet(pctx);
}

int mr_free_connack_packet(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_free_packet_context(pctx);
}

int mr_connack_mdata_dump(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_mdata_dump(pctx);
}

// const uint8_t packet_type;
int mr_get_connack_packet_type(packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_PACKET_TYPE, pu8, pexists);
}

// uint32_t remaining_length;
int mr_get_connack_remaining_length(packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNACK_REMAINING_LENGTH, pu32, pexists);
}

// bool session_present;
int mr_get_connack_session_present(packet_ctx *pctx, bool *pboolean, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_boolean(pctx, CONNACK_SESSION_PRESENT, pboolean, pexists);
}

int mr_set_connack_session_present(packet_ctx *pctx, bool boolean) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_SESSION_PRESENT, boolean);
}

int mr_reset_connack_session_present(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_SESSION_PRESENT);
}

// uint8_t reserved; // bits 1-7
int mr_get_connack_reserved(packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_RESERVED, pu8, pexists);
}

// uint8_t connect_reason_code;
int mr_get_connack_connect_reason_code(packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_CONNECT_REASON_CODE, pu8, pexists);
}

int mr_set_connack_connect_reason_code(packet_ctx *pctx, uint8_t u8) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_CONNECT_REASON_CODE, u8);
}

int mr_reset_connack_connect_reason_code(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_CONNECT_REASON_CODE);
}

// uint32_t property_length;
int mr_get_connack_property_length(packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNACK_PROPERTY_LENGTH, pu32, pexists);
}

// uint32_t session_expiry_interval;
int mr_get_connack_session_expiry_interval(packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNACK_SESSION_EXPIRY_INTERVAL, pu32, pexists);
}

int mr_set_connack_session_expiry_interval(packet_ctx *pctx, uint32_t u32) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_SESSION_EXPIRY_INTERVAL, u32);
}

int mr_reset_connack_session_expiry_interval(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_SESSION_EXPIRY_INTERVAL);
}

// uint16_t receive_maximum;
int mr_get_connack_receive_maximum(packet_ctx *pctx, uint16_t *pu16, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u16(pctx, CONNACK_RECEIVE_MAXIMUM, pu16, pexists);
}

int mr_set_connack_receive_maximum(packet_ctx *pctx, uint16_t u16) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_RECEIVE_MAXIMUM, u16);
}

int mr_reset_connack_receive_maximum(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_RECEIVE_MAXIMUM);
}

// uint8_t maximum_qos;
int mr_get_connack_maximum_qos(packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_MAXIMUM_QOS, pu8, pexists);
}

int mr_set_connack_maximum_qos(packet_ctx *pctx, uint8_t u8) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_MAXIMUM_QOS, u8);
}

int mr_reset_connack_maximum_qos(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_MAXIMUM_QOS);
}

// uint8_t retain_available;
int mr_get_connack_retain_available(packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_RETAIN_AVAILABLE, pu8, pexists);
}

int mr_set_connack_retain_available(packet_ctx *pctx, uint8_t u8) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_RETAIN_AVAILABLE, u8);
}

int mr_reset_connack_retain_available(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_RETAIN_AVAILABLE);
}

// uint32_t maximum_packet_size;
int mr_get_connack_maximum_packet_size(packet_ctx *pctx, uint32_t *pu32, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u32(pctx, CONNACK_MAXIMUM_PACKET_SIZE, pu32, pexists);
}

int mr_set_connack_maximum_packet_size(packet_ctx *pctx, uint32_t u32) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_MAXIMUM_PACKET_SIZE, u32);
}

int mr_reset_connack_maximum_packet_size(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_MAXIMUM_PACKET_SIZE);
}

// uint8_t *assigned_client_identifier;
int mr_get_connack_assigned_client_identifier(packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNACK_ASSIGNED_CLIENT_IDENTIFIER, pcv0, pexists);
}

int mr_set_connack_assigned_client_identifier(packet_ctx *pctx, char *cv0) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_ASSIGNED_CLIENT_IDENTIFIER, cv0, strlen(cv0) + 1);
}

int mr_reset_connack_assigned_client_identifier(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_ASSIGNED_CLIENT_IDENTIFIER);
}

// uint16_t topic_alias_maximum;
int mr_get_connack_topic_alias_maximum(packet_ctx *pctx, uint16_t *pu16, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u16(pctx, CONNACK_TOPIC_ALIAS_MAXIMUM, pu16, pexists);
}

int mr_set_connack_topic_alias_maximum(packet_ctx *pctx, uint16_t u16) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_TOPIC_ALIAS_MAXIMUM, u16);
}

int mr_reset_connack_topic_alias_maximum(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_TOPIC_ALIAS_MAXIMUM);
}

// uint8_t *reason_string;
int mr_get_connack_reason_string(packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNACK_REASON_STRING, pcv0, pexists);
}

int mr_set_connack_reason_string(packet_ctx *pctx, char *cv0) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_REASON_STRING, cv0, strlen(cv0) + 1);
}

int mr_reset_connack_reason_string(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_REASON_STRING);
}

// string_pair *user_properties;
int mr_get_connack_user_properties(packet_ctx *pctx, string_pair **pspv0, size_t *plen, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_spv(pctx, CONNACK_USER_PROPERTIES, pspv0, plen, pexists);
}

int mr_set_connack_user_properties(packet_ctx *pctx, string_pair *spv0, size_t len) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_USER_PROPERTIES, spv0, len);
}

int mr_reset_connack_user_properties(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_USER_PROPERTIES);
}

// uint8_t wildcard_subscription_available;
int mr_get_connack_wildcard_subscription_available(packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_WILDCARD_SUBSCRIPTION_AVAILABLE, pu8, pexists);
}

int mr_set_connack_wildcard_subscription_available(packet_ctx *pctx, uint8_t u8) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_WILDCARD_SUBSCRIPTION_AVAILABLE, u8);
}

int mr_reset_connack_wildcard_subscription_available(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_WILDCARD_SUBSCRIPTION_AVAILABLE);
}

// uint8_t subscription_identifiers_available;
int mr_get_connack_subscription_identifiers_available(packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_SUBSCRIPTION_IDENTIFIERS_AVAILABLE, pu8, pexists);
}

int mr_set_connack_subscription_identifiers_available(packet_ctx *pctx, uint8_t u8) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_SUBSCRIPTION_IDENTIFIERS_AVAILABLE, u8);
}

int mr_reset_connack_subscription_identifiers_available(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_SUBSCRIPTION_IDENTIFIERS_AVAILABLE);
}

// uint8_t shared_subscription_available;
int mr_get_connack_shared_subscription_available(packet_ctx *pctx, uint8_t *pu8, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8(pctx, CONNACK_SHARED_SUBSCRIPTION_AVAILABLE, pu8, pexists);
}

int mr_set_connack_shared_subscription_available(packet_ctx *pctx, uint8_t u8) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_SHARED_SUBSCRIPTION_AVAILABLE, u8);
}

int mr_reset_connack_shared_subscription_available(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_SHARED_SUBSCRIPTION_AVAILABLE);
}

// uint16_t server_keep_alive;
int mr_get_connack_server_keep_alive(packet_ctx *pctx, uint16_t *pu16, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u16(pctx, CONNACK_SERVER_KEEP_ALIVE, pu16, pexists);
}

int mr_set_connack_server_keep_alive(packet_ctx *pctx, uint16_t u16) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_scalar(pctx, CONNACK_SERVER_KEEP_ALIVE, u16);
}

int mr_reset_connack_server_keep_alive(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNACK_SERVER_KEEP_ALIVE);
}
// uint8_t *response_information;
int mr_get_connack_response_information(packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNACK_RESPONSE_INFORMATION, pcv0, pexists);
}

int mr_set_connack_response_information(packet_ctx *pctx, char *cv0) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_RESPONSE_INFORMATION, cv0, strlen(cv0) + 1);
}

int mr_reset_connack_response_information(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_RESPONSE_INFORMATION);
}

// uint8_t *server_reference;
int mr_get_connack_server_reference(packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNACK_SERVER_REFERENCE, pcv0, pexists);
}

int mr_set_connack_server_reference(packet_ctx *pctx, char *cv0) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_SERVER_REFERENCE, cv0, strlen(cv0) + 1);
}

int mr_reset_connack_server_reference(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_SERVER_REFERENCE);
}

// uint8_t *authentication_method;
int mr_get_connack_authentication_method(packet_ctx *pctx, char **pcv0, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_str(pctx, CONNACK_AUTHENTICATION_METHOD, pcv0, pexists);
}

int mr_set_connack_authentication_method(packet_ctx *pctx, char *cv0) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_AUTHENTICATION_METHOD, cv0, strlen(cv0) + 1);
}

int mr_reset_connack_authentication_method(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_AUTHENTICATION_METHOD);
}

// uint8_t *authentication_data;
int mr_get_connack_authentication_data(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_get_u8v(pctx, CONNACK_AUTHENTICATION_DATA, pu8v0, plen, pexists);
}

int mr_set_connack_authentication_data(packet_ctx *pctx, uint8_t *u8v0, size_t len) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_set_vector(pctx, CONNACK_AUTHENTICATION_DATA, u8v0, len);
}

int mr_reset_connack_authentication_data(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_reset_vector(pctx, CONNACK_AUTHENTICATION_DATA);
}
