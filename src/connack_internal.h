#ifndef CONNACK_INTERNAL_H
#define CONNACK_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

enum CONNACK_MDATA_FIELDS { // Same order as _CONNACK_MDATA_TEMPLATE
    CONNACK_PACKET_TYPE,
    CONNACK_REMAINING_LENGTH,
    CONNACK_SESSION_PRESENT,
    CONNACK_RESERVED,
    CONNACK_MR_FLAGS,
    CONNACK_CONNECT_REASON_CODE,
    CONNACK_PROPERTY_LENGTH,
    CONNACK_MR_PROPERTIES,
    CONNACK_SESSION_EXPIRY_INTERVAL,
    CONNACK_RECEIVE_MAXIMUM,
    CONNACK_MAXIMUM_QOS,
    CONNACK_RETAIN_AVAILABLE,
    CONNACK_MAXIMUM_PACKET_SIZE,
    CONNACK_ASSIGNED_CLIENT_IDENTIFIER,
    CONNACK_TOPIC_ALIAS_MAXIMUM,
    CONNACK_REASON_STRING,
    CONNACK_USER_PROPERTIES,
    CONNACK_WILDCARD_SUBSCRIPTION_AVAILABLE,
    CONNACK_SUBSCRIPTION_IDENTIFIERS_AVAILABLE,
    CONNACK_SHARED_SUBSCRIPTION_AVAILABLE,
    CONNACK_SERVER_KEEP_ALIVE,
    CONNACK_RESPONSE_INFORMATION,
    CONNACK_SERVER_REFERENCE,
    CONNACK_AUTHENTICATION_METHOD,
    CONNACK_AUTHENTICATION_DATA,
};

struct mr_connack_values { // may or may not be useful
    const uint8_t packet_type;
    uint32_t remaining_length;
    bool session_present;
    uint8_t reserved; // bits 1-7
    uint8_t connect_reason_code;
    uint32_t property_length;
    uint32_t session_expiry_interval;
    uint16_t receive_maximum;
    uint8_t maximum_qos;
    uint8_t retain_available;
    uint32_t maximum_packet_size;
    uint8_t *assigned_client_identifier;
    uint16_t topic_alias_maximum;
    uint8_t *reason_string;
    mr_string_pair *user_properties;
    uint8_t wildcard_subscription_available;
    uint8_t subscription_identifiers_available;
    uint8_t shared_subscription_available;
    uint16_t server_keep_alive;
    uint8_t *response_information;
    uint8_t *server_reference;
    uint8_t *authentication_method;
    uint8_t *authentication_data;
};

#ifdef __cplusplus
}
#endif

#endif // CONNACK_INTERNAL_H
