/* connack.c */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <zlog.h>

#include "mister_internal.h"

enum PUBLISH_MDATA_FIELDS { // Same order as _PUBLISH_MDATA_TEMPLATE
    PUBLISH_PACKET_TYPE,
    PUBLISH_DUP,
    PUBLISH_QOS,
    PUBLISH_RETAIN,
    PUBLISH_MR_HEADER,
    PUBLISH_REMAINING_LENGTH,
    PUBLISH_TOPIC_NAME,
    PUBLISH_PACKET_IDENTIFIER,
    PUBLISH_PROPERTY_LENGTH,
    PUBLISH_MR_PROPERTIES,
    PUBLISH_PAYLOAD_FORMAT_INDICATOR,
    PUBLISH_MESSAGE_EXPIRY_INTERVAL,
    PUBLISH_TOPIC_ALIAS,
    PUBLISH_RESPONSE_TOPIC,
    PUBLISH_CORRELATION_DATA,
    PUBLISH_USER_PROPERTIES,
    PUBLISH_SUBSCRIPTION_IDENTIFIERS,
    PUBLISH_CONTENT_TYPE,
    PUBLISH_PAYLOAD
};

static const uint8_t _PROPS[] = {
    MQTT_PROP_PAYLOAD_FORMAT_INDICATOR,
    MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,
    MQTT_PROP_TOPIC_ALIAS,
    MQTT_PROP_RESPONSE_TOPIC,
    MQTT_PROP_CORRELATION_DATA,
    MQTT_PROP_USER_PROPERTY,
    MQTT_PROP_SUBSCRIPTION_IDENTIFIER,
    MQTT_PROP_CONTENT_TYPE,
};
#define _PSZ 8

#define _NA 0

static const char _S0L[] = "";
static const mr_mvalue_t _MR_PUBLISH_HEADER = MQTT_PUBLISH << 4;

static const mr_mdata _PUBLISH_MDATA_TEMPLATE[] = {
//   name                       dtype               bitpos  value               valloc  vlen    u8vlen  vexists link                    propid                              flagid  idx                                 printable
    {"packet_type",             MR_BITS_DTYPE,      4,      MQTT_PUBLISH,       _NA,    4,      _NA,    true,   PUBLISH_MR_HEADER,      _NA,                                _NA,    PUBLISH_PACKET_TYPE,                NULL},
    {"publish_dup",             MR_BITS_DTYPE,      3,      0,                  _NA,    1,      _NA,    true,   PUBLISH_MR_HEADER,      _NA,                                _NA,    PUBLISH_DUP,                        NULL},
    {"qos",                     MR_BITS_DTYPE,      1,      0,                  _NA,    2,      _NA,    true,   PUBLISH_MR_HEADER,      _NA,                                _NA,    PUBLISH_QOS,                        NULL},
    {"retain",                  MR_BITS_DTYPE,      0,      0,                  _NA,    1,      _NA,    true,   PUBLISH_MR_HEADER,      _NA,                                _NA,    PUBLISH_RETAIN,                     NULL},
    {"mr_header",               MR_BITFLD_DTYPE,    _NA,    _MR_PUBLISH_HEADER, _NA,    1,      1,      true,   _NA,                    _NA,                                _NA,    PUBLISH_MR_HEADER,                  NULL},
    {"remaining_length",        MR_VBI_DTYPE,       _NA,    0,                  _NA,    0,      0,      true,   PUBLISH_PAYLOAD,        _NA,                                _NA,    PUBLISH_REMAINING_LENGTH,           NULL},
    {"topic_name",              MR_STR_DTYPE,       _NA,    (mr_mvalue_t)_S0L,  false,  1,      2,      true,   _NA,                    _NA,                                _NA,    PUBLISH_TOPIC_NAME,                 NULL},
    {"packet_identifier",       MR_U16_DTYPE,       _NA,    0,                  _NA,    2,      2,      false,  PUBLISH_QOS,            _NA,                                _NA,    PUBLISH_PACKET_IDENTIFIER,          NULL},
    {"property_length",         MR_VBI_DTYPE,       _NA,    0,                  _NA,    0,      0,      true,   PUBLISH_CONTENT_TYPE,   _NA,                                _NA,    PUBLISH_PROPERTY_LENGTH,            NULL},
    {"mr_properties",           MR_PROPERTIES_DTYPE,_NA,    (mr_mvalue_t)_PROPS,_NA,    _PSZ,   _NA,    true,   _NA,                    _NA,                                _NA,    PUBLISH_MR_PROPERTIES,              NULL},
    {"payload_format_indicator",MR_U8_DTYPE,        _NA,    0,                  _NA,    1,      2,      false,  _NA,                    MQTT_PROP_PAYLOAD_FORMAT_INDICATOR, _NA,    PUBLISH_PAYLOAD_FORMAT_INDICATOR,   NULL},
    {"message_expiry_interval", MR_U32_DTYPE,       _NA,    0,                  _NA,    4,      5,      false,  _NA,                    MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,  _NA,    PUBLISH_MESSAGE_EXPIRY_INTERVAL,    NULL},
    {"topic_alias",             MR_U16_DTYPE,       _NA,    0,                  _NA,    2,      3,      false,  _NA,                    MQTT_PROP_TOPIC_ALIAS,              _NA,    PUBLISH_TOPIC_ALIAS,                NULL},
    {"response_topic",          MR_STR_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                    MQTT_PROP_RESPONSE_TOPIC,           _NA,    PUBLISH_RESPONSE_TOPIC,             NULL},
    {"correlation_data",        MR_U8V_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                    MQTT_PROP_CORRELATION_DATA,         _NA,    PUBLISH_CORRELATION_DATA,           NULL},
    {"user_properties",         MR_SPV_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                    MQTT_PROP_USER_PROPERTY,            _NA,    PUBLISH_USER_PROPERTIES,            NULL},
    {"subscription_identifiers",MR_VBIV_DTYPE,      _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                    MQTT_PROP_SUBSCRIPTION_IDENTIFIER,  _NA,    PUBLISH_SUBSCRIPTION_IDENTIFIERS,   NULL},
    {"content_type",            MR_STR_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                    MQTT_PROP_CONTENT_TYPE,             _NA,    PUBLISH_CONTENT_TYPE,               NULL},
    {"payload",                 MR_PAYLOAD_DTYPE,   _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      true,   _NA,                    _NA,                                _NA,    PUBLISH_PAYLOAD,                    NULL},
//   name                       dtype               bitpos  value               valloc  vlen    u8vlen  vexists link                    propid                              flagid  idx                                 printable
};

static const size_t _PUBLISH_MDATA_COUNT = sizeof(_PUBLISH_MDATA_TEMPLATE) / sizeof(mr_mdata);

int mr_init_publish_packet(mr_packet_ctx **ppctx) {
    return mr_init_packet(ppctx, _PUBLISH_MDATA_TEMPLATE, _PUBLISH_MDATA_COUNT);
}

int mr_init_unpack_publish_packet(mr_packet_ctx **ppctx, const uint8_t *u8v0, const size_t u8vlen) {
    return mr_init_unpack_packet(ppctx, _PUBLISH_MDATA_TEMPLATE, _PUBLISH_MDATA_COUNT, u8v0, u8vlen);
}

static int mr_check_publish_packet(mr_packet_ctx *pctx) {
    if (pctx->mqtt_packet_type == MQTT_PUBLISH) {
        return 0;
    }
    else {
        dzlog_info("Packet Context is not a PUBLISH packet");
        return -1;
    }
}

int mr_pack_publish_packet(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *pu8vlen) {
    if (mr_check_publish_packet(pctx)) return -1;
    if (mr_validate_publish_pack(pctx)) return -1;
    return mr_pack_packet(pctx, pu8v0, pu8vlen);
}

int mr_free_publish_packet(mr_packet_ctx *pctx) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_free_packet_context(pctx);
}

// const uint8_t packet_type
int mr_get_publish_packet_type(mr_packet_ctx *pctx, uint8_t *pu8) {
    bool exists_flag;
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_u8(pctx, PUBLISH_PACKET_TYPE, pu8, &exists_flag);
}

// uint32_t *subscription_identifiers
int mr_get_publish_subscription_identifiers(mr_packet_ctx *pctx, uint32_t **pu32v0, size_t *plen, bool *pexists_flag) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_VBIv(pctx, PUBLISH_SUBSCRIPTION_IDENTIFIERS, pu32v0, plen, pexists_flag);
}

int mr_set_publish_subscription_identifiers(mr_packet_ctx *pctx, const uint32_t *u32v0, const size_t len) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_set_vector(pctx, PUBLISH_SUBSCRIPTION_IDENTIFIERS, u32v0, len);
}

int mr_reset_publish_subscription_identifiers(mr_packet_ctx *pctx) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_reset_vector(pctx, PUBLISH_SUBSCRIPTION_IDENTIFIERS);
}


// validation

static int mr_validate_publish_pack(mr_packet_ctx *pctx) {
    return 0;
}

// PUBLISH ptype_fn invoked from packet.c during unpack
int mr_validate_publish_unpack(mr_packet_ctx *pctx) {
/*
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    bool exists_flag;

    if (mr_get_publish_connect_reason_code(pctx, &u8)) return -1;
    if (mr_validate_publish_connect_reason_code(u8)) return -1;

    if (mr_get_publish_receive_maximum(pctx, &u16, &exists_flag)) return -1;
    if (exists_flag && mr_validate_publish_receive_maximum(u16)) return -1;

    if (mr_get_publish_maximum_qos(pctx, &u8, &exists_flag)) return -1;
    if (exists_flag && mr_validate_publish_maximum_qos(u8)) return -1;

    if (mr_get_publish_retain_available(pctx, &u8, &exists_flag)) return -1;
    if (exists_flag && mr_validate_publish_retain_available(u8)) return -1;

    if (mr_get_publish_maximum_packet_size(pctx, &u32, &exists_flag)) return -1;
    if (exists_flag && mr_validate_publish_maximum_packet_size(u32)) return -1;

    if (mr_get_publish_wildcard_subscription_available(pctx, &u8, &exists_flag)) return -1;
    if (exists_flag && mr_validate_publish_wildcard_subscription_available(u8)) return -1;

    if (mr_get_publish_subscription_identifiers_available(pctx, &u8, &exists_flag)) return -1;
    if (exists_flag && mr_validate_publish_subscription_identifiers_available(u8)) return -1;

    if (mr_get_publish_shared_subscription_available(pctx, &u8, &exists_flag)) return -1;
    if (exists_flag && mr_validate_publish_shared_subscription_available(u8)) return -1;
 */
    return 0;
}

int mr_get_publish_printable(mr_packet_ctx *pctx, const bool all_flag, char **pcv) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_printable(pctx, all_flag, pcv);
}
