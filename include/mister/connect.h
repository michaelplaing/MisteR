#ifndef CONNECT_H
#define CONNECT_H

#include "mister/packet.h"

int init_connect_pctx(packet_ctx **ppctx);
int pack_connect_buffer(packet_ctx *pctx);
int unpack_connect_buffer(packet_ctx *pctx);
int free_connect_pctx(packet_ctx *pctx);

int get_connect_packet_type(packet_ctx *pctx, uint8_t *pu8);
int get_connect_remaining_length(packet_ctx *pctx, uint32_t *pu32);
int get_connect_protocol_name(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);
int get_connect_protocol_version(packet_ctx *pctx, uint8_t *pu8);
int get_connect_reserved(packet_ctx *pctx, bool *pboolean);

int set_connect_clean_start(packet_ctx *pctx, bool boolean);
int reset_connect_clean_start(packet_ctx *pctx);
int get_connect_clean_start(packet_ctx *pctx, bool *pboolean);

int get_connect_will_flag(packet_ctx *pctx, bool *pboolean);

int set_connect_will_qos(packet_ctx *pctx, uint8_t u8);
int reset_connect_will_qos(packet_ctx *pctx);
int get_connect_will_qos(packet_ctx *pctx, uint8_t *pu8);

int set_connect_will_retain(packet_ctx *pctx, bool boolean);
int reset_connect_will_retain(packet_ctx *pctx);
int get_connect_will_retain(packet_ctx *pctx, bool *pboolean);

int set_connect_password_flag(packet_ctx *pctx, bool boolean);
int reset_connect_password_flag(packet_ctx *pctx);
int get_connect_password_flag(packet_ctx *pctx, bool *pboolean);

int set_connect_username_flag(packet_ctx *pctx, bool boolean);
int reset_connect_username_flag(packet_ctx *pctx);
int get_connect_username_flag(packet_ctx *pctx, bool *pboolean);

int set_connect_keep_alive(packet_ctx *pctx, uint16_t u16);
int reset_connect_keep_alive(packet_ctx *pctx);
int get_connect_keep_alive(packet_ctx *pctx, uint16_t *pu16);

int get_connect_property_length(packet_ctx *pctx, uint32_t *pu32);

int set_connect_session_expiry(packet_ctx *pctx, uint32_t u32);
int reset_connect_session_expiry(packet_ctx *pctx);
int get_connect_session_expiry(packet_ctx *pctx, uint32_t *pu32);

int set_connect_receive_maximum(packet_ctx *pctx, uint16_t u16);
int reset_connect_receive_maximum(packet_ctx *pctx);
int get_connect_receive_maximum(packet_ctx *pctx, uint16_t *pu16);

int set_connect_maximum_packet_size(packet_ctx *pctx, uint32_t u32);
int reset_connect_maximum_packet_size(packet_ctx *pctx);
int get_connect_maximum_packet_size(packet_ctx *pctx, uint32_t *pu32);

int set_connect_topic_alias_maximum(packet_ctx *pctx, uint16_t u16);
int reset_connect_topic_alias_maximum(packet_ctx *pctx);
int get_connect_topic_alias_maximum(packet_ctx *pctx, uint16_t *pu16);

int set_connect_request_response_information(packet_ctx *pctx, uint8_t u8);
int reset_connect_request_response_information(packet_ctx *pctx);
int get_connect_request_response_information(packet_ctx *pctx, uint8_t *pu8);

int set_connect_request_problem_information(packet_ctx *pctx, uint8_t u8);
int reset_connect_request_problem_information(packet_ctx *pctx);
int get_connect_request_problem_information(packet_ctx *pctx, uint8_t *pu8);

int set_connect_user_properties(packet_ctx *pctx, string_pair *spv0, size_t len);
int reset_connect_user_properties(packet_ctx *pctx);
int get_connect_user_properties(packet_ctx *pctx, string_pair **pspv0, size_t *plen);

int set_connect_authentication_method(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int reset_connect_authentication_method(packet_ctx *pctx);
int get_connect_authentication_method(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int set_connect_authentication_data(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int reset_connect_authentication_data(packet_ctx *pctx);
int get_connect_authentication_data(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int set_connect_client_identifier(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int reset_connect_client_identifier(packet_ctx *pctx);
int get_connect_client_identifier(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int get_will_property_length(packet_ctx *pctx, uint32_t *pu32);

int set_connect_will_delay_interval(packet_ctx *pctx, uint32_t u32);
int reset_connect_will_delay_interval(packet_ctx *pctx);
int get_connect_will_delay_interval(packet_ctx *pctx, uint32_t *pu32);

int set_connect_payload_format_indicator(packet_ctx *pctx, uint8_t u8);
int reset_connect_payload_format_indicator(packet_ctx *pctx);
int get_connect_payload_format_indicator(packet_ctx *pctx, uint8_t *pu8);

int set_connect_message_expiry_interval(packet_ctx *pctx, uint32_t u32);
int reset_connect_message_expiry_interval(packet_ctx *pctx);
int get_connect_message_expiry_interval(packet_ctx *pctx, uint32_t *pu32);

int set_connect_content_type(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int reset_connect_content_type(packet_ctx *pctx);
int get_connect_content_type(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int set_connect_response_topic(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int reset_connect_response_topic(packet_ctx *pctx);
int get_connect_response_topic(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int set_connect_correlation_data(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int reset_connect_correlation_data(packet_ctx *pctx);
int get_connect_correlation_data(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int set_connect_will_user_properties(packet_ctx *pctx, string_pair *spv0, size_t len);
int reset_connect_will_user_properties(packet_ctx *pctx);
int get_connect_will_user_properties(packet_ctx *pctx, string_pair **pspv0, size_t *plen);

int set_connect_will_topic(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int reset_connect_will_topic(packet_ctx *pctx);
int get_connect_will_topic(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int set_connect_will_payload(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int reset_connect_will_payload(packet_ctx *pctx);
int get_connect_will_payload(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int set_connect_user_name(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int reset_connect_user_name(packet_ctx *pctx);
int get_connect_user_name(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int set_connect_password(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int reset_connect_password(packet_ctx *pctx);
int get_connect_password(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

typedef struct connect_values { // may or may not be useful
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
} connect_values;

#endif /* CONNECT_H */
