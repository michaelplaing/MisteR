/* suback.c */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <zlog.h>

#include "mister_internal.h"

enum SUBACK_MDATA_FIELDS { // Same order as SUBACK_MDATA_TEMPLATE
    SUBACK_PACKET_TYPE,
    SUBACK_RESERVED_HEADER,
    SUBACK_MR_HEADER,
    SUBACK_REMAINING_LENGTH,
    SUBACK_PROPERTY_LENGTH,
    SUBACK_MR_PROPERTIES,
    SUBACK_REASON_STRING,
    SUBACK_USER_PROPERTIES,
    SUBACK_SUBSCRIBE_REASON_CODES
};

static const uint8_t VALID_SUBSCRIBE_REASON_CODES[] = {
    MQTT_RC_SUCCESS,
    MQTT_RC_GRANTED_QOS1,
    MQTT_RC_GRANTED_QOS2,
    MQTT_RC_UNSPECIFIED,
    MQTT_RC_IMPLEMENTATION_SPECIFIC,
    MQTT_RC_NOT_AUTHORIZED,
    MQTT_RC_TOPIC_FILTER_INVALID,
    MQTT_RC_PACKET_ID_IN_USE,
    MQTT_RC_QUOTA_EXCEEDED,
    MQTT_RC_SHARED_SUBSCRIPTIONS_NOT_SUPPORTED,
    MQTT_RC_SUBSCRIPTION_IDENTIFIERS_NOT_SUPPORTED,
    MQTT_RC_WILDCARD_SUBSCRIPTIONS_NOT_SUPPORTED,
};

static const size_t VSRCSZ = sizeof(VALID_SUBSCRIBE_REASON_CODES) / sizeof(VALID_SUBSCRIBE_REASON_CODES[0]);

static const uint8_t PROPS[] = {
    MQTT_PROP_REASON_STRING,
    MQTT_PROP_USER_PROPERTY,
};

static const size_t PSZ = sizeof(PROPS) / sizeof(PROPS[0]);

#define NA 0

static const uintptr_t MR_SUBACK_HEADER = MQTT_SUBACK << 4;

static const uint8_t MR_RCV[] = {MQTT_RC_SUCCESS};

static const size_t RCVSZ = 1;

static const mr_mdata SUBACK_MDATA_TEMPLATE[] = {
//   name                       dtype               value               valloc  vlen    u8vlen  vexists link                            propid                  flagid                  idx                             printable
    {"packet_type",             MR_BITS_DTYPE,      MQTT_SUBACK,        NA,     4,      4,      true,   SUBACK_MR_HEADER,               NA,                     NA,                     SUBACK_PACKET_TYPE,             NULL},
    {"reserved_header",         MR_BITS_DTYPE,      0,                  NA,     4,      0,      true,   SUBACK_MR_HEADER,               NA,                     NA,                     SUBACK_RESERVED_HEADER,         NULL},
    {"mr_header",               MR_BITFLD_DTYPE,    MR_SUBACK_HEADER,   NA,     1,      1,      true,   NA,                             NA,                     NA,                     SUBACK_MR_HEADER,               NULL},
    {"remaining_length",        MR_VBI_DTYPE,       0,                  NA,     0,      0,      true,   SUBACK_SUBSCRIBE_REASON_CODES,  NA,                     NA,                     SUBACK_REMAINING_LENGTH,        NULL},
    {"property_length",         MR_VBI_DTYPE,       0,                  NA,     0,      0,      true,   SUBACK_USER_PROPERTIES,         NA,                     NA,                     SUBACK_PROPERTY_LENGTH,         NULL},
    {"mr_properties",           MR_PROPERTIES_DTYPE,(uintptr_t)PROPS,   NA,     PSZ,    NA,     true,   NA,                             NA,                     NA,                     SUBACK_MR_PROPERTIES,           NULL},
    {"reason_string",           MR_STR_DTYPE,       (uintptr_t)NULL,    false,  0,      0,      false,  NA,                             MQTT_PROP_REASON_STRING,NA,                     SUBACK_REASON_STRING,           NULL},
    {"user_properties",         MR_SPV_DTYPE,       (uintptr_t)NULL,    false,  0,      0,      false,  NA,                             MQTT_PROP_USER_PROPERTY,NA,                     SUBACK_USER_PROPERTIES,         NULL},
    {"subscribe_reason_codes",  MR_PAYLOAD_DTYPE,   (uintptr_t)MR_RCV,  false,  RCVSZ,  RCVSZ,  true,   NA,                             NA,                     NA,                     SUBACK_SUBSCRIBE_REASON_CODES,  NULL},
//   name                       dtype               value               valloc  vlen    u8vlen  vexists link                            propid                  flagid                  idx                             printable
};

static const size_t SUBACK_MDATA_COUNT = sizeof(SUBACK_MDATA_TEMPLATE) / sizeof(SUBACK_MDATA_TEMPLATE[0]);

int mr_init_suback_packet(mr_packet_ctx **ppctx) {
    return mr_init_packet(ppctx, SUBACK_MDATA_TEMPLATE, SUBACK_MDATA_COUNT);
}

int mr_init_unpack_suback_packet(mr_packet_ctx **ppctx, const uint8_t *u8v0, const size_t u8vlen) {
    return mr_init_unpack_packet(ppctx, SUBACK_MDATA_TEMPLATE, SUBACK_MDATA_COUNT, u8v0, u8vlen);
}

static int mr_check_suback_packet(mr_packet_ctx *pctx) {
    if (pctx->mqtt_packet_type == MQTT_SUBACK) {
        return 0;
    }
    else {
        dzlog_info("Packet Context is not a SUBACK packet");
        return -1;
    }
}

int mr_pack_suback_packet(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *pu8vlen) {
    if (mr_check_suback_packet(pctx)) return -1;
    if (mr_validate_suback_pack(pctx)) return -1;
    return mr_pack_packet(pctx, pu8v0, pu8vlen);
}

int mr_free_suback_packet(mr_packet_ctx *pctx) {
    if (mr_check_suback_packet(pctx)) return -1;
    return mr_free_packet_context(pctx);
}

// const uint8_t packet_type
int mr_get_suback_packet_type(mr_packet_ctx *pctx, uint8_t *pu8) {
    bool exists_flag;
    if (mr_check_suback_packet(pctx)) return -1;
    return mr_get_u8(pctx, SUBACK_PACKET_TYPE, pu8, &exists_flag);
}

// const uint8_t reserved_header
int mr_get_suback_reserved_header(mr_packet_ctx *pctx, uint8_t *pu8) {
    bool exists_flag;
    if (mr_check_suback_packet(pctx)) return -1;
    return mr_get_u8(pctx, SUBACK_RESERVED_HEADER, pu8, &exists_flag);
}

// uint32_t remaining_length
int mr_get_suback_remaining_length(mr_packet_ctx *pctx, uint32_t *pu32) {
    bool exists_flag;
    if (mr_check_suback_packet(pctx)) return -1;
    return mr_get_u32(pctx, SUBACK_REMAINING_LENGTH, pu32, &exists_flag);
}

// uint32_t property_length
int mr_get_suback_property_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag) {
    if (mr_check_suback_packet(pctx)) return -1;
    return mr_get_u32(pctx, SUBACK_PROPERTY_LENGTH, pu32, pexists_flag);
}

// char *reason_string
int mr_get_suback_reason_string(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag) {
    if (mr_check_suback_packet(pctx)) return -1;
    return mr_get_str(pctx, SUBACK_REASON_STRING, pcv0, pexists_flag);
}

int mr_set_suback_reason_string(mr_packet_ctx *pctx, const char *cv0) {
    if (mr_check_suback_packet(pctx)) return -1;
    return mr_set_vector(pctx, SUBACK_REASON_STRING, cv0, strlen(cv0) + 1);
}

int mr_reset_suback_reason_string(mr_packet_ctx *pctx) {
    if (mr_check_suback_packet(pctx)) return -1;
    return mr_reset_vector(pctx, SUBACK_REASON_STRING);
}

// mr_string_pair *user_properties
int mr_get_suback_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists_flag) {
    if (mr_check_suback_packet(pctx)) return -1;
    return mr_get_spv(pctx, SUBACK_USER_PROPERTIES, pspv0, plen, pexists_flag);
}

int mr_set_suback_user_properties(mr_packet_ctx *pctx, const mr_string_pair *spv0, const size_t len) {
    if (mr_check_suback_packet(pctx)) return -1;
    return mr_set_vector(pctx, SUBACK_USER_PROPERTIES, spv0, len);
}

int mr_reset_suback_user_properties(mr_packet_ctx *pctx) {
    if (mr_check_suback_packet(pctx)) return -1;
    return mr_reset_vector(pctx, SUBACK_USER_PROPERTIES);
}

// uint8_t *subscribe_reason_codes
int mr_get_suback_subscribe_reason_codes(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists_flag) {
    if (mr_check_suback_packet(pctx)) return -1;
    return mr_get_u8v(pctx, SUBACK_SUBSCRIBE_REASON_CODES, pu8v0, plen, pexists_flag);
}

static int mr_validate_suback_subscribe_reason_codes(const uint8_t *u8v0, const size_t len) {
    uint8_t *pu8 = (uint8_t *)u8v0;
    for (int i = 0; i < len; i++, pu8++) {
        if (!memchr(VALID_SUBSCRIBE_REASON_CODES, *pu8, VSRCSZ)) {
            dzlog_error("invalid puback_reason_code: offset: %u; value: %u", i, *pu8);
            return -1;
        }
    }

    return 0;
}

int mr_set_suback_subscribe_reason_codes(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len) {
    if (mr_check_suback_packet(pctx)) return -1;
    if (mr_validate_suback_subscribe_reason_codes(u8v0, len)) return -1;
    return mr_set_vector(pctx, SUBACK_SUBSCRIBE_REASON_CODES, u8v0, len);
}


// validation

static int mr_validate_suback_cross(mr_packet_ctx *pctx) {
    uint8_t *u8v0;
    size_t len;
    bool exists_flag;

    if (mr_get_suback_subscribe_reason_codes(pctx, &u8v0, &len, &exists_flag)) return -1;

    if (!exists_flag || len < 1) {
        dzlog_error("subscribe_reason_codes must exist and be > 0");
        return -1;
    }

    return 0;
}

static int mr_validate_suback_pack(mr_packet_ctx *pctx) {
    if (mr_validate_suback_cross(pctx)) return -1;
    return 0;
}

// SUBACK ptype_fn invoked from packet.c during unpack
int mr_validate_suback_unpack(mr_packet_ctx *pctx) {
    uint8_t *u8v0;
    size_t len;
    bool exists_flag;

    if (mr_get_suback_subscribe_reason_codes(pctx, &u8v0, &len, &exists_flag)) return -1;
    if (mr_validate_suback_subscribe_reason_codes(u8v0, len)) return -1;

    if (mr_validate_suback_cross(pctx)) return -1;
    return 0;
}

int mr_get_suback_printable(mr_packet_ctx *pctx, const bool all_flag, char **pcv) {
    if (mr_check_suback_packet(pctx)) return -1;
    return mr_get_printable(pctx, all_flag, pcv);
}
