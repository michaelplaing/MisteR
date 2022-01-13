#ifndef CONNACK_H
#define CONNACK_H

#include "mister/packet.h"

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

#endif /* CONNACK_H */
