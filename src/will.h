#ifndef WILL_H
#define WILL_H

#include <stdlib.h>
#include <stdbool.h>

#include "mister/connect.h"
#include "packet_internal.h"

int mr_set_will(packet_ctx *pctx, mr_will_data *pwd);
int mr_reset_will(packet_ctx *pctx);
int mr_get_will(packet_ctx *pctx, mr_will_data *pwd);
int mr_validate_will(packet_ctx *pctx);

static int mr_validate_will_qos(packet_ctx *pctx, mr_will_data *pwd);
static int mr_validate_will_retain(packet_ctx *pctx, mr_will_data *pwd);
static int mr_validate_will_delay_interval(packet_ctx *pctx, mr_will_data *pwd);
static int mr_validate_payload_format_indicator(packet_ctx *pctx, mr_will_data *pwd);
static int mr_validate_message_expiry_interval(packet_ctx *pctx, mr_will_data *pwd);

static int mr_validate_content_type(packet_ctx *pctx, mr_will_data *pwd);
static int mr_validate_response_topic(packet_ctx *pctx, mr_will_data *pwd);
static int mr_validate_correlation_data(packet_ctx *pctx, mr_will_data *pwd);
static int mr_validate_will_user_properties(packet_ctx *pctx, mr_will_data *pwd);
static int mr_validate_will_topic(packet_ctx *pctx, mr_will_data *pwd);
static int mr_validate_will_payload(packet_ctx *pctx, mr_will_data *pwd);

#endif /* WILL_H */