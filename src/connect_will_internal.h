#ifndef WILL_INTERNAL_H
#define WILL_INTERNAL_H

static int mr_validate_will_qos(mr_connect_will_data *pwd);
static int mr_validate_will_retain(mr_connect_will_data *pwd);
static int mr_validate_will_delay_interval(mr_connect_will_data *pwd);
static int mr_validate_payload_format_indicator(mr_connect_will_data *pwd);
static int mr_validate_message_expiry_interval(mr_connect_will_data *pwd);

static int mr_validate_content_type(mr_connect_will_data *pwd);
static int mr_validate_response_topic(mr_connect_will_data *pwd);
static int mr_validate_correlation_data(mr_connect_will_data *pwd);
static int mr_validate_will_user_properties(mr_connect_will_data *pwd);
static int mr_validate_will_topic(mr_connect_will_data *pwd);
static int mr_validate_will_payload(mr_connect_will_data *pwd);

#endif /* WILL_INTERNAL_H */