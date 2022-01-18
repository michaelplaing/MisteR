#ifndef CONNECT_INTERNAL_H
#define CONNECT_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

enum CONNECT_MDATA_FIELDS { // Same order as _CONNECT_MDATA_TEMPLATE
    CONNECT_PACKET_TYPE,
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

struct mr_connect_values { // may or may not be useful
    const uint8_t packet_type;
    uint32_t remaining_length;
    const uint8_t *protocol_name;
    const uint8_t protocol_version;
    const bool reserved;
    bool clean_start;
    bool will_flag;
    uint8_t will_qos;
    bool will_retain;
    bool password_flag;
    bool username_flag;
    uint16_t keep_alive;
    uint32_t property_length;
    uint32_t session_expiry;
    uint16_t receive_maximum;
    uint32_t maximum_packet_size;
    uint16_t topic_alias_maximum;
    uint8_t request_response_information;
    uint8_t request_problem_information;
    string_pair *user_properties;
    uint8_t *authentication_method;
    uint8_t *authentication_data;
    uint8_t *client_identifier;
    uint32_t will_property_length;
    uint32_t will_delay_interval;
    uint8_t payload_format_indicator;
    uint32_t message_expiry_interval;
    uint8_t *content_type;
    uint8_t *response_topic;
    uint8_t *correlation_data;
    string_pair *will_user_properties;
    uint8_t *will_topic;
    uint8_t *will_payload;
    uint8_t *user_name;
    uint8_t *password;
};

#ifdef __cplusplus
}
#endif

#endif // CONNECT_INTERNAL_H
