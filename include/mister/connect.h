#ifndef CONNECT_H
#define CONNECT_H

#include "mister/packet.h"

int mr_init_unpack_connect_pctx(packet_ctx **ppctx, uint8_t *u8v0, size_t ulen);
int mr_init_connect_pctx(packet_ctx **ppctx);
int mr_pack_connect_u8v0(packet_ctx *pctx);
// int mr_unpack_connect_u8v0(packet_ctx *pctx); use 'mr_init_unpack_connect_pctx'
int mr_free_connect_pctx(packet_ctx *pctx);

int mr_get_connect_packet_type(packet_ctx *pctx, uint8_t *pu8);
int mr_get_connect_remaining_length(packet_ctx *pctx, uint32_t *pu32);
int mr_get_connect_protocol_name(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);
int mr_get_connect_protocol_version(packet_ctx *pctx, uint8_t *pu8);
int mr_get_connect_reserved(packet_ctx *pctx, bool *pboolean);

int mr_set_connect_clean_start(packet_ctx *pctx, bool boolean);
int mr_reset_connect_clean_start(packet_ctx *pctx);
int mr_get_connect_clean_start(packet_ctx *pctx, bool *pboolean);

int mr_get_connect_will_flag(packet_ctx *pctx, bool *pboolean);
int mr_get_connect_will_qos(packet_ctx *pctx, uint8_t *pu8);
int mr_get_connect_will_retain(packet_ctx *pctx, bool *pboolean);
int mr_get_connect_password_flag(packet_ctx *pctx, bool *pboolean);
int mr_get_connect_username_flag(packet_ctx *pctx, bool *pboolean);

int mr_set_connect_keep_alive(packet_ctx *pctx, uint16_t u16);
int mr_reset_connect_keep_alive(packet_ctx *pctx);
int mr_get_connect_keep_alive(packet_ctx *pctx, uint16_t *pu16);

int mr_get_connect_property_length(packet_ctx *pctx, uint32_t *pu32);

int mr_set_connect_session_expiry_interval(packet_ctx *pctx, uint32_t u32);
int mr_reset_connect_session_expiry_interval(packet_ctx *pctx);
int mr_get_connect_session_expiry_interval(packet_ctx *pctx, uint32_t *pu32);

int mr_set_connect_receive_maximum(packet_ctx *pctx, uint16_t u16);
int mr_reset_connect_receive_maximum(packet_ctx *pctx);
int mr_get_connect_receive_maximum(packet_ctx *pctx, uint16_t *pu16);

int mr_set_connect_maximum_packet_size(packet_ctx *pctx, uint32_t u32);
int mr_reset_connect_maximum_packet_size(packet_ctx *pctx);
int mr_get_connect_maximum_packet_size(packet_ctx *pctx, uint32_t *pu32);

int mr_set_connect_topic_alias_maximum(packet_ctx *pctx, uint16_t u16);
int mr_reset_connect_topic_alias_maximum(packet_ctx *pctx);
int mr_get_connect_topic_alias_maximum(packet_ctx *pctx, uint16_t *pu16);

int mr_set_connect_request_response_information(packet_ctx *pctx, uint8_t u8);
int mr_reset_connect_request_response_information(packet_ctx *pctx);
int mr_get_connect_request_response_information(packet_ctx *pctx, uint8_t *pu8);

int mr_set_connect_request_problem_information(packet_ctx *pctx, uint8_t u8);
int mr_reset_connect_request_problem_information(packet_ctx *pctx);
int mr_get_connect_request_problem_information(packet_ctx *pctx, uint8_t *pu8);

int mr_set_connect_user_properties(packet_ctx *pctx, string_pair *spv0, size_t len);
int mr_reset_connect_user_properties(packet_ctx *pctx);
int mr_get_connect_user_properties(packet_ctx *pctx, string_pair **pspv0, size_t *plen);

int mr_set_connect_authentication_method(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int mr_reset_connect_authentication_method(packet_ctx *pctx);
int mr_get_connect_authentication_method(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int mr_set_connect_authentication_data(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int mr_reset_connect_authentication_data(packet_ctx *pctx);
int mr_get_connect_authentication_data(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int mr_set_connect_client_identifier(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int mr_reset_connect_client_identifier(packet_ctx *pctx);
int mr_get_connect_client_identifier(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int mr_get_connect_will_property_length(packet_ctx *pctx, uint32_t *pu32);
int mr_get_connect_will_delay_interval(packet_ctx *pctx, uint32_t *pu32);
int mr_get_connect_payload_format_indicator(packet_ctx *pctx, uint8_t *pu8);
int mr_get_connect_message_expiry_interval(packet_ctx *pctx, uint32_t *pu32);
int mr_get_connect_content_type(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);
int mr_get_connect_response_topic(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);
int mr_get_connect_correlation_data(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);
int mr_get_connect_will_user_properties(packet_ctx *pctx, string_pair **pspv0, size_t *plen);
int mr_get_connect_will_topic(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);
int mr_get_connect_will_payload(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int mr_set_connect_user_name(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int mr_reset_connect_user_name(packet_ctx *pctx);
int mr_get_connect_user_name(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

int mr_set_connect_password(packet_ctx *pctx, uint8_t *u8v0, size_t len);
int mr_reset_connect_password(packet_ctx *pctx);
int mr_get_connect_password(packet_ctx *pctx, uint8_t **pu8v0, size_t *plen);

#endif /* CONNECT_H */
