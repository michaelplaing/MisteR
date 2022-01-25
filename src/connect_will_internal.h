#ifndef CONNECT_WILL_INTERNAL_H
#define CONNECT_WILL_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

static int mr_validate_will_qos(mr_connect_will_data *pwd);
static int mr_validate_will_retain(mr_connect_will_data *pwd);
static int mr_validate_will_property_length(mr_connect_will_data *pwd);
static int mr_validate_will_delay_interval(mr_connect_will_data *pwd);
static int mr_validate_payload_format_indicator(mr_connect_will_data *pwd);
static int mr_validate_message_expiry_interval(mr_connect_will_data *pwd);

static int mr_validate_content_type(mr_connect_will_data *pwd);
static int mr_validate_content_type_utf8(mr_connect_will_data *pwd);
static int mr_validate_response_topic(mr_connect_will_data *pwd);
static int mr_validate_response_topic_utf8(mr_connect_will_data *pwd);
static int mr_validate_correlation_data(mr_connect_will_data *pwd);
static int mr_validate_will_user_properties(mr_connect_will_data *pwd);
static int mr_validate_will_user_properties_utf8(mr_connect_will_data *pwd);
static int mr_validate_will_topic(mr_connect_will_data *pwd);
static int mr_validate_will_topic_utf8(mr_connect_will_data *pwd);
static int mr_validate_will_payload(mr_connect_will_data *pwd);
static int mr_validate_will_payload_utf8(mr_connect_will_data *pwd);

#ifdef __cplusplus
}
#endif

#endif /* CONNECT_WILL_INTERNAL_H */