// connect.c

/**
 * @file
 * @brief The CONNECT packet field metadata (mr_mdata vector) and functions.
 *
 * This packet specifies the vector of mr_mdata instances that characterize the CONNECT
 * fields. It provides functions to get, set & reset their values as well as to validate.
 * There are additional functions for cross-validation.
*/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <zlog.h>

#include "mister_internal.h"

enum MR_CONNECT_MDATA_FIELDS { // Same order as _CONNECT_MDATA_TEMPLATE
    CONNECT_PACKET_TYPE,
    CONNECT_RESERVED_HEADER,
    CONNECT_MR_HEADER,
    CONNECT_REMAINING_LENGTH,
    CONNECT_PROTOCOL_NAME,
    CONNECT_PROTOCOL_VERSION,
    CONNECT_RESERVED,
    CONNECT_CLEAN_START,
    CONNECT_WILL_FLAG,
    CONNECT_WILL_QOS,
    CONNECT_WILL_RETAIN,
    CONNECT_PASSWORD_FLAG,
    CONNECT_USERNAME_FLAG,
    CONNECT_MR_FLAGS,
    CONNECT_KEEP_ALIVE,
    CONNECT_PROPERTY_LENGTH,
    CONNECT_MR_PROPERTIES,
    CONNECT_SESSION_EXPIRY_INTERVAL,
    CONNECT_RECEIVE_MAXIMUM,
    CONNECT_MAXIMUM_PACKET_SIZE,
    CONNECT_TOPIC_ALIAS_MAXIMUM,
    CONNECT_REQUEST_RESPONSE_INFORMATION,
    CONNECT_REQUEST_PROBLEM_INFORMATION,
    CONNECT_USER_PROPERTIES,
    CONNECT_AUTHENTICATION_METHOD,
    CONNECT_AUTHENTICATION_DATA,
    CONNECT_CLIENT_IDENTIFIER,
    CONNECT_WILL_PROPERTY_LENGTH,
    CONNECT_MR_WILL_PROPERTIES,
    CONNECT_WILL_DELAY_INTERVAL,
    CONNECT_PAYLOAD_FORMAT_INDICATOR,
    CONNECT_MESSAGE_EXPIRY_INTERVAL,
    CONNECT_CONTENT_TYPE,
    CONNECT_RESPONSE_TOPIC,
    CONNECT_CORRELATION_DATA,
    CONNECT_WILL_USER_PROPERTIES,
    CONNECT_WILL_TOPIC,
    CONNECT_WILL_PAYLOAD,
    CONNECT_USER_NAME,
    CONNECT_PASSWORD
};

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
static const mr_mvalue_t _MR_CONNECT_HEADER = MQTT_CONNECT << 4;

static const mr_mdata _CONNECT_MDATA_TEMPLATE[] = { // Same order as enum CONNECT_MDATA_FIELDS (see column idx)
//   name                           dtype               bitpos  value               valloc  vlen    u8vlen  vexists link                            propid                                  flagid                  idx                                     printable
    {"packet_type",                 MR_BITS_DTYPE,      4,      MQTT_CONNECT,       false,  4,      0,      true,   CONNECT_MR_HEADER,              _NA,                                    _NA,                    CONNECT_PACKET_TYPE,                    NULL},
    {"reserved_header",             MR_BITS_DTYPE,      0,      0,                  false,  4,      0,      true,   CONNECT_MR_HEADER,              _NA,                                    _NA,                    CONNECT_RESERVED_HEADER,                NULL},
    {"mr_header",                   MR_FLAGS_DTYPE,     _NA,    _MR_CONNECT_HEADER, false,  1,      1,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_MR_HEADER,                      NULL},
    {"remaining_length",            MR_VBI_DTYPE,       _NA,    0,                  false,  0,      0,      true,   CONNECT_PASSWORD,               _NA,                                    _NA,                    CONNECT_REMAINING_LENGTH,               NULL},
    {"protocol_name",               MR_U8V_DTYPE,       _NA,    _PS,   false,  _PSSZ,  _PSSZ+2,true,   _NA,                            _NA,                                    _NA,                    CONNECT_PROTOCOL_NAME,                  NULL},
    {"protocol_version",            MR_U8_DTYPE,        _NA,    _PV,                false,  1,      1,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_PROTOCOL_VERSION,               NULL},
    {"reserved",                    MR_BITS_DTYPE,      0,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_RESERVED,                       NULL},
    {"clean_start",                 MR_BITS_DTYPE,      1,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_CLEAN_START,                    NULL},
    {"will_flag",                   MR_BITS_DTYPE,      2,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_WILL_FLAG,                      NULL},
    {"will_qos",                    MR_BITS_DTYPE,      3,      0,                  false,  2,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_QOS,                       NULL},
    {"will_retain",                 MR_BITS_DTYPE,      5,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_RETAIN,                    NULL},
    {"password_flag",               MR_BITS_DTYPE,      6,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_PASSWORD_FLAG,                  NULL},
    {"username_flag",               MR_BITS_DTYPE,      7,      0,                  false,  1,      0,      true,   CONNECT_MR_FLAGS,               _NA,                                    _NA,                    CONNECT_USERNAME_FLAG,                  NULL},
    {"mr_flags",                    MR_FLAGS_DTYPE,     _NA,    0,                  false,  1,      1,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_MR_FLAGS,                       NULL},
    {"keep_alive",                  MR_U16_DTYPE,       _NA,    0,                  false,  2,      2,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_KEEP_ALIVE,                     NULL},
    {"property_length",             MR_VBI_DTYPE,       _NA,    0,                  false,  0,      0,      true,   CONNECT_AUTHENTICATION_DATA,    _NA,                                    _NA,                    CONNECT_PROPERTY_LENGTH,                NULL},
    {"mr_properties",               MR_PROPERTIES_DTYPE,_NA,    (mr_mvalue_t)_CP,   _NA,    _CPSZ,  _NA,    true,   _NA,                            _NA,                                    _NA,                    CONNECT_MR_PROPERTIES,                  NULL},
    {"session_expiry_interval",     MR_U32_DTYPE,       _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_SESSION_EXPIRY_INTERVAL,      _NA,                    CONNECT_SESSION_EXPIRY_INTERVAL,        NULL},
    {"receive_maximum",             MR_U16_DTYPE,       _NA,    0,                  false,  2,      3,      false,  _NA,                            MQTT_PROP_RECEIVE_MAXIMUM,              _NA,                    CONNECT_RECEIVE_MAXIMUM,                NULL},
    {"maximum_packet_size",         MR_U32_DTYPE,       _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_MAXIMUM_PACKET_SIZE,          _NA,                    CONNECT_MAXIMUM_PACKET_SIZE,            NULL},
    {"topic_alias_maximum",         MR_U16_DTYPE,       _NA,    0,                  false,  2,      3,      false,  _NA,                            MQTT_PROP_TOPIC_ALIAS_MAXIMUM,          _NA,                    CONNECT_TOPIC_ALIAS_MAXIMUM,            NULL},
    {"request_response_information",MR_U8_DTYPE,        _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_REQUEST_RESPONSE_INFORMATION, _NA,                    CONNECT_REQUEST_RESPONSE_INFORMATION,   NULL},
    {"request_problem_information", MR_U8_DTYPE,        _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_REQUEST_PROBLEM_INFORMATION,  _NA,                    CONNECT_REQUEST_PROBLEM_INFORMATION,    NULL},
    {"user_properties",             MR_SPV_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_USER_PROPERTY,                _NA,                    CONNECT_USER_PROPERTIES,                NULL},
    {"authentication_method",       MR_STR_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_AUTHENTICATION_METHOD,        _NA,                    CONNECT_AUTHENTICATION_METHOD,          NULL},
    {"authentication_data",         MR_U8V_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_AUTHENTICATION_DATA,          _NA,                    CONNECT_AUTHENTICATION_DATA,            NULL},
    {"client_identifier",           MR_STR_DTYPE,       _NA,    (mr_mvalue_t)_S0L,  false,  1,      2,      true,   _NA,                            _NA,                                    _NA,                    CONNECT_CLIENT_IDENTIFIER,              NULL},
    {"will_property_length",        MR_VBI_DTYPE,       _NA,    0,                  false,  0,      0,      false,  CONNECT_WILL_USER_PROPERTIES,   _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_PROPERTY_LENGTH,           NULL},
    {"mr_will_properties",          MR_PROPERTIES_DTYPE,_NA,    (mr_mvalue_t)_CWP,  _NA,    _CWPSZ, _NA,    _NA,    _NA,                            _NA,                                    _NA,                    CONNECT_MR_WILL_PROPERTIES,             NULL},
    {"will_delay_interval",         MR_U32_DTYPE,       _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_WILL_DELAY_INTERVAL,          CONNECT_WILL_FLAG,      CONNECT_WILL_DELAY_INTERVAL,            NULL},
    {"payload_format_indicator",    MR_U8_DTYPE,        _NA,    0,                  false,  1,      2,      false,  _NA,                            MQTT_PROP_PAYLOAD_FORMAT_INDICATOR,     CONNECT_WILL_FLAG,      CONNECT_PAYLOAD_FORMAT_INDICATOR,       NULL},
    {"message_expiry_interval",     MR_U32_DTYPE,       _NA,    0,                  false,  4,      5,      false,  _NA,                            MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,      CONNECT_WILL_FLAG,      CONNECT_MESSAGE_EXPIRY_INTERVAL,        NULL},
    {"content_type",                MR_STR_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_CONTENT_TYPE,                 CONNECT_WILL_FLAG,      CONNECT_CONTENT_TYPE,                   NULL},
    {"response_topic",              MR_STR_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_RESPONSE_TOPIC,               CONNECT_WILL_FLAG,      CONNECT_RESPONSE_TOPIC,                 NULL},
    {"correlation_data",            MR_U8V_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_CORRELATION_DATA,             CONNECT_WILL_FLAG,      CONNECT_CORRELATION_DATA,               NULL},
    {"will_user_properties",        MR_SPV_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            MQTT_PROP_USER_PROPERTY,                CONNECT_WILL_FLAG,      CONNECT_WILL_USER_PROPERTIES,           NULL},
    {"will_topic",                  MR_STR_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_TOPIC,                     NULL},
    {"will_payload",                MR_U8V_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            _NA,                                    CONNECT_WILL_FLAG,      CONNECT_WILL_PAYLOAD,                   NULL},
    {"user_name",                   MR_STR_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            _NA,                                    CONNECT_USERNAME_FLAG,  CONNECT_USER_NAME,                      NULL},
    {"password",                    MR_U8V_DTYPE,       _NA,    (mr_mvalue_t)NULL,  false,  0,      0,      false,  _NA,                            _NA,                                    CONNECT_PASSWORD_FLAG,  CONNECT_PASSWORD,                       NULL}
//   name                           dtype               bitpos  value               valloc  vlen    u8vlen  vexists link                            propid                                  flagid                  idx                                     printable
};

static const size_t _CONNECT_MDATA_COUNT = sizeof(_CONNECT_MDATA_TEMPLATE) / sizeof(mr_mdata);

/**
 * @brief Initialize a CONNECT packet for packing and set the address of the packet context.
 *
 * Set the initial packet values to NULL, false or 0 and copy in the default field values and metadata
 * from the packet's mr_mdata template vector. Use function to initialize a mr_packet_ctx (pctx) for use
 * in setting field values then then packing them into a valid network-order binary packet conforming to
 * the MQTT5 specification.
 */
int mr_init_connect_packet(mr_packet_ctx **ppctx) {
    return mr_init_packet(ppctx, _CONNECT_MDATA_TEMPLATE, _CONNECT_MDATA_COUNT);
}

/**
 * @brief Unpack and validate a binary CONNECT packet setting the address of the populated mr_packet_ctx.
 *
 * Initialize a mr_packet_ctx (pctx) then unpack the binary packet into it, overwriting and validating
 * values and metadata. Set the address of the packet context.
 */
int mr_init_unpack_connect_packet(mr_packet_ctx **ppctx, const uint8_t *u8v0, const size_t u8vlen) {
    return mr_init_unpack_packet(ppctx, _CONNECT_MDATA_TEMPLATE, _CONNECT_MDATA_COUNT, u8v0, u8vlen);
}

static int mr_check_connect_packet(mr_packet_ctx *pctx) {
    if (pctx->mqtt_packet_type == MQTT_CONNECT) {
        return 0;
    }
    else {
        dzlog_error("Packet Context is not a CONNECT packet");
        return -1;
    }
}

/**
 * @brief Pack a binary CONNECT packet and set its address.
 *
 * Validate the packet values and then pack them into a valid network-order binary packet conforming to the
 * MQTT5 specification.
 *
 * @note The returned binary packet is allocated as a member of the mr_packet_ctx (pctx) and will be freed when
 * the mr_packet_ctx is freed.
 */
int mr_pack_connect_packet(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *pu8vlen) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_validate_connect_pack(pctx)) return -1;
    return mr_pack_packet(pctx, pu8v0, pu8vlen);
}

/**
 * @brief Free a CONNECT packet context and its members.
 *
 * Free the mr_packet_ctx (pctx) and its members recursively. Always use this function to free the memory
 * associated with the context including the binary packet.
 */
int mr_free_connect_packet(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_free_packet_context(pctx);
}

// const uint8_t packet_type
int mr_get_connect_packet_type(mr_packet_ctx *pctx, uint8_t *pu8) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_PACKET_TYPE, pu8, &exists_flag);
}

// const uint8_t reserved_header
int mr_get_connect_reserved_header(mr_packet_ctx *pctx, uint8_t *pu8) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_RESERVED_HEADER, pu8, &exists_flag);
}

// uint32_t remaining_length
int mr_get_connect_remaining_length(mr_packet_ctx *pctx, uint32_t *pu32) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_REMAINING_LENGTH, pu32, &exists_flag);
}

// const uint8_t *protocol_name
int mr_get_connect_protocol_name(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_PROTOCOL_NAME, pu8v0, plen, &exists_flag);
}

// const uint8_t protocol_version
int mr_get_connect_protocol_version(mr_packet_ctx *pctx, uint8_t *pu8) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_PROTOCOL_VERSION, pu8, &exists_flag);
}

// const bool reserved
int mr_get_connect_reserved(mr_packet_ctx *pctx, bool *pflag_value) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_RESERVED, pflag_value, &exists_flag);
}

// bool clean_start
int mr_get_connect_clean_start(mr_packet_ctx *pctx, bool *pflag_value) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_CLEAN_START, pflag_value, &exists_flag);
}

int mr_set_connect_clean_start(mr_packet_ctx *pctx, const bool flag_value) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_CLEAN_START, flag_value);
}

// bool will_flag
int mr_get_connect_will_flag(mr_packet_ctx *pctx, bool *pflag_value) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_WILL_FLAG, pflag_value, &exists_flag);
}

int mr_set_connect_will_flag(mr_packet_ctx *pctx, const bool flag_value) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_scalar(pctx, CONNECT_WILL_FLAG, flag_value)) return -1;

    if (flag_value) {
        if (mr_set_scalar(pctx, CONNECT_WILL_PROPERTY_LENGTH, 0)) return -1; // set vexists
    }
    else { // set/reset will-related fields to original values
        mr_mdata *mdata = pctx->mdata0;
        for (int i = 0; i < _CONNECT_MDATA_COUNT; i++, mdata++) {
            if (mdata->flagid == CONNECT_WILL_FLAG) {
                if (mdata->dtype == MR_BITS_DTYPE) {
                    if (mr_set_scalar(pctx, mdata->idx, 0)) return -1;
                }
                else if (mdata->dtype == MR_U8V_DTYPE || mdata->dtype == MR_STR_DTYPE || mdata->dtype == MR_SPV_DTYPE) {
                    if (mr_reset_vector(pctx, mdata->idx)) return -1;
                }
                else { // a scalar dtype
                    if (mr_reset_scalar(pctx, mdata->idx)) return -1;
                }
            }
        }
    }

    return 0;
}

// uint8_t will_qos
int mr_get_connect_will_qos(mr_packet_ctx *pctx, uint8_t *pu8) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_WILL_QOS, pu8, &exists_flag);
}

static int mr_validate_connect_will_qos(const uint8_t u8) {
    if (u8 > 2) {
        dzlog_error("will_qos out of range (0..2): %u", u8);
        return -1;
    }

    return 0;
}

int mr_set_connect_will_qos(mr_packet_ctx *pctx, const uint8_t u8) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_validate_connect_will_qos(u8)) return -1;
    if (mr_set_connect_will_flag(pctx, true)) return -1;
    return mr_set_scalar(pctx, CONNECT_WILL_QOS, u8);
}

// bool will_retain
int mr_get_connect_will_retain(mr_packet_ctx *pctx, bool *pflag_value) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_WILL_RETAIN, pflag_value, &exists_flag);
}

int mr_set_connect_will_retain(mr_packet_ctx *pctx, const bool flag_value) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (flag_value && mr_set_connect_will_flag(pctx, true)) return -1;
    return mr_set_scalar(pctx, CONNECT_WILL_RETAIN, flag_value);
}

// bool password_flag - set by setting password
int mr_get_connect_password_flag(mr_packet_ctx *pctx, bool *pflag_value) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_PASSWORD_FLAG, pflag_value, &exists_flag);
}

// bool username_flag - set by setting username
int mr_get_connect_username_flag(mr_packet_ctx *pctx, bool *pflag_value) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_boolean(pctx, CONNECT_USERNAME_FLAG, pflag_value, &exists_flag);
}

// uint16_t keep_alive - always exists hence never reset
int mr_get_connect_keep_alive(mr_packet_ctx *pctx, uint16_t *pu16) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u16(pctx, CONNECT_KEEP_ALIVE, pu16, &exists_flag);
}

int mr_set_connect_keep_alive(mr_packet_ctx *pctx, const uint16_t u16) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_KEEP_ALIVE, u16);
}

// uint32_t property_length - calculated
int mr_get_connect_property_length(mr_packet_ctx *pctx, uint32_t *pu32) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_PROPERTY_LENGTH, pu32, &exists_flag);
}

// uint32_t session_expiry_interval
int mr_get_connect_session_expiry_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_SESSION_EXPIRY_INTERVAL, pu32, pexists_flag);
}

int mr_set_connect_session_expiry_interval(mr_packet_ctx *pctx, const uint32_t u32) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_SESSION_EXPIRY_INTERVAL, u32);
}

int mr_reset_connect_session_expiry_interval(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_SESSION_EXPIRY_INTERVAL);
}

// uint16_t receive_maximum
int mr_get_connect_receive_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u16(pctx, CONNECT_RECEIVE_MAXIMUM, pu16, pexists_flag);
}

static int mr_validate_connect_receive_maximum(const uint16_t u16) {
    if (u16 == 0) {
        dzlog_error("receive_maximum must be > 0");
        return -1;
    }

    return 0;
}

int mr_set_connect_receive_maximum(mr_packet_ctx *pctx, const uint16_t u16) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_validate_connect_receive_maximum(u16)) return -1;
    return mr_set_scalar(pctx, CONNECT_RECEIVE_MAXIMUM, u16);
}

int mr_reset_connect_receive_maximum(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_RECEIVE_MAXIMUM);
}

// uint32_t maximum_packet_size
int mr_get_connect_maximum_packet_size(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_MAXIMUM_PACKET_SIZE, pu32, pexists_flag);
}

static int mr_validate_connect_maximum_packet_size(const uint32_t u32) {
    if (u32 == 0) {
        dzlog_error("if present, maximum_packet_size must be > 0");
        return -1;
    }

    return 0;
}

int mr_set_connect_maximum_packet_size(mr_packet_ctx *pctx, const uint32_t u32) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_validate_connect_maximum_packet_size(u32)) return -1;
    return mr_set_scalar(pctx, CONNECT_MAXIMUM_PACKET_SIZE, u32);
}

int mr_reset_connect_maximum_packet_size(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_MAXIMUM_PACKET_SIZE);
}

// uint16_t topic_alias_maximum
int mr_get_connect_topic_alias_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u16(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM, pu16, pexists_flag);
}

int mr_set_connect_topic_alias_maximum(mr_packet_ctx *pctx, const uint16_t u16) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_set_scalar(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM, u16);
}

int mr_reset_connect_topic_alias_maximum(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_TOPIC_ALIAS_MAXIMUM);
}

// uint8_t request_response_information
int mr_get_connect_request_response_information(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION, pu8, pexists_flag);
}

static int mr_validate_connect_request_response_information(const uint8_t u8) {
    if (u8 > 1) {
        dzlog_error("request_response_information out of range (0..1): %u", u8);
        return -1;
    }

    return 0;
}

int mr_set_connect_request_response_information(mr_packet_ctx *pctx, const uint8_t u8) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_validate_connect_request_response_information(u8)) return -1;
    return mr_set_scalar(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION, u8);
}

int mr_reset_connect_request_response_information(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_REQUEST_RESPONSE_INFORMATION);
}

// uint8_t request_problem_information
int mr_get_connect_request_problem_information(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION, pu8, pexists_flag);
}

static int mr_validate_connect_request_problem_information(const uint8_t u8) {
    if (u8 > 1) {
        dzlog_error("request_problem_information out of range (0..1): %u", u8);
        return -1;
    }

    return 0;
}

int mr_set_connect_request_problem_information(mr_packet_ctx *pctx, const uint8_t u8) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_validate_connect_request_problem_information(u8)) return -1;
    return mr_set_scalar(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION, u8);
}

int mr_reset_connect_request_problem_information(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_REQUEST_PROBLEM_INFORMATION);
}

// mr_string_pair *user_properties
int mr_get_connect_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_spv(pctx, CONNECT_USER_PROPERTIES, pspv0, plen, pexists_flag);
}

int mr_set_connect_user_properties(mr_packet_ctx *pctx, const mr_string_pair *spv0, const size_t len) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_USER_PROPERTIES, spv0, len);
}

int mr_reset_connect_user_properties(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_USER_PROPERTIES);
}

// char *authentication_method
int mr_get_connect_authentication_method(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_str(pctx, CONNECT_AUTHENTICATION_METHOD, pcv0, pexists_flag);
}

int mr_set_connect_authentication_method(mr_packet_ctx *pctx, const char *cv0) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_AUTHENTICATION_METHOD, cv0, strlen(cv0) + 1);
}

int mr_reset_connect_authentication_method(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_AUTHENTICATION_METHOD);
}

// uint8_t *authentication_data
int mr_get_connect_authentication_data(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_AUTHENTICATION_DATA, pu8v0, plen, pexists_flag);
}

int mr_set_connect_authentication_data(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_AUTHENTICATION_DATA, u8v0, len);
}

int mr_reset_connect_authentication_data(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_AUTHENTICATION_DATA);
}

// char *client_identifier - always exists
int mr_get_connect_client_identifier(mr_packet_ctx *pctx, char **pcv0) {
    bool exists_flag;
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_str(pctx, CONNECT_CLIENT_IDENTIFIER, pcv0, &exists_flag);
}

int mr_set_connect_client_identifier(mr_packet_ctx *pctx, const char *cv0) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_set_vector(pctx, CONNECT_CLIENT_IDENTIFIER, cv0, strlen(cv0) + 1);
}

// uint32_t will_property_length - linked to will_flag and calculated if present
int mr_get_connect_will_property_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_WILL_PROPERTY_LENGTH, pu32, pexists_flag);
}

 // uint32_t will_delay_interval
int mr_get_connect_will_delay_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_WILL_DELAY_INTERVAL, pu32, pexists_flag);
}

int mr_set_connect_will_delay_interval(mr_packet_ctx *pctx, const uint32_t u32) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_connect_will_flag(pctx, true)) return -1;
    return mr_set_scalar(pctx, CONNECT_WILL_DELAY_INTERVAL, u32);
}

int mr_reset_connect_will_delay_interval(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_WILL_DELAY_INTERVAL);
}

// uint8_t payload_format_indicator
int mr_get_connect_payload_format_indicator(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u8(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, pu8, pexists_flag);
}

int mr_set_connect_payload_format_indicator(mr_packet_ctx *pctx, const uint8_t u8) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_connect_will_flag(pctx, true)) return -1;
    return mr_set_scalar(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, u8);
}

int mr_reset_connect_payload_format_indicator(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR);
}

// uint32_t message_expiry_interval
int mr_get_connect_message_expiry_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u32(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, pu32, pexists_flag);
}

int mr_set_connect_message_expiry_interval(mr_packet_ctx *pctx, const uint32_t u32) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_connect_will_flag(pctx, true)) return -1;
    return mr_set_scalar(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, u32);
}

int mr_reset_connect_message_expiry_interval(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_scalar(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL);
}

// char *content_type
int mr_get_connect_content_type(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_str(pctx, CONNECT_CONTENT_TYPE, pcv0, pexists_flag);
}

int mr_set_connect_content_type(mr_packet_ctx *pctx, const char *cv0) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_connect_will_flag(pctx, true)) return -1;
    return mr_set_vector(pctx, CONNECT_CONTENT_TYPE, cv0, strlen(cv0) + 1);
}

int mr_reset_connect_content_type(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_CONTENT_TYPE);
}

// char *response_topic
int mr_get_connect_response_topic(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_str(pctx, CONNECT_RESPONSE_TOPIC, pcv0, pexists_flag);
}

int mr_set_connect_response_topic(mr_packet_ctx *pctx, const char *cv0) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_connect_will_flag(pctx, true)) return -1;
    return mr_set_vector(pctx, CONNECT_RESPONSE_TOPIC, cv0, strlen(cv0) + 1);
}

int mr_reset_connect_response_topic(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_RESPONSE_TOPIC);
}

// uint8_t *correlation_data
int mr_get_connect_correlation_data(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_CORRELATION_DATA, pu8v0, plen, pexists_flag);
}

int mr_set_connect_correlation_data(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_connect_will_flag(pctx, true)) return -1;
    return mr_set_vector(pctx, CONNECT_CORRELATION_DATA, u8v0, len);
}

int mr_reset_connect_correlation_data(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_CORRELATION_DATA);
}

// mr_string_pair *will_user_properties
int mr_get_connect_will_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_spv(pctx, CONNECT_WILL_USER_PROPERTIES, pspv0, plen, pexists_flag);
}

int mr_set_connect_will_user_properties(mr_packet_ctx *pctx, const mr_string_pair *spv0, const size_t len) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_connect_will_flag(pctx, true)) return -1;
    return mr_set_vector(pctx, CONNECT_WILL_USER_PROPERTIES, spv0, len);
}

int mr_reset_connect_will_user_properties(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_WILL_USER_PROPERTIES);
}

// char *will_topic
int mr_get_connect_will_topic(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_str(pctx, CONNECT_WILL_TOPIC, pcv0, pexists_flag);
}

int mr_set_connect_will_topic(mr_packet_ctx *pctx, const char *cv0) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_connect_will_flag(pctx, true)) return -1;
    return mr_set_vector(pctx, CONNECT_WILL_TOPIC, cv0, strlen(cv0) + 1);
}

int mr_reset_connect_will_topic(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_WILL_TOPIC);
}

// uint8_t *will_payload
int mr_get_connect_will_payload(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_WILL_PAYLOAD, pu8v0, plen, pexists_flag);
}

int mr_set_connect_will_payload(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_connect_will_flag(pctx, true)) return -1;
    return mr_set_vector(pctx, CONNECT_WILL_PAYLOAD, u8v0, len);
}

int mr_reset_connect_will_payload(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_reset_vector(pctx, CONNECT_WILL_PAYLOAD);
}

// char *user_name
int mr_get_connect_user_name(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_str(pctx, CONNECT_USER_NAME, pcv0, pexists_flag);
}

int mr_set_connect_user_name(mr_packet_ctx *pctx, const char *cv0) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_scalar(pctx, CONNECT_USERNAME_FLAG, true)) return -1;
    return mr_set_vector(pctx, CONNECT_USER_NAME, cv0, strlen(cv0) + 1);
}

int mr_reset_connect_user_name(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_scalar(pctx, CONNECT_USERNAME_FLAG, false)) return -1;
    return mr_reset_vector(pctx, CONNECT_USER_NAME);
}

// uint8_t *password
int mr_get_connect_password(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists_flag) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_u8v(pctx, CONNECT_PASSWORD, pu8v0, plen, pexists_flag);
}

int mr_set_connect_password(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_scalar(pctx, CONNECT_PASSWORD_FLAG, true)) return -1;
    return mr_set_vector(pctx, CONNECT_PASSWORD, u8v0, len);
}

int mr_reset_connect_password(mr_packet_ctx *pctx) {
    if (mr_check_connect_packet(pctx)) return -1;
    if (mr_set_scalar(pctx, CONNECT_PASSWORD_FLAG, false)) return -1;
    return mr_reset_vector(pctx, CONNECT_PASSWORD);
}

// validation

static int mr_validate_connect_cross(mr_packet_ctx *pctx) {
    uint8_t u8;
    uint8_t *u8v0;
    size_t len;
    char *cv0;
    bool pexists_flag;

    // payload_format_indicator & will_payload
    if (mr_get_connect_payload_format_indicator(pctx, &u8, &pexists_flag)) return -1;

    if (pexists_flag) {
        if (u8 > 1) {
            dzlog_error("payload_format_indicator out of range (0..1): %u", u8);
            return -1;
        }
        else { // previous cross check of will_flag insures existence of will_payload
            if (u8 > 0 && mr_validate_u8v_utf8(pctx, CONNECT_WILL_PAYLOAD)) return -1;
        }
    }

    // authentication_method & authentication_data
    if (mr_get_connect_authentication_data(pctx, &u8v0, &len, &pexists_flag)) return -1;

    if (pexists_flag) {
        if (mr_get_connect_authentication_method(pctx, &cv0, &pexists_flag)) return -1;

        if (!pexists_flag) {
            dzlog_error("authentication_method must exist since authentication_data exists");
            return -1;
        }
    }

    return 0;
}

static int mr_validate_connect_pack(mr_packet_ctx *pctx) {
    bool pexists_flag;
    bool will_flag;
    if (mr_get_boolean(pctx, CONNECT_WILL_FLAG, &will_flag, &pexists_flag)) return -1;

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

    if (mr_validate_connect_cross(pctx)) return -1;
    return 0;
}

// CONNECT ptype_fn invoked from packet.c during unpack
int mr_validate_connect_unpack(mr_packet_ctx *pctx) {
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    bool exists_flag;

    if (mr_get_connect_will_qos(pctx, &u8)) return -1;
    if (mr_validate_connect_will_qos(u8)) return -1;

    if (mr_get_connect_receive_maximum(pctx, &u16, &exists_flag)) return -1;
    if (exists_flag && mr_validate_connect_receive_maximum(u16)) return -1;

    if (mr_get_connect_maximum_packet_size(pctx, &u32, &exists_flag)) return -1;
    if (exists_flag && mr_validate_connect_maximum_packet_size(u32)) return -1;

    if (mr_get_connect_request_response_information(pctx, &u8, &exists_flag)) return -1;
    if (exists_flag && mr_validate_connect_request_response_information(u8)) return -1;

    if (mr_get_connect_request_problem_information(pctx, &u8, &exists_flag)) return -1;
    if (exists_flag && mr_validate_connect_request_problem_information(u8)) return -1;

    if (mr_validate_connect_cross(pctx)) return -1;
    return 0;
}

/**
 * @brief Set a c-string to a printable version of the CONNECT packet's field values.
 *
 * See mr_get_printable() in packet.c.
 */
int mr_get_connect_printable(mr_packet_ctx *pctx, const bool all_flag, char **pcv) {
    if (mr_check_connect_packet(pctx)) return -1;
    return mr_get_printable(pctx, all_flag, pcv);
}
