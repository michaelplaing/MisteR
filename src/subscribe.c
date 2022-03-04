/* subscribe.c */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <zlog.h>

#include "mister_internal.h"

enum SUBSCRIBE_MDATA_FIELDS { // Same order as SUBSCRIBE_MDATA_TEMPLATE
    SUBSCRIBE_PACKET_TYPE,
    SUBSCRIBE_RESERVED_HEADER,
    SUBSCRIBE_MR_HEADER,
    SUBSCRIBE_REMAINING_LENGTH,
    SUBSCRIBE_PACKET_IDENTIFIER,
    SUBSCRIBE_PROPERTY_LENGTH,
    SUBSCRIBE_MR_PROPERTIES,
    SUBSCRIBE_SUBSCRIPTION_IDENTIFIER,
    SUBSCRIBE_USER_PROPERTIES,
    SUBSCRIBE_TOPIC_FILTERS
};

static const uint8_t PROPS[] = {
    MQTT_PROP_SUBSCRIPTION_IDENTIFIER,
    MQTT_PROP_USER_PROPERTY
};
static const size_t PSZ = sizeof(PROPS) / sizeof(PROPS[0]);

#define NA 0

static const uintptr_t MR_SUBSCRIBE_HEADER = MQTT_SUBSCRIBE << 4;

static const mr_mdata SUBSCRIBE_MDATA_TEMPLATE[] = {
//   name                       dtype               value               valloc  vlen    u8vlen  vexists link                        propid                              flagid  idx                                 printable
    {"packet_type",             MR_BITS_DTYPE,      MQTT_SUBSCRIBE,     NA,     4,      4,      true,   SUBSCRIBE_MR_HEADER,        NA,                                 NA,     SUBSCRIBE_PACKET_TYPE,              NULL},
    {"reserved_header",         MR_BITS_DTYPE,      2,                  NA,     4,      0,      true,   SUBSCRIBE_MR_HEADER,        NA,                                 NA,     SUBSCRIBE_RESERVED_HEADER,          NULL},
    {"mr_header",               MR_BITFLD_DTYPE,    MR_SUBSCRIBE_HEADER,NA,     1,      1,      true,   NA,                         NA,                                 NA,     SUBSCRIBE_MR_HEADER,                NULL},
    {"remaining_length",        MR_VBI_DTYPE,       0,                  NA,     0,      0,      true,   SUBSCRIBE_TOPIC_FILTERS,    NA,                                 NA,     SUBSCRIBE_REMAINING_LENGTH,         NULL},
    {"packet_identifier",       MR_U16_DTYPE,       0,                  NA,     2,      2,      true,   NA,                         NA,                                 NA,     SUBSCRIBE_PACKET_IDENTIFIER,        NULL},
    {"property_length",         MR_VBI_DTYPE,       0,                  NA,     0,      0,      true,   SUBSCRIBE_USER_PROPERTIES,  NA,                                 NA,     SUBSCRIBE_PROPERTY_LENGTH,          NULL},
    {"mr_properties",           MR_PROPERTIES_DTYPE,(uintptr_t)PROPS,   NA,     PSZ,    NA,     true,   NA,                         NA,                                 NA,     SUBSCRIBE_MR_PROPERTIES,            NULL},
    {"subscription_identifier", MR_VBI_DTYPE,       0,                  NA,     0,      0,      false,  NA,                         MQTT_PROP_SUBSCRIPTION_IDENTIFIER,  NA,     SUBSCRIBE_SUBSCRIPTION_IDENTIFIER,  NULL},
    {"user_properties",         MR_SPV_DTYPE,       (uintptr_t)NULL,    false,  0,      0,      false,  NA,                         MQTT_PROP_USER_PROPERTY,            NA,     SUBSCRIBE_USER_PROPERTIES,          NULL},
    {"topic_filters",           MR_TFV_DTYPE,       (uintptr_t)NULL,    false,  0,      0,      true,   NA,                         NA,                                 NA,     SUBSCRIBE_TOPIC_FILTERS,            NULL},
//   name                       dtype               value               valloc  vlen    u8vlen  vexists link                        propid                              flagid  idx                                 printable
};

static const size_t SUBSCRIBE_MDATA_COUNT = sizeof(SUBSCRIBE_MDATA_TEMPLATE) / sizeof(SUBSCRIBE_MDATA_TEMPLATE[0]);

int mr_init_subscribe_packet(mr_packet_ctx **ppctx) {
    return mr_init_packet(ppctx, SUBSCRIBE_MDATA_TEMPLATE, SUBSCRIBE_MDATA_COUNT);
}

int mr_init_unpack_subscribe_packet(mr_packet_ctx **ppctx, const uint8_t *u8v0, const size_t u8vlen) {
    return mr_init_unpack_packet(ppctx, SUBSCRIBE_MDATA_TEMPLATE, SUBSCRIBE_MDATA_COUNT, u8v0, u8vlen);
}

static int mr_check_subscribe_packet(mr_packet_ctx *pctx) {
    if (pctx->mqtt_packet_type == MQTT_SUBSCRIBE) {
        return 0;
    }
    else {
        dzlog_info("Packet Context is not a SUBSCRIBE packet");
        return -1;
    }
}

int mr_pack_subscribe_packet(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *pu8vlen) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    if (mr_validate_subscribe_pack(pctx)) return -1;
    return mr_pack_packet(pctx, pu8v0, pu8vlen);
}

int mr_free_subscribe_packet(mr_packet_ctx *pctx) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_free_packet_context(pctx);
}

// const uint8_t packet_type
int mr_get_subscribe_packet_type(mr_packet_ctx *pctx, uint8_t *pu8) {
    bool exists_flag;
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_get_u8(pctx, SUBSCRIBE_PACKET_TYPE, pu8, &exists_flag);
}

// const uint8_t reserved_header
int mr_get_subscribe_reserved_header(mr_packet_ctx *pctx, uint8_t *pu8) {
    bool exists_flag;
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_get_u8(pctx, SUBSCRIBE_RESERVED_HEADER, pu8, &exists_flag);
}

// uint32_t remaining_length
int mr_get_subscribe_remaining_length(mr_packet_ctx *pctx, uint32_t *pu32) {
    bool exists_flag;
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_get_u32(pctx, SUBSCRIBE_REMAINING_LENGTH, pu32, &exists_flag);
}

// uint16_t packet_identifier
int mr_get_subscribe_packet_identifier(mr_packet_ctx *pctx, uint16_t *pu16) {
    bool exists_flag;
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_get_u16(pctx, SUBSCRIBE_PACKET_IDENTIFIER, pu16, &exists_flag);
}

int mr_set_subscribe_packet_identifier(mr_packet_ctx *pctx, const uint16_t u16) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_set_scalar(pctx, SUBSCRIBE_PACKET_IDENTIFIER, u16);
}

// uint8_t subscribe_reason_code
int mr_get_subscribe_subscribe_reason_code(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_get_u8(pctx, SUBSCRIBE_SUBSCRIBE_REASON_CODE, pu8, pexists_flag);
}

static int mr_validate_subscribe_subscribe_reason_code(const uint8_t u8) {
    if (!memchr(SUBSCRIBE_SUBSCRIBE_REASON_CODES, u8, CCRCSZ)) {
        dzlog_error("invalid subscribe_reason_code: %u", u8);
        return -1;
    }

    return 0;
}

int mr_set_subscribe_subscribe_reason_code(mr_packet_ctx *pctx, const uint8_t u8) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    if (mr_validate_subscribe_subscribe_reason_code(u8)) return -1;
    return mr_set_scalar(pctx, SUBSCRIBE_SUBSCRIBE_REASON_CODE, u8);
}

int mr_reset_subscribe_subscribe_reason_code(mr_packet_ctx *pctx) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, SUBSCRIBE_SUBSCRIBE_REASON_CODE);
}

// uint32_t property_length
int mr_get_subscribe_property_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_get_u32(pctx, SUBSCRIBE_PROPERTY_LENGTH, pu32, pexists_flag);
}

// char *reason_string
int mr_get_subscribe_reason_string(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_get_str(pctx, SUBSCRIBE_REASON_STRING, pcv0, pexists_flag);
}

int mr_set_subscribe_reason_string(mr_packet_ctx *pctx, const char *cv0) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_set_vector(pctx, SUBSCRIBE_REASON_STRING, cv0, strlen(cv0) + 1);
}

int mr_reset_subscribe_reason_string(mr_packet_ctx *pctx) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_reset_vector(pctx, SUBSCRIBE_REASON_STRING);
}

// mr_string_pair *user_properties
int mr_get_subscribe_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists_flag) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_get_spv(pctx, SUBSCRIBE_USER_PROPERTIES, pspv0, plen, pexists_flag);
}

int mr_set_subscribe_user_properties(mr_packet_ctx *pctx, const mr_string_pair *spv0, const size_t len) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_set_vector(pctx, SUBSCRIBE_USER_PROPERTIES, spv0, len);
}

int mr_reset_subscribe_user_properties(mr_packet_ctx *pctx) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    return mr_reset_vector(pctx, SUBSCRIBE_USER_PROPERTIES);
}

// validation

static int mr_validate_subscribe_cross(mr_packet_ctx *pctx) {
    char *cv0;
    mr_string_pair *spv0;
    uint8_t u8;
    size_t len;
    bool reason_string_exists_flag;
    bool user_properties_exists_flag;

    if (mr_get_subscribe_reason_string(pctx, &cv0, &reason_string_exists_flag)) return -1;
    if (mr_get_subscribe_user_properties(pctx, &spv0, &len, &user_properties_exists_flag)) return -1;

    if (reason_string_exists_flag || user_properties_exists_flag) {
        if (mr_set_scalar(pctx, SUBSCRIBE_PROPERTY_LENGTH, 0)) return -1; // set vexists
    }
    else { // reset vexists as needed
        if (mr_reset_scalar(pctx, SUBSCRIBE_PROPERTY_LENGTH)) return -1; // reset vexists
        bool exists_flag;
        if (mr_get_subscribe_subscribe_reason_code(pctx, &u8, &exists_flag)) return -1;
        if (exists_flag && u8 == 0 && mr_reset_subscribe_subscribe_reason_code(pctx)) return -1;
    }

    return 0;
}

static int mr_validate_subscribe_pack(mr_packet_ctx *pctx) {
    if (mr_validate_subscribe_cross(pctx)) return -1;
    return 0;
}

// SUBSCRIBE ptype_fn invoked from packet.c during unpack
int mr_validate_subscribe_unpack(mr_packet_ctx *pctx) {
    uint8_t u8;
    bool exists_flag;

    if (mr_get_subscribe_subscribe_reason_code(pctx, &u8, &exists_flag)) return -1;
    if (exists_flag && mr_validate_subscribe_subscribe_reason_code(u8)) return -1;

    return 0;
}

int mr_get_subscribe_printable(mr_packet_ctx *pctx, const bool all_flag, char **pcv) {
    if (mr_check_subscribe_packet(pctx)) return -1;
    if (mr_validate_subscribe_cross(pctx)) return -1; // (re)set vexists
    return mr_get_printable(pctx, all_flag, pcv);
}
