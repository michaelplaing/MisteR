#ifndef CONNECT_H
#define CONNECT_H

#include "mister/packet.h"

typedef struct connect_packet {
    uint8_t packet_type;
    uint32_t remaining_length;
    uint8_t *protocol_name;
    uint8_t protocol_version;
    bool reserved;
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
} connect_packet;

packet_ctx *init_connect_pctx(void);

int pack_connect_buffer(packet_ctx *pctx);
int unpack_connect_buffer(packet_ctx *pctx);

int free_connect_pctx(packet_ctx *pctx);

int get_connect_packet_type(packet_ctx *pctx, uint8_t *Pptype);

int get_connect_remaining_length(packet_ctx *pctx, uint32_t *Premlen);

int set_connect_clean_start(packet_ctx *pctx, bool value);
int get_connect_clean_start(packet_ctx *pctx, bool *flag);

int set_connect_user_properties(packet_ctx *pctx, string_pair *sp0, size_t len);
int get_connect_user_properties(packet_ctx *pctx, string_pair **Psp0, size_t *Plen);

int set_connect_authentication_data(packet_ctx *pctx, uint8_t *authdata, size_t len);
int get_connect_authentication_data(packet_ctx *pctx, uint8_t **Pauthdata, size_t *Plen);

#endif /* CONNECT_H */
