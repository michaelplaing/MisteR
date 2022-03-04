// mister.h

#ifndef MISTER_H
#define MISTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdlib.h>
/*
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
 */
// spec & mosquitto
enum mqtt_reason_codes {
    MQTT_RC_SUCCESS = 0,                                    ///< CONNACK, PUBACK, PUBREC, PUBREL, PUBCOMP, UNSUBACK, AUTH
    MQTT_RC_NORMAL_DISCONNECTION = 0,                       ///< DISCONNECT
    MQTT_RC_GRANTED_QOS0 = 0,                               ///< SUBACK
    MQTT_RC_GRANTED_QOS1 = 1,                               ///< SUBACK
    MQTT_RC_GRANTED_QOS2 = 2,                               ///< SUBACK
    MQTT_RC_DISCONNECT_WITH_WILL_MSG = 4,                   ///< DISCONNECT
    MQTT_RC_NO_MATCHING_SUBSCRIBERS = 16,                   ///< PUBACK, PUBREC
    MQTT_RC_NO_SUBSCRIPTION_EXISTED = 17,                   ///< UNSUBACK
    MQTT_RC_CONTINUE_AUTHENTICATION = 24,                   ///< AUTH
    MQTT_RC_REAUTHENTICATE = 25,                            ///< AUTH
    MQTT_RC_UNSPECIFIED = 128,                              ///< CONNACK, PUBACK, PUBREC, SUBACK, UNSUBACK, DISCONNECT
    MQTT_RC_MALFORMED_PACKET = 129,		                    ///< CONNACK, DISCONNECT
    MQTT_RC_PROTOCOL_ERROR = 130,		                    ///< DISCONNECT
    MQTT_RC_IMPLEMENTATION_SPECIFIC = 131,                  ///< CONNACK, PUBACK, PUBREC, SUBACK, UNSUBACK, DISCONNECT
    MQTT_RC_UNSUPPORTED_PROTOCOL_VERSION = 132,             ///< CONNACK
    MQTT_RC_CLIENTID_NOT_VALID = 133,                       ///< CONNACK
    MQTT_RC_BAD_USERNAME_OR_PASSWORD = 134,                 ///< CONNACK
    MQTT_RC_NOT_AUTHORIZED = 135,                           ///< CONNACK, PUBACK, PUBREC, SUBACK, UNSUBACK, DISCONNECT
    MQTT_RC_SERVER_UNAVAILABLE = 136,                       ///< CONNACK
    MQTT_RC_SERVER_BUSY = 137,                              ///< CONNACK, DISCONNECT
    MQTT_RC_BANNED = 138,                                   ///< CONNACK
    MQTT_RC_SERVER_SHUTTING_DOWN = 139,                     ///< DISCONNECT
    MQTT_RC_BAD_AUTHENTICATION_METHOD = 140,                ///< CONNACK
    MQTT_RC_KEEP_ALIVE_TIMEOUT = 141,                       ///< DISCONNECT
    MQTT_RC_SESSION_TAKEN_OVER = 142,                       ///< DISCONNECT
    MQTT_RC_TOPIC_FILTER_INVALID = 143,                     ///< SUBACK, UNSUBACK, DISCONNECT
    MQTT_RC_TOPIC_NAME_INVALID = 144,                       ///< CONNACK, PUBACK, PUBREC, DISCONNECT
    MQTT_RC_PACKET_ID_IN_USE = 145,                         ///< PUBACK, SUBACK, UNSUBACK
    MQTT_RC_PACKET_ID_NOT_FOUND = 146,                      ///< PUBREL, PUBCOMP
    MQTT_RC_RECEIVE_MAXIMUM_EXCEEDED = 147,                 ///< DISCONNECT
    MQTT_RC_TOPIC_ALIAS_INVALID = 148,                      ///< DISCONNECT
    MQTT_RC_PACKET_TOO_LARGE = 149,                         ///< CONNACK, PUBACK, PUBREC, DISCONNECT
    MQTT_RC_MESSAGE_RATE_TOO_HIGH = 150,                    ///< DISCONNECT
    MQTT_RC_QUOTA_EXCEEDED = 151,                           ///< PUBACK, PUBREC, SUBACK, DISCONNECT
    MQTT_RC_ADMINISTRATIVE_ACTION = 152,                    ///< DISCONNECT
    MQTT_RC_PAYLOAD_FORMAT_INVALID = 153,                   ///< CONNACK, DISCONNECT
    MQTT_RC_RETAIN_NOT_SUPPORTED = 154,                     ///< CONNACK, DISCONNECT
    MQTT_RC_QOS_NOT_SUPPORTED = 155,                        ///< CONNACK, DISCONNECT
    MQTT_RC_USE_ANOTHER_SERVER = 156,                       ///< CONNACK, DISCONNECT
    MQTT_RC_SERVER_MOVED = 157,                             ///< CONNACK, DISCONNECT
    MQTT_RC_SHARED_SUBSCRIPTIONS_NOT_SUPPORTED = 158,       ///< SUBACK, DISCONNECT
    MQTT_RC_CONNECTION_RATE_EXCEEDED = 159,                 ///< CONNACK, DISCONNECT
    MQTT_RC_MAXIMUM_CONNECT_TIME = 160,                     ///< DISCONNECT
    MQTT_RC_SUBSCRIPTION_IDENTIFIERS_NOT_SUPPORTED = 161,   ///< SUBACK, DISCONNECT
    MQTT_RC_WILDCARD_SUBSCRIPTIONS_NOT_SUPPORTED = 162,     ///< SUBACK, DISCONNECT
};

extern int mr_errno;

typedef struct mr_packet_ctx mr_packet_ctx;

typedef struct mr_string_pair {
    char *name;
    char *value;
} mr_string_pair;

typedef struct mr_topic_filter {
    char *topic_filter;
    uint8_t maximum_qos;            // packed into bits 0-1
    uint8_t no_local;               // packed into bit 2
    uint8_t retain_as_published;    // packed into bit 3
    uint8_t retain_handling;        // packed into bits 4-5 (6-7 are reserved and set to 0)
} mr_topic_filter;

// utilities

int mr_print_hexdump(uint8_t *u8v, const size_t u8vlen);
int mr_get_hexdump(char *cv0, const size_t cvlen, const uint8_t *u8v, const size_t u8vlen);
void mr_compress_spaces_lines(char *cv);
size_t u64tobase62cv(uint64_t u64, char* cv);
void get_uuidbase62cv(char *uuidbase62cv);

// connect packet

int mr_init_connect_packet(mr_packet_ctx **ppctx);
int mr_init_unpack_connect_packet(mr_packet_ctx **ppctx, const uint8_t *u8v0, const size_t u8vlen);
int mr_pack_connect_packet(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *pu8vlen);
int mr_free_connect_packet(mr_packet_ctx *pctx);

int mr_get_connect_packet_type(mr_packet_ctx *pctx, uint8_t *pu8);
int mr_get_connect_reserved_header(mr_packet_ctx *pctx, uint8_t *pu8);
int mr_get_connect_remaining_length(mr_packet_ctx *pctx, uint32_t *pu32);
int mr_get_connect_protocol_name(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);
int mr_get_connect_protocol_version(mr_packet_ctx *pctx, uint8_t *pu8);
int mr_get_connect_reserved_flags(mr_packet_ctx *pctx, bool *pflag_value);

int mr_get_connect_clean_start(mr_packet_ctx *pctx, bool *pflag_value);
int mr_set_connect_clean_start(mr_packet_ctx *pctx, const bool flag_value);

int mr_get_connect_will_flag(mr_packet_ctx *pctx, bool *pflag_value);
int mr_set_connect_will_flag(mr_packet_ctx *pctx, const bool flag_value);

int mr_get_connect_will_qos(mr_packet_ctx *pctx, uint8_t *pu8);
int mr_set_connect_will_qos(mr_packet_ctx *pctx, const uint8_t u8);

int mr_get_connect_will_retain(mr_packet_ctx *pctx, bool *pflag_value);
int mr_set_connect_will_retain(mr_packet_ctx *pctx, const bool flag_value);

int mr_get_connect_password_flag(mr_packet_ctx *pctx, bool *pflag_value);
int mr_get_connect_username_flag(mr_packet_ctx *pctx, bool *pflag_value);

int mr_get_connect_keep_alive(mr_packet_ctx *pctx, uint16_t *pu16);
int mr_set_connect_keep_alive(mr_packet_ctx *pctx, const uint16_t u16);

int mr_get_connect_property_length(mr_packet_ctx *pctx, uint32_t *pu32);

int mr_get_connect_session_expiry_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag);
int mr_set_connect_session_expiry_interval(mr_packet_ctx *pctx, const uint32_t u32);
int mr_reset_connect_session_expiry_interval(mr_packet_ctx *pctx);

int mr_get_connect_receive_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists_flag);
int mr_set_connect_receive_maximum(mr_packet_ctx *pctx, const uint16_t u16);
int mr_reset_connect_receive_maximum(mr_packet_ctx *pctx);

int mr_get_connect_maximum_packet_size(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag);
int mr_set_connect_maximum_packet_size(mr_packet_ctx *pctx, const uint32_t u32);
int mr_reset_connect_maximum_packet_size(mr_packet_ctx *pctx);

int mr_get_connect_topic_alias_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists_flag);
int mr_set_connect_topic_alias_maximum(mr_packet_ctx *pctx, const uint16_t u16);
int mr_reset_connect_topic_alias_maximum(mr_packet_ctx *pctx);

int mr_get_connect_request_response_information(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag);
int mr_set_connect_request_response_information(mr_packet_ctx *pctx, const uint8_t u8);
int mr_reset_connect_request_response_information(mr_packet_ctx *pctx);

int mr_get_connect_request_problem_information(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag);
int mr_set_connect_request_problem_information(mr_packet_ctx *pctx, const uint8_t u8);
int mr_reset_connect_request_problem_information(mr_packet_ctx *pctx);

int mr_get_connect_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists_flag);
int mr_set_connect_user_properties(mr_packet_ctx *pctx, const mr_string_pair *spv0, const size_t len);
int mr_reset_connect_user_properties(mr_packet_ctx *pctx);

int mr_get_connect_authentication_method(mr_packet_ctx *pctx, char **cv0, bool *pexists_flag);
int mr_set_connect_authentication_method(mr_packet_ctx *pctx, const char *cv0);
int mr_reset_connect_authentication_method(mr_packet_ctx *pctx);

int mr_get_connect_authentication_data(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists_flag);
int mr_set_connect_authentication_data(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len);
int mr_reset_connect_authentication_data(mr_packet_ctx *pctx);

int mr_get_connect_client_identifier(mr_packet_ctx *pctx, char **pcv0);
int mr_set_connect_client_identifier(mr_packet_ctx *pctx, const char *cv0);

int mr_get_connect_will_property_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag);

int mr_get_connect_will_delay_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag);
int mr_set_connect_will_delay_interval(mr_packet_ctx *pctx, const uint32_t u32);
int mr_reset_connect_will_delay_interval(mr_packet_ctx *pctx);

int mr_get_connect_payload_format_indicator(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag);
int mr_set_connect_payload_format_indicator(mr_packet_ctx *pctx, const uint8_t u8);
int mr_reset_connect_payload_format_indicator(mr_packet_ctx *pctx);

int mr_get_connect_message_expiry_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag);
int mr_set_connect_message_expiry_interval(mr_packet_ctx *pctx, const uint32_t u32);
int mr_reset_connect_message_expiry_interval(mr_packet_ctx *pctx);

int mr_get_connect_content_type(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag);
int mr_set_connect_content_type(mr_packet_ctx *pctx, const char *cv0);
int mr_reset_connect_content_type(mr_packet_ctx *pctx);

int mr_get_connect_response_topic(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag);
int mr_set_connect_response_topic(mr_packet_ctx *pctx, const char *cv0);
int mr_reset_connect_response_topic(mr_packet_ctx *pctx);

int mr_get_connect_correlation_data(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists_flag);
int mr_set_connect_correlation_data(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len);
int mr_reset_connect_correlation_data(mr_packet_ctx *pctx);

int mr_get_connect_will_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists_flag);
int mr_set_connect_will_user_properties(mr_packet_ctx *pctx, const mr_string_pair *spv0, const size_t len);
int mr_reset_connect_will_user_properties(mr_packet_ctx *pctx);

int mr_get_connect_will_topic(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag);
int mr_set_connect_will_topic(mr_packet_ctx *pctx, const char *cv0);
int mr_reset_connect_will_topic(mr_packet_ctx *pctx);

int mr_get_connect_will_payload(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists_flag);
int mr_set_connect_will_payload(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len);
int mr_reset_connect_will_payload(mr_packet_ctx *pctx);

int mr_get_connect_user_name(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag);
int mr_set_connect_user_name(mr_packet_ctx *pctx, const char *cv0);
int mr_reset_connect_user_name(mr_packet_ctx *pctx);

int mr_get_connect_password(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists_flag);
int mr_set_connect_password(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len);
int mr_reset_connect_password(mr_packet_ctx *pctx);

int mr_get_connect_printable(mr_packet_ctx *pctx, const bool all_flag, char **pcv);

// connack packet

int mr_init_connack_packet(mr_packet_ctx **ppctx);
int mr_init_unpack_connack_packet(mr_packet_ctx **ppctx, const uint8_t *u8v0, const size_t u8vlen);
int mr_pack_connack_packet(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *pu8vlen);
int mr_free_connack_packet(mr_packet_ctx *pctx);

int mr_get_connack_packet_type(mr_packet_ctx *pctx, uint8_t *pu8);
int mr_get_connack_reserved_header(mr_packet_ctx *pctx, uint8_t *pu8);
int mr_get_connack_remaining_length(mr_packet_ctx *pctx, uint32_t *pu32);

int mr_get_connack_session_present(mr_packet_ctx *pctx, bool *pflag_value);
int mr_set_connack_session_present(mr_packet_ctx *pctx, const bool flag_value);

int mr_get_connack_reserved_flags(mr_packet_ctx *pctx, uint8_t *pu8);

int mr_get_connack_connect_reason_code(mr_packet_ctx *pctx, uint8_t *pu8);
int mr_set_connack_connect_reason_code(mr_packet_ctx *pctx, const uint8_t u8);

int mr_get_connack_property_length(mr_packet_ctx *pctx, uint32_t *pu32);

int mr_get_connack_session_expiry_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag);
int mr_set_connack_session_expiry_interval(mr_packet_ctx *pctx, const uint32_t u32);
int mr_reset_connack_session_expiry_interval(mr_packet_ctx *pctx);

int mr_get_connack_receive_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists_flag);
int mr_set_connack_receive_maximum(mr_packet_ctx *pctx, const uint16_t u16);
int mr_reset_connack_receive_maximum(mr_packet_ctx *pctx);

int mr_get_connack_maximum_qos(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag);
int mr_set_connack_maximum_qos(mr_packet_ctx *pctx, const uint8_t u8);
int mr_reset_connack_maximum_qos(mr_packet_ctx *pctx);

int mr_get_connack_retain_available(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag);
int mr_set_connack_retain_available(mr_packet_ctx *pctx, const uint8_t u8);
int mr_reset_connack_retain_available(mr_packet_ctx *pctx);

int mr_get_connack_maximum_packet_size(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag);
int mr_set_connack_maximum_packet_size(mr_packet_ctx *pctx, const uint32_t u32);
int mr_reset_connack_maximum_packet_size(mr_packet_ctx *pctx);

int mr_get_connack_assigned_client_identifier(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag);
int mr_set_connack_assigned_client_identifier(mr_packet_ctx *pctx, const char *cv0);
int mr_reset_connack_assigned_client_identifier(mr_packet_ctx *pctx);

int mr_get_connack_topic_alias_maximum(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists_flag);
int mr_set_connack_topic_alias_maximum(mr_packet_ctx *pctx, const uint16_t u16);
int mr_reset_connack_topic_alias_maximum(mr_packet_ctx *pctx);

int mr_get_connack_reason_string(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag);
int mr_set_connack_reason_string(mr_packet_ctx *pctx, const char *cv0);
int mr_reset_connack_reason_string(mr_packet_ctx *pctx);

int mr_get_connack_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists_flag);
int mr_set_connack_user_properties(mr_packet_ctx *pctx, const mr_string_pair *spv0, const size_t len);
int mr_reset_connack_user_properties(mr_packet_ctx *pctx);

int mr_get_connack_wildcard_subscription_available(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag);
int mr_set_connack_wildcard_subscription_available(mr_packet_ctx *pctx, const uint8_t u8);
int mr_reset_connack_wildcard_subscription_available(mr_packet_ctx *pctx);

int mr_get_connack_subscription_identifiers_available(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag);
int mr_set_connack_subscription_identifiers_available(mr_packet_ctx *pctx, const uint8_t u8);
int mr_reset_connack_subscription_identifiers_available(mr_packet_ctx *pctx);

int mr_get_connack_shared_subscription_available(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag);
int mr_set_connack_shared_subscription_available(mr_packet_ctx *pctx, const uint8_t u8);
int mr_reset_connack_shared_subscription_available(mr_packet_ctx *pctx);

int mr_get_connack_server_keep_alive(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists_flag);
int mr_set_connack_server_keep_alive(mr_packet_ctx *pctx, const uint16_t u16);
int mr_reset_connack_server_keep_alive(mr_packet_ctx *pctx);

int mr_get_connack_response_information(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag);
int mr_set_connack_response_information(mr_packet_ctx *pctx, const char *cv0);
int mr_reset_connack_response_information(mr_packet_ctx *pctx);

int mr_get_connack_server_reference(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag);
int mr_set_connack_server_reference(mr_packet_ctx *pctx, const char *cv0);
int mr_reset_connack_server_reference(mr_packet_ctx *pctx);

int mr_get_connack_authentication_method(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag);
int mr_set_connack_authentication_method(mr_packet_ctx *pctx, const char *cv0);
int mr_reset_connack_authentication_method(mr_packet_ctx *pctx);

int mr_get_connack_authentication_data(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists_flag);
int mr_set_connack_authentication_data(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len);
int mr_reset_connack_authentication_data(mr_packet_ctx *pctx);

int mr_get_connack_printable(mr_packet_ctx *pctx, const bool all_flag, char **pcv);

// PUBLISH

int mr_init_publish_packet(mr_packet_ctx **ppctx);
int mr_init_unpack_publish_packet(mr_packet_ctx **ppctx, const uint8_t *u8v0, const size_t u8vlen);
int mr_pack_publish_packet(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *pu8vlen);
int mr_free_publish_packet(mr_packet_ctx *pctx);

int mr_get_publish_packet_type(mr_packet_ctx *pctx, uint8_t *pu8);
int mr_get_publish_property_length(mr_packet_ctx *pctx, uint32_t *pu32);

int mr_get_publish_dup(mr_packet_ctx *pctx, bool *pflag_value);
int mr_set_publish_dup(mr_packet_ctx *pctx, const bool flag_value);

int mr_get_publish_qos(mr_packet_ctx *pctx, uint8_t *pu8);
int mr_set_publish_qos(mr_packet_ctx *pctx, const uint8_t u8);

int mr_get_publish_retain(mr_packet_ctx *pctx, bool *pflag_value);
int mr_set_publish_retain(mr_packet_ctx *pctx, const bool flag_value);

int mr_get_publish_remaining_length(mr_packet_ctx *pctx, uint32_t *pu32);

int mr_get_publish_topic_name(mr_packet_ctx *pctx, char **pcv0);
int mr_set_publish_topic_name(mr_packet_ctx *pctx, const char *cv0);

int mr_get_publish_packet_identifier(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists_flag);
int mr_set_publish_packet_identifier(mr_packet_ctx *pctx, const uint16_t u16);
int mr_reset_publish_packet_identifier(mr_packet_ctx *pctx);

int mr_get_publish_property_length(mr_packet_ctx *pctx, uint32_t *pu32);

int mr_get_publish_payload_format_indicator(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag);
int mr_set_publish_payload_format_indicator(mr_packet_ctx *pctx, const uint8_t u8);
int mr_reset_publish_payload_format_indicator(mr_packet_ctx *pctx);

int mr_get_publish_message_expiry_interval(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag);
int mr_set_publish_message_expiry_interval(mr_packet_ctx *pctx, const uint32_t u32);
int mr_reset_publish_message_expiry_interval(mr_packet_ctx *pctx);

int mr_get_publish_topic_alias(mr_packet_ctx *pctx, uint16_t *pu16, bool *pexists_flag);
int mr_set_publish_topic_alias(mr_packet_ctx *pctx, const uint16_t u16);
int mr_reset_publish_topic_alias(mr_packet_ctx *pctx);

int mr_get_publish_response_topic(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag);
int mr_set_publish_response_topic(mr_packet_ctx *pctx, const char *cv0);
int mr_reset_publish_response_topic(mr_packet_ctx *pctx);

int mr_get_publish_correlation_data(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen, bool *pexists_flag);
int mr_set_publish_correlation_data(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len);
int mr_reset_publish_correlation_data(mr_packet_ctx *pctx);

int mr_get_publish_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists_flag);
int mr_set_publish_user_properties(mr_packet_ctx *pctx, const mr_string_pair *spv0, const size_t len);
int mr_reset_publish_user_properties(mr_packet_ctx *pctx);

int mr_get_publish_subscription_identifiers(mr_packet_ctx *pctx, uint32_t **pu32v0, size_t *plen, bool *pexists_flag);
int mr_set_publish_subscription_identifiers(mr_packet_ctx *pctx, const uint32_t *u32v0, const size_t len);
int mr_reset_publish_subscription_identifiers(mr_packet_ctx *pctx);

int mr_get_publish_content_type(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag);
int mr_set_publish_content_type(mr_packet_ctx *pctx, const char *cv0);
int mr_reset_publish_content_type(mr_packet_ctx *pctx);

int mr_get_publish_payload(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);
int mr_set_publish_payload(mr_packet_ctx *pctx, const uint8_t *u8v0, const size_t len);

int mr_get_publish_printable(mr_packet_ctx *pctx, const bool all_flag, char **pcv);

// PUBACK

int mr_init_puback_packet(mr_packet_ctx **ppctx);
int mr_init_unpack_puback_packet(mr_packet_ctx **ppctx, const uint8_t *u8v0, const size_t u8vlen);
int mr_pack_puback_packet(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *pu8vlen);
int mr_free_puback_packet(mr_packet_ctx *pctx);

int mr_get_puback_packet_type(mr_packet_ctx *pctx, uint8_t *pu8);
int mr_get_puback_reserved_header(mr_packet_ctx *pctx, uint8_t *pu8);
int mr_get_puback_remaining_length(mr_packet_ctx *pctx, uint32_t *pu32);

int mr_get_puback_packet_identifier(mr_packet_ctx *pctx, uint16_t *pu16);
int mr_set_puback_packet_identifier(mr_packet_ctx *pctx, const uint16_t u16);

int mr_get_puback_puback_reason_code(mr_packet_ctx *pctx, uint8_t *pu8, bool *pexists_flag);
int mr_set_puback_puback_reason_code(mr_packet_ctx *pctx, const uint8_t u8);
int mr_reset_puback_puback_reason_code(mr_packet_ctx *pctx);

int mr_get_puback_property_length(mr_packet_ctx *pctx, uint32_t *pu32, bool *pexists_flag);

int mr_get_puback_reason_string(mr_packet_ctx *pctx, char **pcv0, bool *pexists_flag);
int mr_set_puback_reason_string(mr_packet_ctx *pctx, const char *cv0);
int mr_reset_puback_reason_string(mr_packet_ctx *pctx);

int mr_get_puback_user_properties(mr_packet_ctx *pctx, mr_string_pair **pspv0, size_t *plen, bool *pexists_flag);
int mr_set_puback_user_properties(mr_packet_ctx *pctx, const mr_string_pair *spv0, const size_t len);
int mr_reset_puback_user_properties(mr_packet_ctx *pctx);

int mr_get_puback_printable(mr_packet_ctx *pctx, const bool all_flag, char **pcv);

#ifdef __cplusplus
}
#endif

#endif // MISTER_H