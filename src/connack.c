/* connack.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mister/mister.h"
#include "connack_internal.h"
#include "packet_internal.h"

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

#define NA 0

static const mr_mdata CONNACK_MDATA_TEMPLATE[] = {
//   name                           isprop  dtype           value           valloc  vlen    vexists link                            xf                                      idx                                     ualloc u8vlen  u8v0
    {"packet_type",                 false,  MR_U8_DTYPE,    MQTT_CONNACK,   false,  1,      true,   0,                              NA,                                     CONNACK_PACKET_TYPE,                    false,  0,      NULL},
    {"remaining_length",            false,  MR_VBI_DTYPE,   0,              false,  0,      true,   CONNACK_REMAINING_LENGTH,       NA,                                     CONNACK_REMAINING_LENGTH,               false,  0,      NULL},
//   name                           isprop, dtype           value           valloc  vlen    vexists link                            xf                                      idx                                     ualloc u8vlen  u8v0
};

int mr_init_connack_pctx(packet_ctx **ppctx) {
    size_t mdata_count = sizeof(CONNACK_MDATA_TEMPLATE) / sizeof(mr_mdata);
    return mr_init_packet_context(ppctx, CONNACK_MDATA_TEMPLATE, mdata_count);
}

int mr_pack_connack_u8v0(packet_ctx *pctx) {
    return mr_pack_mdata_u8v0(pctx);
}

int mr_unpack_connack_u8v0(packet_ctx *pctx) {
    return mr_unpack_mdata_u8v0(pctx);
}

int mr_free_connack_pctx(packet_ctx *pctx) {
    return mr_free_packet_context(pctx);
}
