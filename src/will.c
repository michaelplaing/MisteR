/* will.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mister/mister.h"
#include "packet_internal.h"
#include "connect_internal.h"
#include "mister/mrzlog.h"
#include "will.h"

int mr_set_will(packet_ctx *pctx, mr_will_data *pwd) {
    if (mr_set_scalar(pctx, CONNECT_WILL_FLAG, pwd->will_flag)) { // should be true
        mr_reset_will(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_WILL_QOS, pwd->will_qos))  {
        mr_reset_will(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_WILL_RETAIN, pwd->will_retain))  {
        mr_reset_will(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_WILL_DELAY_INTERVAL, pwd->will_delay_interval)) {
        mr_reset_will(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, pwd->payload_format_indicator)) {
        mr_reset_will(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, pwd->message_expiry_interval)) {
        mr_reset_will(pctx);
        return -1;
    }

    if (mr_set_vector(pctx, CONNECT_CONTENT_TYPE, pwd->content_type, pwd->content_type_len)) {
        mr_reset_will(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_RESPONSE_TOPIC, pwd->response_topic, pwd->response_topic_len)) {
        mr_reset_will(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_CORRELATION_DATA, pwd->correlation_data, pwd->correlation_data_len)) {
        mr_reset_will(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_WILL_USER_PROPERTIES, pwd->will_user_properties, pwd->will_user_properties_len)) {
        mr_reset_will(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_WILL_TOPIC, pwd->will_topic, pwd->will_topic_len)) {
        mr_reset_will(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_WILL_PAYLOAD, pwd->will_payload, pwd->will_payload_len)) {
        mr_reset_will(pctx);
        return -1;
    }

    return mr_validate_will(pctx);
}

int mr_reset_will(packet_ctx *pctx) {
    if (mr_reset_value(pctx, CONNECT_WILL_FLAG)) return -1;;
    if (mr_reset_value(pctx, CONNECT_WILL_QOS)) return -1;
    if (mr_reset_value(pctx, CONNECT_WILL_RETAIN)) return -1;
    if (mr_reset_value(pctx, CONNECT_WILL_DELAY_INTERVAL)) return -1;
    if (mr_reset_value(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR)) return -1;
    if (mr_reset_value(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL)) return -1;

    if (mr_reset_value(pctx, CONNECT_CONTENT_TYPE)) return -1;
    if (mr_reset_value(pctx, CONNECT_RESPONSE_TOPIC)) return -1;
    if (mr_reset_value(pctx, CONNECT_CORRELATION_DATA)) return -1;
    if (mr_reset_value(pctx, CONNECT_WILL_USER_PROPERTIES)) return -1;
    if (mr_reset_value(pctx, CONNECT_WILL_TOPIC)) return -1;
    if (mr_reset_value(pctx, CONNECT_WILL_PAYLOAD)) return -1;

    return 0;
}

int mr_get_will(packet_ctx *pctx, mr_will_data *pwd) {
    if (mr_get_boolean(pctx, CONNECT_WILL_FLAG, &pwd->will_flag)) return -1;
    if (mr_get_u8(pctx, CONNECT_WILL_QOS, &pwd->will_qos)) return -1;
    if (mr_get_boolean(pctx, CONNECT_WILL_RETAIN, &pwd->will_retain)) return -1;
    if (mr_get_u32(pctx, CONNECT_WILL_DELAY_INTERVAL, &pwd->will_delay_interval)) return -1;
    if (mr_get_u8(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, &pwd->payload_format_indicator)) return -1;
    if (mr_get_u32(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, &pwd->message_expiry_interval)) return -1;

    if (mr_get_u8v(pctx, CONNECT_CONTENT_TYPE, &pwd->content_type, &pwd->content_type_len)) return -1;
    if (mr_get_u8v(pctx, CONNECT_RESPONSE_TOPIC, &pwd->response_topic, &pwd->response_topic_len)) return -1;
    if (mr_get_u8v(pctx, CONNECT_CORRELATION_DATA, &pwd->correlation_data, &pwd->correlation_data_len)) return -1;
    if (mr_get_spv(pctx, CONNECT_WILL_USER_PROPERTIES, &pwd->will_user_properties, &pwd->will_user_properties_len)) return -1;
    if (mr_get_u8v(pctx, CONNECT_WILL_TOPIC, &pwd->will_topic, &pwd->will_topic_len)) return -1;
    if (mr_get_u8v(pctx, CONNECT_WILL_PAYLOAD, &pwd->will_payload, &pwd->will_payload_len)) return -1;

    return 0;
}

int mr_validate_will(packet_ctx *pctx) {
    int rc;
    mr_will_data wd;
    rc = mr_get_will(pctx, &wd); if (rc) return rc;

    rc = mr_validate_will_qos(pctx, &wd); if (rc) return rc;
    rc = mr_validate_will_retain(pctx, &wd); if (rc) return rc;
    rc = mr_validate_will_delay_interval(pctx, &wd); if (rc) return rc;
    rc = mr_validate_payload_format_indicator(pctx, &wd); if (rc) return rc;
    rc = mr_validate_message_expiry_interval(pctx, &wd); if (rc) return rc;

    rc = mr_validate_content_type(pctx, &wd); if (rc) return rc;
    rc = mr_validate_response_topic(pctx, &wd); if (rc) return rc;
    rc = mr_validate_correlation_data(pctx, &wd); if (rc) return rc;
    rc = mr_validate_will_user_properties(pctx, &wd); if (rc) return rc;
    rc = mr_validate_will_topic(pctx, &wd); if (rc) return rc;
    rc = mr_validate_will_payload(pctx, &wd); if (rc) return rc;

    return 0;
}

static int mr_validate_will_qos(packet_ctx *pctx, mr_will_data *pwd) {
    if (!pwd->will_flag && pwd->will_qos) {
        dzlog_error("will_flag false but will_qos set: %u", pwd->will_qos);
        return -1;
    }

    if (pwd->will_qos > 2) {
        dzlog_error("will_qos out of range (0..2): %u", pwd->will_qos);
        return -1;
    }

    return 0;
}

static int mr_validate_will_retain(packet_ctx *pctx, mr_will_data *pwd) {
    if (!pwd->will_flag && pwd->will_retain) {
        dzlog_error("will_flag false but will_retain true");
        return -1;
    }

    return 0;
}

int mr_validate_will_delay_interval(packet_ctx *pctx, mr_will_data *pwd) {
    return 0;
}

static int mr_validate_payload_format_indicator(packet_ctx *pctx, mr_will_data *pwd) {
    if (pwd->payload_format_indicator > 1) {
        dzlog_error("payload_format_indicator must be 0 or 1: %u", pwd->payload_format_indicator);
        return -1;
    }

    return 0;
}

static int mr_validate_message_expiry_interval(packet_ctx *pctx, mr_will_data *pwd) {
    return 0;
}

static int mr_validate_content_type(packet_ctx *pctx, mr_will_data *pwd) {
    return 0; // if exists, already validated as utf8 on set and unpack
}

static int mr_validate_response_topic(packet_ctx *pctx, mr_will_data *pwd) {
    return 0; // if exists, already validated as utf8 on set and unpack
}

static int mr_validate_correlation_data(packet_ctx *pctx, mr_will_data *pwd) {
    return 0;
}

static int mr_validate_will_user_properties(packet_ctx *pctx, mr_will_data *pwd) {
    return 0;
}

static int mr_validate_will_topic(packet_ctx *pctx, mr_will_data *pwd) {
    if (!pwd->will_topic || !pwd->will_topic_len) {
        dzlog_error("will_topic is missing and must be present");
        return -1;
    }

    return 0;
}

static int mr_validate_will_payload(packet_ctx *pctx, mr_will_data *pwd) {
    return 0;
}
