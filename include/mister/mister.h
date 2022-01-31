// mister.h

#ifndef MISTER_H
#define MISTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// spec
enum mqtt_packet_type {
    MQTT_CONNECT        = 0x10U,
    MQTT_CONNACK        = 0x20U,
    MQTT_PUBLISH        = 0x30U,
    MQTT_PUBACK         = 0x40U,
    MQTT_PUBREC         = 0x50U,
    MQTT_PUBREL         = 0x60U,
    MQTT_PUBCOMP        = 0x70U,
    MQTT_SUBSCRIBE      = 0x80U,
    MQTT_SUBACK         = 0x90U,
    MQTT_UNSUBSCRIBE    = 0xA0U,
    MQTT_UNSUBACK       = 0xB0U,
    MQTT_PINGREQ        = 0xC0U,
    MQTT_PINGRESP       = 0xD0U,
    MQTT_DISCONNECT     = 0xE0U,
    MQTT_AUTH           = 0xF0U
};

// from mosquitto & spec
enum mqtt_property {
    MQTT_PROP_PAYLOAD_FORMAT_INDICATOR = 1,             /* Byte :               PUBLISH, Will Properties */
    MQTT_PROP_MESSAGE_EXPIRY_INTERVAL = 2,              /* 4 byte int :         PUBLISH, Will Properties */
    MQTT_PROP_CONTENT_TYPE = 3,                         /* UTF-8 string :       PUBLISH, Will Properties */
    MQTT_PROP_RESPONSE_TOPIC = 8,                       /* UTF-8 string :       PUBLISH, Will Properties */
    MQTT_PROP_CORRELATION_DATA = 9,                     /* Binary Data :        PUBLISH, Will Properties */
    MQTT_PROP_SUBSCRIPTION_IDENTIFIER = 11,             /* Variable byte int :  PUBLISH, SUBSCRIBE */
    MQTT_PROP_SESSION_EXPIRY_INTERVAL = 17,             /* 4 byte int :         CONNECT, CONNACK, DISCONNECT */
    MQTT_PROP_ASSIGNED_CLIENT_IDENTIFIER = 18,          /* UTF-8 string :       CONNACK */
    MQTT_PROP_SERVER_KEEP_ALIVE = 19,                   /* 2 byte int :         CONNACK */
    MQTT_PROP_AUTHENTICATION_METHOD = 21,               /* UTF-8 string :       CONNECT, CONNACK, AUTH */
    MQTT_PROP_AUTHENTICATION_DATA = 22,                 /* Binary Data :        CONNECT, CONNACK, AUTH */
    MQTT_PROP_REQUEST_PROBLEM_INFORMATION = 23,         /* Byte :               CONNECT */
    MQTT_PROP_WILL_DELAY_INTERVAL = 24,                 /* 4 byte int :         Will properties */
    MQTT_PROP_REQUEST_RESPONSE_INFORMATION = 25,        /* Byte :               CONNECT */
    MQTT_PROP_RESPONSE_INFORMATION = 26,                /* UTF-8 string :       CONNACK */
    MQTT_PROP_SERVER_REFERENCE = 28,                    /* UTF-8 string :       CONNACK, DISCONNECT */
    MQTT_PROP_REASON_STRING = 31,                       /* UTF-8 string :       All except Will properties */
    MQTT_PROP_RECEIVE_MAXIMUM = 33,                     /* 2 byte int :         CONNECT, CONNACK */
    MQTT_PROP_TOPIC_ALIAS_MAXIMUM = 34,                 /* 2 byte int :         CONNECT, CONNACK */
    MQTT_PROP_TOPIC_ALIAS = 35,                         /* 2 byte int :         PUBLISH */
    MQTT_PROP_MAXIMUM_QOS = 36,                         /* Byte :               CONNACK */
    MQTT_PROP_RETAIN_AVAILABLE = 37,                    /* Byte :               CONNACK */
    MQTT_PROP_USER_PROPERTY = 38,                       /* UTF-8 string pair :  All */
    MQTT_PROP_MAXIMUM_PACKET_SIZE = 39,                 /* 4 byte int :         CONNECT, CONNACK */
    MQTT_PROP_WILDCARD_SUBSCRIPTION_AVAILABLE = 40,     /* Byte :               CONNACK */
    MQTT_PROP_SUBSCRIPTION_IDENTIFIERS_AVAILABLE = 41,  /* Byte :               CONNACK */
    MQTT_PROP_SHARED_SUBSCRIPTION_AVAILABLE = 42,       /* Byte :               CONNACK */
};

// spec & mosquitto
enum mqtt_reason_codes {
    MQTT_RC_SUCCESS = 0,                                    /* CONNACK, PUBACK, PUBREC, PUBREL, PUBCOMP, UNSUBACK, AUTH */
    MQTT_RC_NORMAL_DISCONNECTION = 0,                       /* DISCONNECT */
    MQTT_RC_GRANTED_QOS0 = 0,                               /* SUBACK */
    MQTT_RC_GRANTED_QOS1 = 1,                               /* SUBACK */
    MQTT_RC_GRANTED_QOS2 = 2,                               /* SUBACK */
    MQTT_RC_DISCONNECT_WITH_WILL_MSG = 4,                   /* DISCONNECT */
    MQTT_RC_NO_MATCHING_SUBSCRIBERS = 16,                   /* PUBACK, PUBREC */
    MQTT_RC_NO_SUBSCRIPTION_EXISTED = 17,                   /* UNSUBACK */
    MQTT_RC_CONTINUE_AUTHENTICATION = 24,                   /* AUTH */
    MQTT_RC_REAUTHENTICATE = 25,                            /* AUTH */
    MQTT_RC_UNSPECIFIED = 128,                              /* CONNACK, PUBACK, PUBREC, SUBACK, UNSUBACK, DISCONNECT */
    MQTT_RC_MALFORMED_PACKET = 129,		                    /* CONNACK, DISCONNECT */
    MQTT_RC_PROTOCOL_ERROR = 130,		                    /* DISCONNECT */
    MQTT_RC_IMPLEMENTATION_SPECIFIC = 131,                  /* CONNACK, PUBACK, PUBREC, SUBACK, UNSUBACK, DISCONNECT */
    MQTT_RC_UNSUPPORTED_PROTOCOL_VERSION = 132,             /* CONNACK */
    MQTT_RC_CLIENTID_NOT_VALID = 133,                       /* CONNACK */
    MQTT_RC_BAD_USERNAME_OR_PASSWORD = 134,                 /* CONNACK */
    MQTT_RC_NOT_AUTHORIZED = 135,                           /* CONNACK, PUBACK, PUBREC, SUBACK, UNSUBACK, DISCONNECT */
    MQTT_RC_SERVER_UNAVAILABLE = 136,                       /* CONNACK */
    MQTT_RC_SERVER_BUSY = 137,                              /* CONNACK, DISCONNECT */
    MQTT_RC_BANNED = 138,                                   /* CONNACK */
    MQTT_RC_SERVER_SHUTTING_DOWN = 139,                     /* DISCONNECT */
    MQTT_RC_BAD_AUTHENTICATION_METHOD = 140,                /* CONNACK */
    MQTT_RC_KEEP_ALIVE_TIMEOUT = 141,                       /* DISCONNECT */
    MQTT_RC_SESSION_TAKEN_OVER = 142,                       /* DISCONNECT */
    MQTT_RC_TOPIC_FILTER_INVALID = 143,                     /* SUBACK, UNSUBACK, DISCONNECT */
    MQTT_RC_TOPIC_NAME_INVALID = 144,                       /* CONNACK, PUBACK, PUBREC, DISCONNECT */
    MQTT_RC_PACKET_ID_IN_USE = 145,                         /* PUBACK, SUBACK, UNSUBACK */
    MQTT_RC_PACKET_ID_NOT_FOUND = 146,                      /* PUBREL, PUBCOMP */
    MQTT_RC_RECEIVE_MAXIMUM_EXCEEDED = 147,                 /* DISCONNECT */
    MQTT_RC_TOPIC_ALIAS_INVALID = 148,                      /* DISCONNECT */
    MQTT_RC_PACKET_TOO_LARGE = 149,                         /* CONNACK, PUBACK, PUBREC, DISCONNECT */
    MQTT_RC_MESSAGE_RATE_TOO_HIGH = 150,                    /* DISCONNECT */
    MQTT_RC_QUOTA_EXCEEDED = 151,                           /* PUBACK, PUBREC, SUBACK, DISCONNECT */
    MQTT_RC_ADMINISTRATIVE_ACTION = 152,                    /* DISCONNECT */
    MQTT_RC_PAYLOAD_FORMAT_INVALID = 153,                   /* CONNACK, DISCONNECT */
    MQTT_RC_RETAIN_NOT_SUPPORTED = 154,                     /* CONNACK, DISCONNECT */
    MQTT_RC_QOS_NOT_SUPPORTED = 155,                        /* CONNACK, DISCONNECT */
    MQTT_RC_USE_ANOTHER_SERVER = 156,                       /* CONNACK, DISCONNECT */
    MQTT_RC_SERVER_MOVED = 157,                             /* CONNACK, DISCONNECT */
    MQTT_RC_SHARED_SUBSCRIPTIONS_NOT_SUPPORTED = 158,       /* SUBACK, DISCONNECT */
    MQTT_RC_CONNECTION_RATE_EXCEEDED = 159,                 /* CONNACK, DISCONNECT */
    MQTT_RC_MAXIMUM_CONNECT_TIME = 160,                     /* DISCONNECT */
    MQTT_RC_SUBSCRIPTION_IDENTIFIERS_NOT_SUPPORTED = 161,   /* SUBACK, DISCONNECT */
    MQTT_RC_WILDCARD_SUBSCRIPTIONS_NOT_SUPPORTED = 162,     /* SUBACK, DISCONNECT */
};

// MisteR Commands understood by the Redis mister module
#define MR_CONNECT          "mr.connect"
#define MR_CONNACK          "mr.connack"
#define MR_PUBLISH          "mr.publish"
#define MR_PUBACK           "mr.puback"
#define MR_PUBREC           "mr.pubrec"
#define MR_PUBREL           "mr.pubrel"
#define MR_PUBCOMP          "mr.pubcomp"
#define MR_SUBSCRIBE        "mr.subscribe"
#define MR_SUBACK           "mr.suback"
#define MR_UNSUBSCRIBE      "mr.unsubscribe"
#define MR_UNSUBACK         "mr.unsuback"
#define MR_PINGREQ          "mr.pingreq"
#define MR_PINGRESP         "mr.pingresp"
#define MR_DISCONNECT       "mr.disconnect"
#define MR_AUTH             "mr.auth"

// error handling

extern int mr_errno;

// memory

int mr_calloc(void **ppv, size_t count, size_t sz);
int mr_malloc(void **ppv, size_t sz);
int mr_realloc(void **ppv, size_t sz);
int mr_free(void *pv);

// packets

typedef struct mr_packet_ctx {
    uint8_t mqtt_packet_type;
    char *mqtt_packet_name;
    char *mdata_dump;
    uint8_t *u8v0;
    bool u8valloc;
    size_t u8vlen;
    size_t u8vpos;
    struct mr_mdata *mdata0;
    size_t mdata_count;
} mr_packet_ctx;

typedef struct mr_string_pair {
    char *name;
    char *value;
} mr_string_pair;

// packet utilities

int mr_print_hexdump(uint8_t *u8v, const size_t u8vlen);
int mr_get_hexdump(char *cv0, const size_t cvlen, const uint8_t *u8v, const size_t u8vlen);
void mr_compress_spaces_lines(char *cv);

// connect packet

int mr_init_connect_pctx(mr_packet_ctx **ppctx);
int mr_init_unpack_connect_packet(mr_packet_ctx **ppctx, uint8_t *u8v0, size_t ulen);

static int mr_connect_packet_check(mr_packet_ctx *pctx);
int mr_pack_connect_packet(mr_packet_ctx *pctx);
int mr_free_connect_pctx(mr_packet_ctx *pctx);
int mr_connect_mdata_dump(mr_packet_ctx *pctx);
int mr_validate_connect_values(mr_packet_ctx *pctx);

int mr_get_connect_packet_type(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_get_connect_remaining_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_get_connect_protocol_name(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists);
int mr_get_connect_protocol_version(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_get_connect_reserved(mr_packet_ctx *pctx, bool *pboolean, bool *pexists);

int mr_get_connect_clean_start(mr_packet_ctx *pctx, bool *pboolean, bool *pexists);
int mr_set_connect_clean_start(mr_packet_ctx *pctx, bool boolean);

int mr_get_connect_will_flag(mr_packet_ctx *pctx, bool *pboolean, bool *pexists);
int mr_set_connect_will_flag(mr_packet_ctx *pctx, bool boolean);

int mr_get_connect_will_qos(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connect_will_qos(mr_packet_ctx *pctx, uint8_t u8);

int mr_get_connect_will_retain(mr_packet_ctx *pctx, bool *pboolean, bool *pexists);
int mr_set_connect_will_retain(mr_packet_ctx *pctx, bool boolean);

int mr_get_connect_password_flag(mr_packet_ctx *pctx, bool *pboolean, bool *pexists);

int mr_get_connect_username_flag(mr_packet_ctx *pctx, bool *pboolean, bool *pexists);

int mr_get_connect_keep_alive(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists);
int mr_set_connect_keep_alive(mr_packet_ctx *pctx, uint16_t u16);

int mr_get_connect_property_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists);

int mr_get_connect_session_expiry_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_set_connect_session_expiry_interval(mr_packet_ctx *pctx, uint32_t u32);
int mr_reset_connect_session_expiry_interval(mr_packet_ctx *pctx);

int mr_get_connect_receive_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists);
int mr_set_connect_receive_maximum(mr_packet_ctx *pctx, uint16_t u16);
int mr_reset_connect_receive_maximum(mr_packet_ctx *pctx);

int mr_get_connect_maximum_packet_size(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_set_connect_maximum_packet_size(mr_packet_ctx *pctx, uint32_t u32);
int mr_reset_connect_maximum_packet_size(mr_packet_ctx *pctx);

int mr_get_connect_topic_alias_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists);
int mr_set_connect_topic_alias_maximum(mr_packet_ctx *pctx, uint16_t u16);
int mr_reset_connect_topic_alias_maximum(mr_packet_ctx *pctx);

int mr_get_connect_request_response_information(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connect_request_response_information(mr_packet_ctx *pctx, uint8_t u8);
int mr_reset_connect_request_response_information(mr_packet_ctx *pctx);

int mr_get_connect_request_problem_information(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connect_request_problem_information(mr_packet_ctx *pctx, uint8_t u8);
int mr_reset_connect_request_problem_information(mr_packet_ctx *pctx);

int mr_get_connect_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists);
int mr_set_connect_user_properties(mr_packet_ctx *pctx, mr_string_pair *spv0, size_t len);
int mr_reset_connect_user_properties(mr_packet_ctx *pctx);

int mr_get_connect_authentication_method(mr_packet_ctx *pctx, char **cv0, bool *pexists);
int mr_set_connect_authentication_method(mr_packet_ctx *pctx, char *cv0);
int mr_reset_connect_authentication_method(mr_packet_ctx *pctx);

int mr_get_connect_authentication_data(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists);
int mr_set_connect_authentication_data(mr_packet_ctx *pctx, uint8_t *u8v0, size_t len);
int mr_reset_connect_authentication_data(mr_packet_ctx *pctx);

int mr_get_connect_client_identifier(mr_packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connect_client_identifier(mr_packet_ctx *pctx, char *cv0);

int mr_get_connect_will_property_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_set_connect_will_property_length(mr_packet_ctx *pctx, uint32_t u32);
int mr_reset_connect_will_property_length(mr_packet_ctx *pctx);

int mr_get_connect_will_delay_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_set_connect_will_delay_interval(mr_packet_ctx *pctx, uint32_t u32);
int mr_reset_connect_will_delay_interval(mr_packet_ctx *pctx);

int mr_get_connect_payload_format_indicator(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connect_payload_format_indicator(mr_packet_ctx *pctx, uint8_t u8);
int mr_reset_connect_payload_format_indicator(mr_packet_ctx *pctx);

int mr_get_connect_message_expiry_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_set_connect_message_expiry_interval(mr_packet_ctx *pctx, uint32_t u32);
int mr_reset_connect_message_expiry_interval(mr_packet_ctx *pctx);

int mr_get_connect_content_type(mr_packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connect_content_type(mr_packet_ctx *pctx, char *cv0);
int mr_reset_connect_content_type(mr_packet_ctx *pctx);

int mr_get_connect_response_topic(mr_packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connect_response_topic(mr_packet_ctx *pctx, char *cv0);
int mr_reset_connect_response_topic(mr_packet_ctx *pctx);

int mr_get_connect_correlation_data(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists);
int mr_set_connect_correlation_data(mr_packet_ctx *pctx, uint8_t *u8v0, size_t len);
int mr_reset_connect_correlation_data(mr_packet_ctx *pctx);

int mr_get_connect_will_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists);
int mr_set_connect_will_user_properties(mr_packet_ctx *pctx, mr_string_pair *spv0, size_t len);
int mr_reset_connect_will_user_properties(mr_packet_ctx *pctx);

int mr_get_connect_will_topic(mr_packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connect_will_topic(mr_packet_ctx *pctx, char *cv0);
int mr_reset_connect_will_topic(mr_packet_ctx *pctx);

int mr_get_connect_will_payload(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists);
int mr_set_connect_will_payload(mr_packet_ctx *pctx, uint8_t *u8v0, size_t len);
int mr_reset_connect_will_payload(mr_packet_ctx *pctx);

int mr_get_connect_user_name(mr_packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connect_user_name(mr_packet_ctx *pctx, char *cv0);
int mr_reset_connect_user_name(mr_packet_ctx *pctx);

int mr_get_connect_password(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists);
int mr_set_connect_password(mr_packet_ctx *pctx, uint8_t *u8v0, size_t len);
int mr_reset_connect_password(mr_packet_ctx *pctx);

// connack packet

int mr_init_connack_packet(mr_packet_ctx **ppctx);
int mr_init_unpack_connack_packet(mr_packet_ctx **ppctx, uint8_t *u8v0, size_t ulen);
static int mr_connack_packet_check(mr_packet_ctx *pctx);
int mr_pack_connack_packet(mr_packet_ctx *pctx);
int mr_free_connack_packet(mr_packet_ctx *pctx);
int mr_connack_mdata_dump(mr_packet_ctx *pctx);

int mr_get_connack_packet_type(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_get_connack_remaining_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists);

int mr_get_connack_session_present(mr_packet_ctx *pctx, bool *pboolean, bool *pexists);
int mr_set_connack_session_present(mr_packet_ctx *pctx, bool boolean);
int mr_reset_connack_session_present(mr_packet_ctx *pctx);

int mr_get_connack_reserved(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);

int mr_get_connack_connect_reason_code(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connack_connect_reason_code(mr_packet_ctx *pctx, uint8_t u8);
int mr_reset_connack_connect_reason_code(mr_packet_ctx *pctx);

int mr_get_connack_property_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists);

int mr_get_connack_session_expiry_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_set_connack_session_expiry_interval(mr_packet_ctx *pctx, uint32_t u32);
int mr_reset_connack_session_expiry_interval(mr_packet_ctx *pctx);

int mr_get_connack_receive_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists);
int mr_set_connack_receive_maximum(mr_packet_ctx *pctx, uint16_t u16);
int mr_reset_connack_receive_maximum(mr_packet_ctx *pctx);

int mr_get_connack_maximum_qos(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connack_maximum_qos(mr_packet_ctx *pctx, uint8_t u8);
int mr_reset_connack_maximum_qos(mr_packet_ctx *pctx);

int mr_get_connack_retain_available(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connack_retain_available(mr_packet_ctx *pctx, uint8_t u8);
int mr_reset_connack_retain_available(mr_packet_ctx *pctx);

int mr_get_connack_maximum_packet_size(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_set_connack_maximum_packet_size(mr_packet_ctx *pctx, uint32_t u32);
int mr_reset_connack_maximum_packet_size(mr_packet_ctx *pctx);

int mr_get_connack_assigned_client_identifier(mr_packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connack_assigned_client_identifier(mr_packet_ctx *pctx, char *cv0);
int mr_reset_connack_assigned_client_identifier(mr_packet_ctx *pctx);

int mr_get_connack_topic_alias_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists);
int mr_set_connack_topic_alias_maximum(mr_packet_ctx *pctx, uint16_t u16);
int mr_reset_connack_topic_alias_maximum(mr_packet_ctx *pctx);

int mr_get_connack_reason_string(mr_packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connack_reason_string(mr_packet_ctx *pctx, char *cv0);
int mr_reset_connack_reason_string(mr_packet_ctx *pctx);

int mr_get_connack_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists);
int mr_set_connack_user_properties(mr_packet_ctx *pctx, mr_string_pair *spv0, size_t len);
int mr_reset_connack_user_properties(mr_packet_ctx *pctx);

int mr_get_connack_wildcard_subscription_available(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connack_wildcard_subscription_available(mr_packet_ctx *pctx, uint8_t u8);
int mr_reset_connack_wildcard_subscription_available(mr_packet_ctx *pctx);

int mr_get_connack_subscription_identifiers_available(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connack_subscription_identifiers_available(mr_packet_ctx *pctx, uint8_t u8);
int mr_reset_connack_subscription_identifiers_available(mr_packet_ctx *pctx);

int mr_get_connack_shared_subscription_available(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connack_shared_subscription_available(mr_packet_ctx *pctx, uint8_t u8);
int mr_reset_connack_shared_subscription_available(mr_packet_ctx *pctx);

int mr_get_connack_server_keep_alive(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists);
int mr_set_connack_server_keep_alive(mr_packet_ctx *pctx, uint16_t u16);
int mr_reset_connack_server_keep_alive(mr_packet_ctx *pctx);

int mr_get_connack_response_information(mr_packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connack_response_information(mr_packet_ctx *pctx, char *cv0);
int mr_reset_connack_response_information(mr_packet_ctx *pctx);

int mr_get_connack_server_reference(mr_packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connack_server_reference(mr_packet_ctx *pctx, char *cv0);
int mr_reset_connack_server_reference(mr_packet_ctx *pctx);

int mr_get_connack_authentication_method(mr_packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connack_authentication_method(mr_packet_ctx *pctx, char *cv0);
int mr_reset_connack_authentication_method(mr_packet_ctx *pctx);

int mr_get_connack_authentication_data(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists);
int mr_set_connack_authentication_data(mr_packet_ctx *pctx, uint8_t *u8v0, size_t len);
int mr_reset_connack_authentication_data(mr_packet_ctx *pctx);

int mr_validate_connack_values(mr_packet_ctx *pctx);
int mr_validate_connack_extra(mr_packet_ctx *pctx);

#ifdef __cplusplus
}
#endif

#endif // MISTER_H