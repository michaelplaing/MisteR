/* will.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mister/mister.h"
#include "packet_internal.h"
#include "connect_internal.h"
#include "mister/mrzlog.h"
#include "mister/will.h"
#include "mister/util.h"

int mr_set_will_values(packet_ctx *pctx, mr_will_data *pwd) {
    if (mr_set_scalar(pctx, CONNECT_WILL_FLAG, pwd->will_flag)) { // should be true
        mr_reset_will_values(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_WILL_QOS, pwd->will_qos))  {
        mr_reset_will_values(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_WILL_RETAIN, pwd->will_retain))  {
        mr_reset_will_values(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_WILL_DELAY_INTERVAL, pwd->will_delay_interval)) {
        mr_reset_will_values(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, pwd->payload_format_indicator)) {
        mr_reset_will_values(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, pwd->message_expiry_interval)) {
        mr_reset_will_values(pctx);
        return -1;
    }

    if (mr_set_vector(pctx, CONNECT_CONTENT_TYPE, pwd->content_type, pwd->content_type_len)) {
        mr_reset_will_values(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_RESPONSE_TOPIC, pwd->response_topic, pwd->response_topic_len)) {
        mr_reset_will_values(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_CORRELATION_DATA, pwd->correlation_data, pwd->correlation_data_len)) {
        mr_reset_will_values(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_WILL_USER_PROPERTIES, pwd->will_user_properties, pwd->will_user_properties_len)) {
        mr_reset_will_values(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_WILL_TOPIC, pwd->will_topic, pwd->will_topic_len)) {
        mr_reset_will_values(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_WILL_PAYLOAD, pwd->will_payload, pwd->will_payload_len)) {
        mr_reset_will_values(pctx);
        return -1;
    }

    if (mr_validate_will_values(pctx)) {
        mr_reset_will_values(pctx);
        return -1;
    }

    return 0;
}

int mr_reset_will_values(packet_ctx *pctx) {
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

int mr_get_will_data_from_values(packet_ctx *pctx, mr_will_data *pwd) {
    if (mr_get_boolean(pctx, CONNECT_WILL_FLAG, &pwd->will_flag))
        pwd->will_flag = false;
    if (mr_get_u8(pctx, CONNECT_WILL_QOS, &pwd->will_qos))
        pwd->will_qos = 0;
    if (mr_get_boolean(pctx, CONNECT_WILL_RETAIN, &pwd->will_retain))
        pwd->will_retain = false;
    if (mr_get_u32(pctx, CONNECT_WILL_DELAY_INTERVAL, &pwd->will_delay_interval))
        pwd->will_delay_interval = 0;
    if (mr_get_u8(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, &pwd->payload_format_indicator))
        pwd->payload_format_indicator = 0;
    if (mr_get_u32(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, &pwd->message_expiry_interval))
        pwd->message_expiry_interval = 0;

    if (mr_get_u8v(pctx, CONNECT_CONTENT_TYPE, &pwd->content_type, &pwd->content_type_len)) {
        pwd->content_type = NULL; pwd->content_type_len = 0;
    }
    if (mr_get_u8v(pctx, CONNECT_RESPONSE_TOPIC, &pwd->response_topic, &pwd->response_topic_len)) {
        pwd->response_topic = NULL; pwd->response_topic_len = 0;
    }
    if (mr_get_u8v(pctx, CONNECT_CORRELATION_DATA, &pwd->correlation_data, &pwd->correlation_data_len)) {
        pwd->correlation_data = NULL; pwd->correlation_data_len = 0;
    }
    if (mr_get_spv(pctx, CONNECT_WILL_USER_PROPERTIES, &pwd->will_user_properties, &pwd->will_user_properties_len)) {
        pwd->will_user_properties = NULL; pwd->will_user_properties_len = 0;
    }
    if (mr_get_u8v(pctx, CONNECT_WILL_TOPIC, &pwd->will_topic, &pwd->will_topic_len)) {
        pwd->will_topic = NULL; pwd->will_topic_len = 0;
    }
    if (mr_get_u8v(pctx, CONNECT_WILL_PAYLOAD, &pwd->will_payload, &pwd->will_payload_len)) {
        pwd->will_payload = NULL; pwd->will_payload_len = 0;
    }

    return 0;
}

int mr_validate_will_values(packet_ctx *pctx) {
    int rc;
    mr_will_data wd;
    rc = mr_get_will_data_from_values(pctx, &wd); if (rc) return rc;

    rc = mr_validate_will_qos(&wd); if (rc) return rc;
    rc = mr_validate_will_retain(&wd); if (rc) return rc;
    rc = mr_validate_will_delay_interval(&wd); if (rc) return rc;
    rc = mr_validate_payload_format_indicator(&wd); if (rc) return rc;
    rc = mr_validate_message_expiry_interval(&wd); if (rc) return rc;

    rc = mr_validate_content_type(&wd); if (rc) return rc;
    rc = mr_validate_response_topic(&wd); if (rc) return rc;
    rc = mr_validate_correlation_data(&wd); if (rc) return rc;
    rc = mr_validate_will_user_properties(&wd); if (rc) return rc;
    rc = mr_validate_will_topic(&wd); if (rc) return rc;
    rc = mr_validate_will_payload(&wd); if (rc) return rc;

    return 0;
}

static int mr_validate_will_qos(mr_will_data *pwd) {
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

static int mr_validate_will_retain(mr_will_data *pwd) {
    if (!pwd->will_flag && pwd->will_retain) {
        dzlog_error("will_flag false but will_retain true");
        return -1;
    }

    return 0;
}

int mr_validate_will_delay_interval(mr_will_data *pwd) {
    if (!pwd->will_flag && pwd->will_delay_interval) {
        dzlog_error("will_flag false but will_delay_interval > 0");
        return -1;
    }

    return 0;
}

static int mr_validate_payload_format_indicator(mr_will_data *pwd) {
    if (!pwd->will_flag && pwd->payload_format_indicator) {
        dzlog_error("will_flag false but payload_format_indicator > 0");
        return -1;
    }

    if (pwd->payload_format_indicator > 1) {
        dzlog_error("payload_format_indicator must be 0 or 1: %u", pwd->payload_format_indicator);
        return -1;
    }

    return 0;
}

static int mr_validate_message_expiry_interval(mr_will_data *pwd) {
    if (!pwd->will_flag && pwd->message_expiry_interval) {
        dzlog_error("will_flag false but message_expiry_interval > 0");
        return -1;
    }

    return 0;
}

static int mr_validate_content_type(mr_will_data *pwd) {
    if (!pwd->will_flag) {
        if (pwd->content_type || pwd->content_type_len) {
            dzlog_error("will_flag false but content_type is not NULL or its len > 0");
            return -1;
        }
        else {
            return 0;
        }
    }

    if (!pwd->content_type && pwd->content_type_len) { // mr_set_vector makes len 0 if pointer is NULL
        dzlog_error("content_type is NULL but its len > 0");
        return -1;
    }

    if (pwd->content_type) {
        int err_pos = utf8val(pwd->content_type, pwd->content_type_len);

        if (err_pos) {
            dzlog_error(
                "utf8 validation failed for content_type: %.*s, pos: %d",
                (int)pwd->content_type_len, (char *)pwd->content_type, err_pos
            );

            return -1;
        }
    }

    return 0;
}

static int mr_validate_response_topic(mr_will_data *pwd) {
    if (!pwd->will_flag) {
        if (pwd->response_topic || pwd->response_topic_len) {
            dzlog_error("will_flag false but response_topic is not NULL or its len > 0");
            return -1;
        }
        else {
            return 0;
        }
    }

    if (!pwd->response_topic && pwd->response_topic_len) { // mr_set_vector makes len 0 if pointer is NULL
        dzlog_error("response_topic is NULL but its len > 0");
        return -1;
    }

    if (pwd->response_topic) {
        int err_pos = utf8val(pwd->response_topic, pwd->response_topic_len);

        if (err_pos) {
            dzlog_error(
                "utf8 validation failed for response_topic: %.*s, pos: %d",
                (int)pwd->response_topic_len, (char *)pwd->response_topic, err_pos
            );

            return -1;
        }
    }

    return 0;
}

static int mr_validate_correlation_data(mr_will_data *pwd) {
    if (!pwd->will_flag) {
        if (pwd->correlation_data || pwd->correlation_data_len) {
            dzlog_error("will_flag false but correlation_data is not NULL or its len > 0");
            return -1;
        }
        else {
            return 0;
        }
    }

    if (!pwd->correlation_data && pwd->correlation_data_len) {
        dzlog_error("correlation_data is NULL but its len > 0"); // mr_set_vector makes len 0 if pointer is NULL
        return -1;
    }

    return 0;
}

static int mr_validate_will_user_properties(mr_will_data *pwd) {
    if (!pwd->will_flag) {
        if (pwd->will_user_properties || pwd->will_user_properties_len) {
            dzlog_error("will_flag false but will_user_properties is not NULL or its len > 0");
            return -1;
        }
        else {
            return 0;
        }
    }

    if (!pwd->will_user_properties && pwd->will_user_properties_len) {
        dzlog_error("will_user_properties is NULL but its len > 0"); // mr_set_vector makes len 0 if pointer is NULL
        return -1;
    }

    string_pair *spv = pwd->will_user_properties;

    for (int i = 0; i < pwd->will_user_properties_len; i++) {
        int err_pos = utf8val(spv[i].name, spv[i].nlen);

        if (err_pos) {
            dzlog_error(
                "invalid utf8: string_pair: %d; name: %.*s; pos: %d",
                i, spv[i].nlen, spv[i].name, err_pos
            );

            return -1;
        }

        err_pos = utf8val(spv[i].value, spv[i].vlen);

        if (err_pos) {
            dzlog_error(
                "invalid utf8: string_pair: %d; value: %.*s; pos: %d",
                i, spv[i].nlen, spv[i].value, err_pos
            );

            return -1;
        }
    }

    return 0;
}

static int mr_validate_will_topic(mr_will_data *pwd) {
    if (!pwd->will_flag) {
        if (pwd->will_topic || pwd->will_topic_len) {
            dzlog_error("will_flag false but will_topic is not NULL or its len > 0");
            return -1;
        }
        else {
            return 0;
        }
    }

    if (!(pwd->will_topic && pwd->will_topic_len)) {
        dzlog_error("will_topic is missing and must have len > 0");
        return -1;
    }

    int err_pos = utf8val(pwd->will_topic, pwd->will_topic_len);

    if (err_pos) {
        dzlog_error(
            "utf8 validation failed for will_topic: %.*s, pos: %d",
            (int)pwd->will_topic_len, (char *)pwd->will_topic, err_pos
        );

        return -1;
    }

    return 0;
}

static int mr_validate_will_payload(mr_will_data *pwd) {
    if (!pwd->will_flag) {
        if (pwd->will_payload || pwd->will_payload_len) {
            dzlog_error("will_flag false but will_payload is not NULL or its len > 0");
            return -1;
        }
        else {
            return 0;
        }
    }

    if (!(pwd->will_payload && pwd->will_payload_len)) {
        dzlog_error("will_payload is missing and must have len > 0");
        return -1;
    }

    if (pwd->payload_format_indicator) {
        int err_pos = utf8val(pwd->will_payload, pwd->will_payload_len);

        if (err_pos) {
            dzlog_error(
                "utf8 validation failed for will_payload: %.*s, pos: %d",
                (int)pwd->will_payload_len, (char *)pwd->will_payload, err_pos
            );

            return -1;
        }
    }

    return 0;
}
