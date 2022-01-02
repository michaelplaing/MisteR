#ifndef WILL_H
#define WILL_H

#include <stdlib.h>
#include <stdbool.h>

#include "mister/connect.h"
// #include "packet_internal.h"

int mr_set_will(packet_ctx *pctx, mr_will_data *pwd);
int mr_reset_will(packet_ctx *pctx);
int mr_get_will(packet_ctx *pctx, mr_will_data *pwd);
int mr_validate_will(packet_ctx *pctx);

static int mr_validate_will_qos(mr_will_data *pwd);
static int mr_validate_will_retain(mr_will_data *pwd);
static int mr_validate_will_delay_interval(mr_will_data *pwd);
static int mr_validate_payload_format_indicator(mr_will_data *pwd);
static int mr_validate_message_expiry_interval(mr_will_data *pwd);

static int mr_validate_content_type(mr_will_data *pwd);
static int mr_validate_response_topic(mr_will_data *pwd);
static int mr_validate_correlation_data(mr_will_data *pwd);
static int mr_validate_will_user_properties(mr_will_data *pwd);
static int mr_validate_will_topic(mr_will_data *pwd);
static int mr_validate_will_payload(mr_will_data *pwd);

#endif /* WILL_H */