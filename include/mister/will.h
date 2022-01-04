#ifndef WILL_H
#define WILL_H

typedef struct mr_will_data {
    bool will_flag;
    uint8_t will_qos;
    bool will_retain;
    uint32_t will_property_length;
    uint32_t will_delay_interval;
    uint8_t payload_format_indicator;
    uint32_t message_expiry_interval;
    uint8_t *content_type;
    size_t content_type_len;
    uint8_t *response_topic;
    size_t response_topic_len;
    uint8_t *correlation_data;
    size_t correlation_data_len;
    string_pair *will_user_properties;
    size_t will_user_properties_len;
    uint8_t *will_topic;
    size_t will_topic_len;
    uint8_t *will_payload;
    size_t will_payload_len;
} mr_will_data;

int mr_clear_will_data(mr_will_data *pwd);

int mr_set_will_values(packet_ctx *pctx, mr_will_data *pwd);
int mr_reset_will_values(packet_ctx *pctx);
int mr_get_will_data_from_values(packet_ctx *pctx, mr_will_data *pwd);
int mr_validate_will_values(packet_ctx *pctx);

#endif /* WILL_H */