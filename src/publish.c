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
//   name                       dtype               value               valloc  vlen    u8vlen  vexists link                    propid                              flagid      idx                                 printable
    {"packet_type",             MR_BITS_DTYPE,      MQTT_PUBLISH,       _NA,    4,      4,      true,   PUBLISH_MR_HEADER,      _NA,                                _NA,        PUBLISH_PACKET_TYPE,                NULL},
    {"dup",                     MR_BITS_DTYPE,      0,                  _NA,    1,      3,      true,   PUBLISH_MR_HEADER,      _NA,                                _NA,        PUBLISH_DUP,                        NULL},
    {"qos",                     MR_BITS_DTYPE,      0,                  _NA,    2,      1,      true,   PUBLISH_MR_HEADER,      _NA,                                _NA,        PUBLISH_QOS,                        NULL},
    {"retain",                  MR_BITS_DTYPE,      0,                  _NA,    1,      0,      true,   PUBLISH_MR_HEADER,      _NA,                                _NA,        PUBLISH_RETAIN,                     NULL},
    {"mr_header",               MR_BITFLD_DTYPE,    _MR_PUBLISH_HEADER, _NA,    1,      1,      true,   _NA,                    _NA,                                _NA,        PUBLISH_MR_HEADER,                  NULL},
    {"remaining_length",        MR_VBI_DTYPE,       0,                  _NA,    0,      0,      true,   PUBLISH_PAYLOAD,        _NA,                                _NA,        PUBLISH_REMAINING_LENGTH,           NULL},
    {"topic_name",              MR_STR_DTYPE,       (mr_mvalue_t)_S0L,  false,  1,      2,      true,   _NA,                    _NA,                                _NA,        PUBLISH_TOPIC_NAME,                 NULL},
    {"packet_identifier",       MR_U16_DTYPE,       0,                  _NA,    2,      2,      false,  _NA,                    _NA,                                PUBLISH_QOS,PUBLISH_PACKET_IDENTIFIER,          NULL},
    {"property_length",         MR_VBI_DTYPE,       0,                  _NA,    0,      0,      true,   PUBLISH_CONTENT_TYPE,   _NA,                                _NA,        PUBLISH_PROPERTY_LENGTH,            NULL},
    {"mr_properties",           MR_PROPERTIES_DTYPE,(mr_mvalue_t)_PROPS,_NA,    _PSZ,   _NA,    true,   _NA,                    _NA,                                _NA,        PUBLISH_MR_PROPERTIES,              NULL},
    {"payload_format_indicator",MR_U8_DTYPE,        0,                  _NA,    1,      2,      false,  _NA,                    MQTT_PROP_PAYLOAD_FORMAT_INDICATOR, _NA,        PUBLISH_PAYLOAD_FORMAT_INDICATOR,   NULL},
    {"message_expiry_interval", MR_U32_DTYPE,       0,                  _NA,    4,      5,      false,  _NA,                    MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,  _NA,        PUBLISH_MESSAGE_EXPIRY_INTERVAL,    NULL},
    {"topic_alias",             MR_U16_DTYPE,       0,                  _NA,    2,      3,      false,  _NA,                    MQTT_PROP_TOPIC_ALIAS,              _NA,        PUBLISH_TOPIC_ALIAS,                NULL},
    {"response_topic",          MR_STR_DTYPE,       (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                    MQTT_PROP_RESPONSE_TOPIC,           _NA,        PUBLISH_RESPONSE_TOPIC,             NULL},
    {"correlation_data",        MR_U8V_DTYPE,       (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                    MQTT_PROP_CORRELATION_DATA,         _NA,        PUBLISH_CORRELATION_DATA,           NULL},
    {"user_properties",         MR_SPV_DTYPE,       (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                    MQTT_PROP_USER_PROPERTY,            _NA,        PUBLISH_USER_PROPERTIES,            NULL},
    {"subscription_identifiers",MR_VBIV_DTYPE,      (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                    MQTT_PROP_SUBSCRIPTION_IDENTIFIER,  _NA,        PUBLISH_SUBSCRIPTION_IDENTIFIERS,   NULL},
    {"content_type",            MR_STR_DTYPE,       (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                    MQTT_PROP_CONTENT_TYPE,             _NA,        PUBLISH_CONTENT_TYPE,               NULL},
    {"payload",                 MR_PAYLOAD_DTYPE,   (mr_mvalue_t)NULL,  false,  0,      0,      true,   _NA,                    _NA,                                _NA,        PUBLISH_PAYLOAD,                    NULL},
//   name                       dtype               value               valloc  vlen    u8vlen  vexists link                    propid                              flagid      idx                                 printable
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

// bool dup
int mr_get_publish_dup(mr_packet_ctx *pctx, bool *pflag_value) {
    bool exists_flag;
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_boolean(pctx, PUBLISH_DUP, pflag_value, &exists_flag);
}

int mr_set_publish_dup(mr_packet_ctx *pctx, const bool flag_value) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_set_scalar(pctx, PUBLISH_DUP, flag_value);
}

// uint8_t qos
int mr_get_publish_qos(mr_packet_ctx *pctx, uint8_t *pu8) {
    bool exists_flag;
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_u8(pctx, PUBLISH_QOS, pu8, &exists_flag);
}

static int mr_validate_publish_qos(const uint8_t u8) {
    if (u8 > 2) {
        dzlog_error("qos must be in range (0..2): %u", u8);
        return -1;
    }

    return 0;
}

int mr_set_publish_qos(mr_packet_ctx *pctx, const uint8_t u8) {
    if (mr_check_publish_packet(pctx)) return -1;
    if (mr_validate_publish_qos(u8)) return -1;
    return mr_set_scalar(pctx, PUBLISH_QOS, u8);
}

// bool retain
int mr_get_publish_retain(mr_packet_ctx *pctx, bool *pflag_value) {
    bool exists_flag;
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_boolean(pctx, PUBLISH_RETAIN, pflag_value, &exists_flag);
}

int mr_set_publish_retain(mr_packet_ctx *pctx, const bool flag_value) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_set_scalar(pctx, PUBLISH_RETAIN, flag_value);
}

// uint32_t remaining_length
int mr_get_publish_remaining_length(mr_packet_ctx *pctx, uint32_t *pu32) {
    bool exists_flag;
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_u32(pctx, PUBLISH_REMAINING_LENGTH, pu32, &exists_flag);
}

// char *topic_name
int mr_get_publish_topic_name(mr_packet_ctx *pctx, char **pcv0) {
    bool exists_flag;
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_str(pctx, PUBLISH_TOPIC_NAME, pcv0, &exists_flag);
}

static int mr_validate_publish_topic_name(const char *cv0) {
    if (mr_wildcard_found(cv0)) {
        dzlog_error("topic_name must not contain wildcard characters");
        return -1;
    }

    return 0;
}

int mr_set_publish_topic_name(mr_packet_ctx *pctx, const char *cv0) {
    if (mr_check_publish_packet(pctx)) return -1;
    if (mr_validate_publish_topic_name(cv0)) return -1;
    return mr_set_vector(pctx, PUBLISH_TOPIC_NAME, cv0, strlen(cv0) + 1);
}

// uint16_t packet_identifier
int mr_get_publish_packet_identifier(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists_flag) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_u16(pctx, PUBLISH_PACKET_IDENTIFIER, pu16, pexists_flag);
}

static int mr_validate_publish_packet_identifier(const uint16_t u16) {
    if (u16 == 0) {
        dzlog_error("packet_identifier must be > 0");
        return -1;
    }

    return 0;
}

int mr_set_publish_packet_identifier(mr_packet_ctx *pctx, const uint16_t u16) {
    if (mr_check_publish_packet(pctx)) return -1;
    if (mr_validate_publish_packet_identifier(u16)) return -1;
    return mr_set_scalar(pctx, PUBLISH_PACKET_IDENTIFIER, u16);
}

int mr_reset_publish_packet_identifier(mr_packet_ctx *pctx) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, PUBLISH_PACKET_IDENTIFIER);
}

// uint32_t property_length
int mr_get_publish_property_length(mr_packet_ctx *pctx, uint32_t *pu32) {
    bool exists_flag;
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_u32(pctx, PUBLISH_PROPERTY_LENGTH, pu32, &exists_flag);
}

// uint8_t payload_format_indicator
int mr_get_publish_payload_format_indicator(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_u8(pctx, PUBLISH_PAYLOAD_FORMAT_INDICATOR, pu8, pexists_flag);
}

static int mr_validate_publish_payload_format_indicator(const uint8_t u8) {
    if (u8 > 1) {
        dzlog_error("payload_format_indicator out of range (0..1): %u", u8);
        return -1;
    }

    return 0;
}

int mr_set_publish_payload_format_indicator(mr_packet_ctx *pctx, const uint8_t u8) {
    if (mr_check_publish_packet(pctx)) return -1;
    if (mr_validate_publish_payload_format_indicator(u8)) return -1;
    return mr_set_scalar(pctx, PUBLISH_PAYLOAD_FORMAT_INDICATOR, u8);
}

int mr_reset_publish_payload_format_indicator(mr_packet_ctx *pctx) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, PUBLISH_PAYLOAD_FORMAT_INDICATOR);
}

// uint32_t message_expiry_interval
int mr_get_publish_message_expiry_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_u32(pctx, PUBLISH_MESSAGE_EXPIRY_INTERVAL, pu32, pexists_flag);
}

int mr_set_publish_message_expiry_interval(mr_packet_ctx *pctx, const uint32_t u32) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_set_scalar(pctx, PUBLISH_MESSAGE_EXPIRY_INTERVAL, u32);
}

int mr_reset_publish_message_expiry_interval(mr_packet_ctx *pctx) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, PUBLISH_MESSAGE_EXPIRY_INTERVAL);
}

// uint16_t topic_alias
int mr_get_publish_topic_alias(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists_flag) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_u16(pctx, PUBLISH_PACKET_IDENTIFIER, pu16, pexists_flag);
}

static int mr_validate_publish_topic_alias(const uint16_t u16) {
    if (u16 == 0) {
        dzlog_error("topic_alias must be > 0");
        return -1;
    }

    return 0;
}

int mr_set_publish_topic_alias(mr_packet_ctx *pctx, const uint16_t u16) {
    if (mr_check_publish_packet(pctx)) return -1;
    if (mr_validate_publish_topic_alias(u16)) return -1;
    return mr_set_scalar(pctx, PUBLISH_PACKET_IDENTIFIER, u16);
}

int mr_reset_publish_topic_alias(mr_packet_ctx *pctx) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, PUBLISH_PACKET_IDENTIFIER);
}

// char *response_topic
int mr_get_publish_response_topic(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_str(pctx, PUBLISH_RESPONSE_TOPIC, pcv0, pexists_flag);
}

static int mr_validate_publish_response_topic(const char *cv0) {
    if (mr_wildcard_found(cv0)) {
        dzlog_error("response_topic must not contain wildcard characters");
        return -1;
    }

    return 0;
}

int mr_set_publish_response_topic(mr_packet_ctx *pctx, const char *cv0) {
    if (mr_check_publish_packet(pctx)) return -1;
    if (mr_validate_publish_response_topic(cv0)) return -1;
    return mr_set_vector(pctx, PUBLISH_RESPONSE_TOPIC, cv0, strlen(cv0) + 1);
}

int mr_reset_publish_response_topic(mr_packet_ctx *pctx) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_reset_vector(pctx, PUBLISH_RESPONSE_TOPIC);
}

// uint8_t *correlation_data
int mr_get_publish_correlation_data(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists_flag) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_u8v(pctx, PUBLISH_CORRELATION_DATA, pu8v0, plen, pexists_flag);
}

int mr_set_publish_correlation_data(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_set_vector(pctx, PUBLISH_CORRELATION_DATA, u8v0, len);
}

int mr_reset_publish_correlation_data(mr_packet_ctx *pctx) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_reset_vector(pctx, PUBLISH_CORRELATION_DATA);
}

// mr_string_pair *user_properties
int mr_get_publish_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists_flag) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_spv(pctx, PUBLISH_USER_PROPERTIES, pspv0, plen, pexists_flag);
}

int mr_set_publish_user_properties(mr_packet_ctx *pctx, const mr_string_pair *spv0, const size_t len) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_set_vector(pctx, PUBLISH_USER_PROPERTIES, spv0, len);
}

int mr_reset_publish_user_properties(mr_packet_ctx *pctx) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_reset_vector(pctx, PUBLISH_USER_PROPERTIES);
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

// char *content_type
int mr_get_publish_content_type(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_str(pctx, PUBLISH_CONTENT_TYPE, pcv0, pexists_flag);
}

int mr_set_publish_content_type(mr_packet_ctx *pctx, const char *cv0) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_set_vector(pctx, PUBLISH_CONTENT_TYPE, cv0, strlen(cv0) + 1);
}

int mr_reset_publish_content_type(mr_packet_ctx *pctx) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_reset_vector(pctx, PUBLISH_CONTENT_TYPE);
}

// uint8_t *payload
int mr_get_publish_payload(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    bool exists_flag;
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_u8v(pctx, PUBLISH_PAYLOAD, pu8v0, plen, &exists_flag);
}

int mr_set_publish_payload(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_set_vector(pctx, PUBLISH_PAYLOAD, u8v0, len);
}

// validation

static int mr_validate_publish_cross(mr_packet_ctx *pctx) {
    uint8_t u8;
    uint16_t u16;
    bool exists_flag;

    // qos & packet_identifier
    if (mr_get_publish_qos(pctx, &u8)) return -1;

    if (u8) {
        if (mr_get_publish_packet_identifier(pctx, &u16, &exists_flag)) return -1;

        if (!exists_flag || u16 == 0) {
            dzlog_error("qos > 0 but packet_identifier does not exist or equals 0");
            return -1;
        }
    }

    // payload_format_indicator & payload
    if (mr_get_publish_payload_format_indicator(pctx, &u8, &exists_flag)) return -1;
    if (exists_flag && u8 && mr_validate_u8v_utf8(pctx, PUBLISH_PAYLOAD)) return -1;

    return 0;
}

static int mr_validate_publish_pack(mr_packet_ctx *pctx) {
    if (mr_validate_publish_cross(pctx)) return -1;
    return 0;
}

// PUBLISH ptype_fn invoked from packet.c during unpack
int mr_validate_publish_unpack(mr_packet_ctx *pctx) {
    uint8_t u8;
    uint16_t u16;
    char *cv0;
    bool exists_flag;

    if (mr_get_publish_qos(pctx, &u8)) return -1;
    if (mr_validate_publish_qos(u8)) return -1;

    if (mr_get_publish_topic_name(pctx, &cv0)) return -1;
    if (mr_validate_publish_topic_name(cv0)) return -1;

    if (mr_get_publish_packet_identifier(pctx, &u16, &exists_flag)) return -1;
    if (exists_flag && mr_validate_publish_packet_identifier(u16)) return -1;

    if (mr_get_publish_payload_format_indicator(pctx, &u8, &exists_flag)) return -1;
    if (exists_flag && mr_validate_publish_payload_format_indicator(u8)) return -1;

    if (mr_get_publish_topic_alias(pctx, &u16, &exists_flag)) return -1;
    if (exists_flag && mr_validate_publish_topic_alias(u16)) return -1;

    if (mr_get_publish_response_topic(pctx, &cv0, &exists_flag)) return -1;
    if (exists_flag && mr_validate_publish_response_topic(cv0)) return -1;

    if (mr_validate_publish_cross(pctx)) return -1;

    return 0;
}

int mr_get_publish_printable(mr_packet_ctx *pctx, const bool all_flag, char **pcv) {
    if (mr_check_publish_packet(pctx)) return -1;
    return mr_get_printable(pctx, all_flag, pcv);
}
