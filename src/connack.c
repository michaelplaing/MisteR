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
//   name                                   dtype           bp  value           valloc  vlen    vexists link                            propid                                          flagid  idx                                         ualloc  u8vlen  u8v0,   pvalloc     pvalue
    {"packet_type",                         MR_U8_DTYPE,    NA, MQTT_CONNACK,   false,  1,      true,   NA,                             NA,                                             NA,     CONNACK_PACKET_TYPE,                        false,  0,      NULL,   false,      NULL},
    {"remaining_length",                    MR_VBI_DTYPE,   NA, 0,              false,  0,      true,   CONNACK_AUTHENTICATION_DATA,    NA,                                             NA,     CONNACK_REMAINING_LENGTH,                   false,  0,      NULL,   false,      NULL},
    {"session_present",                     MR_FLAGS_DTYPE, 0,  0,              false,  1,      true,   CONNACK_MR_FLAGS,               NA,                                             NA,     CONNACK_SESSION_PRESENT,                    false,  0,      NULL,   false,      NULL},
    {"reserved",                            MR_FLAGS_DTYPE, 1,  0,              false,  7,      true,   CONNACK_MR_FLAGS,               NA,                                             NA,     CONNACK_RESERVED,                           false,  0,      NULL,   false,      NULL},
    {"mr_flags",                            MR_U8_DTYPE,    NA, 0,              false,  1,      true,   NA,                             NA,                                             NA,     CONNACK_MR_FLAGS,                           false,  0,      NULL,   false,      NULL},
    {"connect_reason_code",                 MR_U8_DTYPE,    NA, 0,              false,  0,      true,   NA,                             NA,                                             NA,     CONNACK_CONNECT_REASON_CODE,                false,  0,      NULL,   false,      NULL},
    {"property_length",                     MR_VBI_DTYPE,   NA, 0,              false,  0,      true,   CONNACK_AUTHENTICATION_DATA,    NA,                                             NA,     CONNACK_PROPERTY_LENGTH,                    false,  0,      NULL,   false,      NULL},
    {"mr_properties",                       MR_PROPS_DTYPE, NA, (Word_t)MRCP,   false,  MRCPSZ, true,   NA,                             NA,                                             NA,     CONNACK_MR_PROPERTIES,                      false,  0,      NULL,   false,      NULL},
    {"session_expiry_interval",             MR_U32_DTYPE,   NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_SESSION_EXPIRY_INTERVAL,              NA,     CONNACK_SESSION_EXPIRY_INTERVAL,            false,  0,      NULL,   false,      NULL},
    {"receive_maximum",                     MR_U16_DTYPE,   NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_RECEIVE_MAXIMUM,                      NA,     CONNACK_RECEIVE_MAXIMUM,                    false,  0,      NULL,   false,      NULL},
    {"maximum_qos",                         MR_U8_DTYPE,    NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_MAXIMUM_QOS,                          NA,     CONNACK_MAXIMUM_QOS,                        false,  0,      NULL,   false,      NULL},
    {"retain_available",                    MR_U8_DTYPE,    NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_RETAIN_AVAILABLE,                     NA,     CONNACK_RETAIN_AVAILABLE,                   false,  0,      NULL,   false,      NULL},
    {"maximum_packet_size",                 MR_U32_DTYPE,   NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_MAXIMUM_PACKET_SIZE,                  NA,     CONNACK_MAXIMUM_PACKET_SIZE,                false,  0,      NULL,   false,      NULL},
    {"assigned_client_identifier",          MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_ASSIGNED_CLIENT_IDENTIFIER,           NA,     CONNACK_ASSIGNED_CLIENT_IDENTIFIER,         false,  0,      NULL,   false,      NULL},
    {"topic_alias_maximum",                 MR_U16_DTYPE,   NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_TOPIC_ALIAS_MAXIMUM,                  NA,     CONNACK_TOPIC_ALIAS_MAXIMUM,                false,  0,      NULL,   false,      NULL},
    {"reason_string",                       MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_REASON_STRING,                        NA,     CONNACK_REASON_STRING,                      false,  0,      NULL,   false,      NULL},
    {"user_properties",                     MR_SPV_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_USER_PROPERTY,                        NA,     CONNACK_USER_PROPERTIES,                    false,  0,      NULL,   false,      NULL},
    {"wildcard_subscription_available",     MR_U8_DTYPE,    NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_WILDCARD_SUBSCRIPTION_AVAILABLE,      NA,     CONNACK_WILDCARD_SUBSCRIPTION_AVAILABLE,    false,  0,      NULL,   false,      NULL},
    {"subscription_identifiers_available",  MR_U8_DTYPE,    NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_SUBSCRIPTION_IDENTIFIERS_AVAILABLE,   NA,     CONNACK_WILDCARD_SUBSCRIPTION_AVAILABLE,    false,  0,      NULL,   false,      NULL},
    {"shared_subscription_available",       MR_U8_DTYPE,    NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_SHARED_SUBSCRIPTION_AVAILABLE,        NA,     CONNACK_SHARED_SUBSCRIPTION_AVAILABLE,      false,  0,      NULL,   false,      NULL},
    {"server_keep_alive",                   MR_U16_DTYPE,   NA, 0,              false,  0,      false,  NA,                             MQTT_PROP_SERVER_KEEP_ALIVE,                    NA,     CONNACK_SERVER_KEEP_ALIVE,                  false,  0,      NULL,   false,      NULL},
    {"response_information",                MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_RESPONSE_INFORMATION,                 NA,     CONNACK_RESPONSE_INFORMATION,               false,  0,      NULL,   false,      NULL},
    {"server_reference",                    MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_SERVER_REFERENCE,                     NA,     CONNACK_SERVER_REFERENCE,                   false,  0,      NULL,   false,      NULL},
    {"authentication_method",               MR_STR_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_AUTHENTICATION_METHOD,                NA,     CONNACK_AUTHENTICATION_METHOD,              false,  0,      NULL,   false,      NULL},
    {"authentication_data",                 MR_U8V_DTYPE,   NA, (Word_t)NULL,   false,  0,      false,  NA,                             MQTT_PROP_AUTHENTICATION_DATA,                  NA,     CONNACK_AUTHENTICATION_DATA,                false,  0,      NULL,   false,      NULL}
//   name                                   dtype           bp  value           valloc  vlen    vexists link                            propid                                          flagid  idx                                         ualloc  u8vlen  u8v0,   spvalloc    spv
};

int mr_init_connack_pctx(packet_ctx **ppctx) {
    size_t mdata_count = sizeof(CONNACK_MDATA_TEMPLATE) / sizeof(mr_mdata);
    return mr_init_packet_context(ppctx, CONNACK_MDATA_TEMPLATE, mdata_count);
}

int mr_init_unpack_connack_pctx(packet_ctx **ppctx, uint8_t *u8v0, size_t ulen) {
    size_t mdata_count = sizeof(CONNACK_MDATA_TEMPLATE) / sizeof(mr_mdata);
    return mr_init_unpack_pctx(ppctx, CONNACK_MDATA_TEMPLATE, mdata_count, u8v0, ulen);
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

int mr_pack_connack_u8v0(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_pack_pctx_u8v0(pctx);
}
/*
int mr_unpack_connack_u8v0(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_unpack_pctx_u8v0(pctx);
}
*/
int mr_free_connack_pctx(packet_ctx *pctx) {
    if (mr_connack_packet_check(pctx)) return -1;
    return mr_free_packet_context(pctx);
}
