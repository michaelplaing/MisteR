/* puback.c */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <zlog.h>

#include "mister_internal.h"

enum PUBACK_MDATA_FIELDS { // Same order as _PUBACK_MDATA_TEMPLATE
    PUBACK_PACKET_TYPE,
    PUBACK_RESERVED_HEADER,
    PUBACK_MR_HEADER,
    PUBACK_REMAINING_LENGTH,
    PUBACK_PACKET_IDENTIFIER,
    PUBACK_PUBACK_REASON_CODE,
    PUBACK_PROPERTY_LENGTH,
    PUBACK_MR_PROPERTIES,
    PUBACK_REASON_STRING,
    PUBACK_USER_PROPERTIES
};

static const uint8_t _PUBACK_PUBACK_REASON_CODES[] = {
    MQTT_RC_SUCCESS,
    MQTT_RC_NO_MATCHING_SUBSCRIBERS,
    MQTT_RC_UNSPECIFIED,
    MQTT_RC_IMPLEMENTATION_SPECIFIC,
    MQTT_RC_NOT_AUTHORIZED,
    MQTT_RC_TOPIC_NAME_INVALID,
    MQTT_RC_PACKET_ID_IN_USE,
    MQTT_RC_QUOTA_EXCEEDED,
    MQTT_RC_PAYLOAD_FORMAT_INVALID
};
static const size_t _CCRCSZ = sizeof(_PUBACK_PUBACK_REASON_CODES) / sizeof(_PUBACK_PUBACK_REASON_CODES[0]);

static const uint8_t _PROPS[] = {
    MQTT_PROP_REASON_STRING,
    MQTT_PROP_USER_PROPERTY,
};
static const size_t _PSZ = sizeof(_PROPS) / sizeof(_PROPS[0]);

#define _NA 0

static const char _S0L[] = "";
static const mr_mvalue_t _MR_PUBACK_HEADER = MQTT_PUBACK << 4;

static const mr_mdata _PUBACK_MDATA_TEMPLATE[] = {
//   name                   dtype               value               valloc  vlen    u8vlen  vexists link                    propid                  flagid                  idx                         printable
    {"packet_type",         MR_BITS_DTYPE,      MQTT_PUBACK,        _NA,    4,      4,      true,   PUBACK_MR_HEADER,       _NA,                    _NA,                    PUBACK_PACKET_TYPE,         NULL},
    {"reserved_header",     MR_BITS_DTYPE,      0,                  _NA,    4,      0,      true,   PUBACK_MR_HEADER,       _NA,                    _NA,                    PUBACK_RESERVED_HEADER,     NULL},
    {"mr_header",           MR_BITFLD_DTYPE,    _MR_PUBACK_HEADER,  _NA,    1,      1,      true,   _NA,                    _NA,                    _NA,                    PUBACK_MR_HEADER,           NULL},
    {"remaining_length",    MR_VBI_DTYPE,       0,                  _NA,    0,      0,      true,   PUBACK_USER_PROPERTIES, _NA,                    _NA,                    PUBACK_REMAINING_LENGTH,    NULL},
    {"packet_identifier",   MR_U16_DTYPE,       0,                  _NA,    2,      2,      true,   _NA,                    _NA,                    _NA,                    PUBACK_PACKET_IDENTIFIER,   NULL},
    {"puback_reason_code",  MR_U8_DTYPE,        0,                  _NA,    1,      1,      false,  _NA,                    _NA,                    PUBACK_REMAINING_LENGTH,PUBACK_PUBACK_REASON_CODE,  NULL},
    {"property_length",     MR_VBI_DTYPE,       0,                  _NA,    0,      0,      false,  PUBACK_USER_PROPERTIES, _NA,                    _NA,                    PUBACK_PROPERTY_LENGTH,     NULL},
    {"mr_properties",       MR_PROPERTIES_DTYPE,(mr_mvalue_t)_PROPS,_NA,    _PSZ,   _NA,    true,   _NA,                    _NA,                    PUBACK_REMAINING_LENGTH,PUBACK_MR_PROPERTIES,       NULL},
    {"reason_string",       MR_STR_DTYPE,       (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                    MQTT_PROP_REASON_STRING,_NA,                    PUBACK_REASON_STRING,       NULL},
    {"user_properties",     MR_SPV_DTYPE,       (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                    MQTT_PROP_USER_PROPERTY,_NA,                    PUBACK_USER_PROPERTIES,     NULL},
//   name                   dtype               value               valloc  vlen    u8vlen  vexists link                    propid                  flagid                  idx                         printable
};
static const size_t _PUBACK_MDATA_COUNT = sizeof(_PUBACK_MDATA_TEMPLATE) / sizeof(_PUBACK_MDATA_TEMPLATE[0]);

int mr_init_puback_packet(mr_packet_ctx **ppctx) {
    return mr_init_packet(ppctx, _PUBACK_MDATA_TEMPLATE, _PUBACK_MDATA_COUNT);
}

int mr_init_unpack_puback_packet(mr_packet_ctx **ppctx, const uint8_t *u8v0, const size_t u8vlen) {
    return mr_init_unpack_packet(ppctx, _PUBACK_MDATA_TEMPLATE, _PUBACK_MDATA_COUNT, u8v0, u8vlen);
}

static int mr_check_puback_packet(mr_packet_ctx *pctx) {
    if (pctx->mqtt_packet_type == MQTT_PUBACK) {
        return 0;
    }
    else {
        dzlog_info("Packet Context is not a PUBACK packet");
        return -1;
    }
}

int mr_pack_puback_packet(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *pu8vlen) {
    if (mr_check_puback_packet(pctx)) return -1;
    if (mr_validate_puback_pack(pctx)) return -1;
    return mr_pack_packet(pctx, pu8v0, pu8vlen);
}

int mr_free_puback_packet(mr_packet_ctx *pctx) {
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_free_packet_context(pctx);
}

// const uint8_t packet_type
int mr_get_puback_packet_type(mr_packet_ctx *pctx, uint8_t *pu8) {
    bool exists_flag;
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_get_u8(pctx, PUBACK_PACKET_TYPE, pu8, &exists_flag);
}

// const uint8_t reserved_header
int mr_get_puback_reserved_header(mr_packet_ctx *pctx, uint8_t *pu8) {
    bool exists_flag;
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_get_u8(pctx, PUBACK_RESERVED_HEADER, pu8, &exists_flag);
}

// uint32_t remaining_length
int mr_get_puback_remaining_length(mr_packet_ctx *pctx, uint32_t *pu32) {
    bool exists_flag;
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_get_u32(pctx, PUBACK_REMAINING_LENGTH, pu32, &exists_flag);
}

// uint16_t packet_identifier
int mr_get_puback_packet_identifier(mr_packet_ctx *pctx, uint16_t *pu16) {
    bool exists_flag;
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_get_u16(pctx, PUBACK_PACKET_IDENTIFIER, pu16, &exists_flag);
}

int mr_set_puback_packet_identifier(mr_packet_ctx *pctx, const uint16_t u16) {
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_set_scalar(pctx, PUBACK_PACKET_IDENTIFIER, u16);
}

// uint8_t puback_reason_code
int mr_get_puback_puback_reason_code(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag) {
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_get_u8(pctx, PUBACK_PUBACK_REASON_CODE, pu8, pexists_flag);
}

static int mr_validate_puback_puback_reason_code(const uint8_t u8) {
    if (!memchr(_PUBACK_PUBACK_REASON_CODES, u8, _CCRCSZ)) {
        dzlog_error("invalid puback_reason_code: %u", u8);
        return -1;
    }

    return 0;
}

int mr_set_puback_puback_reason_code(mr_packet_ctx *pctx, const uint8_t u8) {
    if (mr_check_puback_packet(pctx)) return -1;
    if (mr_validate_puback_puback_reason_code(u8)) return -1;
    return mr_set_scalar(pctx, PUBACK_PUBACK_REASON_CODE, u8);
}

int mr_reset_puback_puback_reason_code(mr_packet_ctx *pctx) {
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, PUBACK_PUBACK_REASON_CODE);
}

// uint32_t property_length
int mr_get_puback_property_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag) {
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_get_u32(pctx, PUBACK_PROPERTY_LENGTH, pu32, pexists_flag);
}

// char *reason_string
int mr_get_puback_reason_string(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag) {
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_get_str(pctx, PUBACK_REASON_STRING, pcv0, pexists_flag);
}

int mr_set_puback_reason_string(mr_packet_ctx *pctx, const char *cv0) {
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_set_vector(pctx, PUBACK_REASON_STRING, cv0, strlen(cv0) + 1);
}

int mr_reset_puback_reason_string(mr_packet_ctx *pctx) {
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_reset_vector(pctx, PUBACK_REASON_STRING);
}

// mr_string_pair *user_properties
int mr_get_puback_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists_flag) {
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_get_spv(pctx, PUBACK_USER_PROPERTIES, pspv0, plen, pexists_flag);
}

int mr_set_puback_user_properties(mr_packet_ctx *pctx, const mr_string_pair *spv0, const size_t len) {
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_set_vector(pctx, PUBACK_USER_PROPERTIES, spv0, len);
}

int mr_reset_puback_user_properties(mr_packet_ctx *pctx) {
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_reset_vector(pctx, PUBACK_USER_PROPERTIES);
}

// validation

static int mr_validate_puback_cross(mr_packet_ctx *pctx) {
    char *cv0;
    mr_string_pair *spv0;
    uint8_t u8;
    size_t len;
    bool reason_string_exists_flag;
    bool user_properties_exists_flag;

    if (mr_get_puback_reason_string(pctx, &cv0, &reason_string_exists_flag)) return -1;
    if (mr_get_puback_user_properties(pctx, &spv0, &len, &user_properties_exists_flag)) return -1;

    if (!reason_string_exists_flag && !user_properties_exists_flag) {
        bool exists_flag;
        if (mr_get_puback_puback_reason_code(pctx, &u8, &exists_flag)) return -1;
        if (exists_flag && u8 == 0 && mr_reset_puback_puback_reason_code(pctx)) return -1;
    }

    return 0;
}

static int mr_validate_puback_pack(mr_packet_ctx *pctx) {
    if (mr_validate_puback_cross(pctx)) return -1;
    return 0;
}

// PUBACK ptype_fn invoked from packet.c during unpack
int mr_validate_puback_unpack(mr_packet_ctx *pctx) {
    uint8_t u8;
    bool exists_flag;

    if (mr_get_puback_puback_reason_code(pctx, &u8, &exists_flag)) return -1;
    if (exists_flag && mr_validate_puback_puback_reason_code(u8)) return -1;

    return 0;
}

int mr_get_puback_printable(mr_packet_ctx *pctx, const bool all_flag, char **pcv) {
    if (mr_check_puback_packet(pctx)) return -1;
    return mr_get_printable(pctx, all_flag, pcv);
}
