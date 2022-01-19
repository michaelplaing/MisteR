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

// spec: Reason Codes for Malformed Packet and Protocol Errors
enum mqtt_reason_codes {
    MQTT_MALFORMED_PACKET                       = 0x81,
    MQTT_PROTOCOL_ERROR                         = 0x82,
    MQTT_RECEIVE_MAXIMUM_EXCEEDED               = 0x93,
    MQTT_PACKET_TOO_LARGE                       = 0x95,
    MQTT_RETAIN_NOT_SUPPORTED                   = 0x9A,
    MQTT_QOS_NOT_SUPPORTED                      = 0x9B,
    MQTT_SHARED_SUBSCRIPTIONS_NOT_SUPPORTED     = 0x9E,
    MQTT_SUBSCRIPTION_IDENTIFIERS_NOT_SUPPORTED = 0xA1,
    MQTT_WILDCARD_SUBSCRIPTIONS_NOT_SUPPORTED   = 0xA2
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

typedef struct packet_ctx {
    uint8_t mqtt_packet_type;
    char *mqtt_packet_name;
    char *mdata_dump;
    uint8_t *u8v0;
    bool u8valloc;
    size_t u8vlen;
    size_t u8vpos;
    struct mr_mdata *mdata0;
    size_t mdata_count;
} packet_ctx;

typedef struct string_pair {
    char *name;
    char *value;
} string_pair;

// packet utilities

int mr_print_hexdump(const uint8_t *u8v, const size_t u8vlen);
int mr_get_hexdump(char *cv0, const size_t cvlen, const uint8_t *u8v, const size_t u8vlen);
void mr_compress_spaces_lines(char *cv);

// connect packet

int mr_init_connect_packet(packet_ctx **ppctx);
int mr_init_unpack_connect_packet(packet_ctx **ppctx, uint8_t *u8v0, size_t ulen);
static int mr_connect_packet_check(packet_ctx *pctx);
int mr_pack_connect_packet(packet_ctx *pctx);
int mr_free_connect_packet(packet_ctx *pctx);
int mr_connect_mdata_dump(packet_ctx *pctx);

int mr_get_connect_packet_type(packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_get_connect_remaining_length(packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_get_connect_protocol_name(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists);
int mr_get_connect_protocol_version(packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_get_connect_reserved(packet_ctx *pctx, bool *pboolean, bool *pexists);

int mr_get_connect_clean_start(packet_ctx *pctx, bool *pboolean, bool *pexists);
int mr_set_connect_clean_start(packet_ctx *pctx, bool boolean);
int mr_reset_connect_clean_start(packet_ctx *pctx);

int mr_get_connect_will_flag(packet_ctx *pctx, bool *pboolean, bool *pexists);
int mr_get_connect_will_qos(packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_get_connect_will_retain(packet_ctx *pctx, bool *pboolean, bool *pexists);
int mr_get_connect_password_flag(packet_ctx *pctx, bool *pboolean, bool *pexists);
int mr_get_connect_username_flag(packet_ctx *pctx, bool *pboolean, bool *pexists);

int mr_get_connect_keep_alive(packet_ctx *pctx, uint16_t *pu16, bool *pexists);
int mr_set_connect_keep_alive(packet_ctx *pctx, uint16_t u16);
int mr_reset_connect_keep_alive(packet_ctx *pctx);

int mr_get_connect_property_length(packet_ctx *pctx, uint32_t *pu32, bool *pexists);

int mr_get_connect_session_expiry_interval(packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_set_connect_session_expiry_interval(packet_ctx *pctx, uint32_t u32);
int mr_reset_connect_session_expiry_interval(packet_ctx *pctx);

int mr_get_connect_receive_maximum(packet_ctx *pctx, uint16_t *pu16, bool *pexists);
int mr_set_connect_receive_maximum(packet_ctx *pctx, uint16_t u16);
int mr_reset_connect_receive_maximum(packet_ctx *pctx);

int mr_get_connect_maximum_packet_size(packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_set_connect_maximum_packet_size(packet_ctx *pctx, uint32_t u32);
int mr_reset_connect_maximum_packet_size(packet_ctx *pctx);

int mr_get_connect_topic_alias_maximum(packet_ctx *pctx, uint16_t *pu16, bool *pexists);
int mr_set_connect_topic_alias_maximum(packet_ctx *pctx, uint16_t u16);
int mr_reset_connect_topic_alias_maximum(packet_ctx *pctx);

int mr_get_connect_request_response_information(packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connect_request_response_information(packet_ctx *pctx, uint8_t u8);
int mr_reset_connect_request_response_information(packet_ctx *pctx);

int mr_get_connect_request_problem_information(packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connect_request_problem_information(packet_ctx *pctx, uint8_t u8);
int mr_reset_connect_request_problem_information(packet_ctx *pctx);

int mr_get_connect_user_properties(packet_ctx *pctx, string_pair **pspv0, size_t *plen, bool *pexists);
int mr_set_connect_user_properties(packet_ctx *pctx, string_pair *spv0, size_t len);
int mr_reset_connect_user_properties(packet_ctx *pctx);

int mr_get_connect_authentication_method(packet_ctx *pctx, char **cv0, bool *pexists);
int mr_set_connect_authentication_method(packet_ctx *pctx, char *cv0);
int mr_reset_connect_authentication_method(packet_ctx *pctx);

int mr_get_connect_authentication_data(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists);
int mr_set_connect_authentication_data(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int mr_reset_connect_authentication_data(packet_ctx *pctx);

int mr_get_connect_client_identifier(packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connect_client_identifier(packet_ctx *pctx, char *cv0);
int mr_reset_connect_client_identifier(packet_ctx *pctx);

int mr_get_connect_will_property_length(packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_get_connect_will_delay_interval(packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_get_connect_payload_format_indicator(packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_get_connect_message_expiry_interval(packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_get_connect_content_type(packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_get_connect_response_topic(packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_get_connect_correlation_data(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists);
int mr_get_connect_will_user_properties(packet_ctx *pctx, string_pair **pspv0, size_t *plen, bool *pexists);
int mr_get_connect_will_topic(packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_get_connect_will_payload(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists);

int mr_get_connect_user_name(packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connect_user_name(packet_ctx *pctx, char *cv0);
int mr_reset_connect_user_name(packet_ctx *pctx);

int mr_get_connect_password(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists);
int mr_set_connect_password(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int mr_reset_connect_password(packet_ctx *pctx);

// connect packet - will values

typedef struct mr_connect_will_data {
    bool will_flag;
    uint8_t will_qos;
    bool will_retain;
    uint32_t will_property_length;
    uint32_t will_delay_interval;
    uint8_t payload_format_indicator;
    uint32_t message_expiry_interval;
    char *content_type;
    char *response_topic;
    uint8_t *correlation_data;
    size_t correlation_data_len;
    string_pair *will_user_properties;
    size_t will_user_properties_len;
    char *will_topic;
    uint8_t *will_payload;
    size_t will_payload_len;
} mr_connect_will_data;

int mr_clear_connect_will_data(mr_connect_will_data *pwd);

int mr_get_connect_will_values(packet_ctx *pctx, mr_connect_will_data *pwd);
int mr_set_connect_will_values(packet_ctx *pctx, mr_connect_will_data *pwd);
int mr_reset_connect_will_values(packet_ctx *pctx);
int mr_validate_connect_will_values(packet_ctx *pctx);

// connack packet

int mr_init_connack_packet(packet_ctx **ppctx);
int mr_init_unpack_connack_packet(packet_ctx **ppctx, uint8_t *u8v0, size_t ulen);
static int mr_connack_packet_check(packet_ctx *pctx);
int mr_pack_connack_packet(packet_ctx *pctx);
int mr_free_connack_packet(packet_ctx *pctx);
int mr_connack_mdata_dump(packet_ctx *pctx);

int mr_get_connack_packet_type(packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_get_connack_remaining_length(packet_ctx *pctx, uint32_t *pu32, bool *pexists);

int mr_get_connack_session_present(packet_ctx *pctx, bool *pboolean, bool *pexists);
int mr_set_connack_session_present(packet_ctx *pctx, bool boolean);
int mr_reset_connack_session_present(packet_ctx *pctx);

int mr_get_connack_reserved(packet_ctx *pctx, uint8_t *pu8, bool *pexists);

int mr_get_connack_connect_reason_code(packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connack_connect_reason_code(packet_ctx *pctx, uint8_t u8);
int mr_reset_connack_connect_reason_code(packet_ctx *pctx);

int mr_get_connack_property_length(packet_ctx *pctx, uint32_t *pu32, bool *pexists);

int mr_get_connack_session_expiry_interval(packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_set_connack_session_expiry_interval(packet_ctx *pctx, uint32_t u32);
int mr_reset_connack_session_expiry_interval(packet_ctx *pctx);

int mr_get_connack_receive_maximum(packet_ctx *pctx, uint16_t *pu16, bool *pexists);
int mr_set_connack_receive_maximum(packet_ctx *pctx, uint16_t u16);
int mr_reset_connack_receive_maximum(packet_ctx *pctx);

int mr_get_connack_maximum_qos(packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connack_maximum_qos(packet_ctx *pctx, uint8_t u8);
int mr_reset_connack_maximum_qos(packet_ctx *pctx);

int mr_get_connack_retain_available(packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connack_retain_available(packet_ctx *pctx, uint8_t u8);
int mr_reset_connack_retain_available(packet_ctx *pctx);

int mr_get_connack_maximum_packet_size(packet_ctx *pctx, uint32_t *pu32, bool *pexists);
int mr_set_connack_maximum_packet_size(packet_ctx *pctx, uint32_t u32);
int mr_reset_connack_maximum_packet_size(packet_ctx *pctx);

int mr_get_connack_assigned_client_identifier(packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connack_assigned_client_identifier(packet_ctx *pctx, char *cv0);
int mr_reset_connack_assigned_client_identifier(packet_ctx *pctx);

int mr_get_connack_topic_alias_maximum(packet_ctx *pctx, uint16_t *pu16, bool *pexists);
int mr_set_connack_topic_alias_maximum(packet_ctx *pctx, uint16_t u16);
int mr_reset_connack_topic_alias_maximum(packet_ctx *pctx);

int mr_get_connack_reason_string(packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connack_reason_string(packet_ctx *pctx, char *cv0);
int mr_reset_connack_reason_string(packet_ctx *pctx);

int mr_get_connack_user_properties(packet_ctx *pctx, string_pair **pspv0, size_t *plen, bool *pexists);
int mr_set_connack_user_properties(packet_ctx *pctx, string_pair *spv0, size_t len);
int mr_reset_connack_user_properties(packet_ctx *pctx);

int mr_get_connack_wildcard_subscription_available(packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connack_wildcard_subscription_available(packet_ctx *pctx, uint8_t u8);
int mr_reset_connack_wildcard_subscription_available(packet_ctx *pctx);

int mr_get_connack_subscription_identifiers_available(packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connack_subscription_identifiers_available(packet_ctx *pctx, uint8_t u8);
int mr_reset_connack_subscription_identifiers_available(packet_ctx *pctx);

int mr_get_connack_shared_subscription_available(packet_ctx *pctx, uint8_t *pu8, bool *pexists);
int mr_set_connack_shared_subscription_available(packet_ctx *pctx, uint8_t u8);
int mr_reset_connack_shared_subscription_available(packet_ctx *pctx);

int mr_get_connack_server_keep_alive(packet_ctx *pctx, uint16_t *pu16, bool *pexists);
int mr_set_connack_server_keep_alive(packet_ctx *pctx, uint16_t u16);
int mr_reset_connack_server_keep_alive(packet_ctx *pctx);

int mr_get_connack_response_information(packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connack_response_information(packet_ctx *pctx, char *cv0);
int mr_reset_connack_response_information(packet_ctx *pctx);

int mr_get_connack_server_reference(packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connack_server_reference(packet_ctx *pctx, char *cv0);
int mr_reset_connack_server_reference(packet_ctx *pctx);

int mr_get_connack_authentication_method(packet_ctx *pctx, char **pcv0, bool *pexists);
int mr_set_connack_authentication_method(packet_ctx *pctx, char *cv0);
int mr_reset_connack_authentication_method(packet_ctx *pctx);

int mr_get_connack_authentication_data(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists);
int mr_set_connack_authentication_data(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int mr_reset_connack_authentication_data(packet_ctx *pctx);

#ifdef __cplusplus
}
#endif

#endif // MISTER_H