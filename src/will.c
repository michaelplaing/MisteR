/* will.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mister/mister.h"
#include "mister/mrzlog.h"
#include "packet_internal.h"
#include "connect_internal.h"
#include "will_internal.h"
#include "util_internal.h"

int mr_set_connect_will_values(packet_ctx *pctx, mr_connect_will_data *pwd) {
    if (mr_set_scalar(pctx, CONNECT_WILL_FLAG, pwd->will_flag)) { // should be true
        mr_reset_connect_will_values(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_WILL_QOS, pwd->will_qos))  {
        mr_reset_connect_will_values(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_WILL_RETAIN, pwd->will_retain))  {
        mr_reset_connect_will_values(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_WILL_PROPERTY_LENGTH, 0)) { // sets vexists = true
        mr_reset_connect_will_values(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_WILL_DELAY_INTERVAL, pwd->will_delay_interval)) {
        mr_reset_connect_will_values(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, pwd->payload_format_indicator)) {
        mr_reset_connect_will_values(pctx);
        return -1;
    }
    if (mr_set_scalar(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, pwd->message_expiry_interval)) {
        mr_reset_connect_will_values(pctx);
        return -1;
    }

    if (mr_set_vector(pctx, CONNECT_CONTENT_TYPE, pwd->content_type, strlen(pwd->content_type) + 1)) {
        mr_reset_connect_will_values(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_RESPONSE_TOPIC, pwd->response_topic, strlen(pwd->response_topic) + 1)) {
        mr_reset_connect_will_values(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_CORRELATION_DATA, pwd->correlation_data, pwd->correlation_data_len)) {
        mr_reset_connect_will_values(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_WILL_USER_PROPERTIES, pwd->will_user_properties, pwd->will_user_properties_len)) {
        mr_reset_connect_will_values(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_WILL_TOPIC, pwd->will_topic, strlen(pwd->will_topic) + 1)) {
        mr_reset_connect_will_values(pctx);
        return -1;
    }
    if (mr_set_vector(pctx, CONNECT_WILL_PAYLOAD, pwd->will_payload, pwd->will_payload_len)) {
        mr_reset_connect_will_values(pctx);
        return -1;
    }

    if (mr_validate_connect_will_values(pctx)) {
        mr_reset_connect_will_values(pctx);
        return -1;
    }

    return 0;
}

int mr_clear_connect_will_data(mr_connect_will_data *pwd) {
    pwd->will_flag = false;
    pwd->will_qos = 0;
    pwd->will_retain = false;
    pwd->will_property_length = 0;
    pwd->will_delay_interval = 0;
    pwd->payload_format_indicator = 0;
    pwd->message_expiry_interval = 0;
    pwd->content_type = NULL;
    pwd->response_topic = NULL;
    pwd->correlation_data = NULL;
    pwd->correlation_data_len = 0;
    pwd->will_user_properties = NULL;
    pwd->will_user_properties_len = 0;
    pwd->will_topic = NULL;
    pwd->will_payload = NULL;
    pwd->will_payload_len = 0;

    return 0;
}

int mr_reset_connect_will_values(packet_ctx *pctx) {
    if (mr_reset_scalar(pctx, CONNECT_WILL_FLAG)) return -1;;
    if (mr_reset_scalar(pctx, CONNECT_WILL_QOS)) return -1;
    if (mr_reset_scalar(pctx, CONNECT_WILL_RETAIN)) return -1;
    if (mr_reset_scalar(pctx, CONNECT_WILL_DELAY_INTERVAL)) return -1;
    if (mr_reset_scalar(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR)) return -1;
    if (mr_reset_scalar(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL)) return -1;

    if (mr_reset_vector(pctx, CONNECT_CONTENT_TYPE)) return -1;
    if (mr_reset_vector(pctx, CONNECT_RESPONSE_TOPIC)) return -1;
    if (mr_reset_vector(pctx, CONNECT_CORRELATION_DATA)) return -1;
    if (mr_reset_vector(pctx, CONNECT_WILL_USER_PROPERTIES)) return -1;
    if (mr_reset_vector(pctx, CONNECT_WILL_TOPIC)) return -1;
    if (mr_reset_vector(pctx, CONNECT_WILL_PAYLOAD)) return -1;

    return 0;
}

int mr_get_connect_will_values(packet_ctx *pctx, mr_connect_will_data *pwd) {
    bool exists;

    mr_get_boolean(pctx, CONNECT_WILL_FLAG, &pwd->will_flag, &exists);
    mr_get_u8(pctx, CONNECT_WILL_QOS, &pwd->will_qos, &exists);
    mr_get_boolean(pctx, CONNECT_WILL_RETAIN, &pwd->will_retain, &exists);
    mr_get_u32(pctx, CONNECT_WILL_DELAY_INTERVAL, &pwd->will_delay_interval, &exists);
    mr_get_u8(pctx, CONNECT_PAYLOAD_FORMAT_INDICATOR, &pwd->payload_format_indicator, &exists);
    mr_get_u32(pctx, CONNECT_MESSAGE_EXPIRY_INTERVAL, &pwd->message_expiry_interval, &exists);

    mr_get_str(pctx, CONNECT_CONTENT_TYPE, &pwd->content_type, &exists);
    mr_get_str(pctx, CONNECT_RESPONSE_TOPIC, &pwd->response_topic, &exists);
    mr_get_u8v(pctx, CONNECT_CORRELATION_DATA, &pwd->correlation_data, &pwd->correlation_data_len, &exists);
    mr_get_spv(pctx, CONNECT_WILL_USER_PROPERTIES, &pwd->will_user_properties, &pwd->will_user_properties_len, &exists);
    mr_get_str(pctx, CONNECT_WILL_TOPIC, &pwd->will_topic, &exists);
    mr_get_u8v(pctx, CONNECT_WILL_PAYLOAD, &pwd->will_payload, &pwd->will_payload_len, &exists);

    return 0;
}

int mr_validate_connect_will_values(packet_ctx *pctx) {
    int rc;
    mr_connect_will_data wd;
    rc = mr_get_connect_will_values(pctx, &wd); if (rc) return rc;

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

static int mr_validate_will_qos(mr_connect_will_data *pwd) {
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

static int mr_validate_will_retain(mr_connect_will_data *pwd) {
    if (!pwd->will_flag && pwd->will_retain) {
        dzlog_error("will_flag false but will_retain true");
        return -1;
    }

    return 0;
}

int mr_validate_will_delay_interval(mr_connect_will_data *pwd) {
    if (!pwd->will_flag && pwd->will_delay_interval) {
        dzlog_error("will_flag false but will_delay_interval > 0");
        return -1;
    }

    return 0;
}

static int mr_validate_payload_format_indicator(mr_connect_will_data *pwd) {
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

static int mr_validate_message_expiry_interval(mr_connect_will_data *pwd) {
    if (!pwd->will_flag && pwd->message_expiry_interval) {
        dzlog_error("will_flag false but message_expiry_interval > 0");
        return -1;
    }

    return 0;
}

static int mr_validate_content_type(mr_connect_will_data *pwd) {
    if (!pwd->will_flag) {
        if (pwd->content_type) {
            dzlog_error("will_flag false but content_type is not NULL");
            return -1;
        }
        else {
            return 0;
        }
    }

    if (pwd->content_type) {
        int err_pos = utf8val((uint8_t *)pwd->content_type, strlen(pwd->content_type));

        if (err_pos) {
            dzlog_error(
                "utf8 validation failed for content_type: %s, pos: %d",
                (char *)pwd->content_type, err_pos
            );

            return -1;
        }
    }

    return 0;
}

static int mr_validate_response_topic(mr_connect_will_data *pwd) {
    if (!pwd->will_flag) {
        if (pwd->response_topic) {
            dzlog_error("will_flag false but response_topic is not NULL");
            return -1;
        }
        else {
            return 0;
        }
    }

    if (pwd->response_topic) {
        int err_pos = utf8val((uint8_t *)pwd->response_topic, strlen(pwd->response_topic));

        if (err_pos) {
            dzlog_error(
                "utf8 validation failed for response_topic: %s, pos: %d",
                (char *)pwd->response_topic, err_pos
            );

            return -1;
        }
    }

    return 0;
}

static int mr_validate_correlation_data(mr_connect_will_data *pwd) {
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

static int mr_validate_will_user_properties(mr_connect_will_data *pwd) {
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
        int err_pos = utf8val((uint8_t *)spv[i].name, strlen(spv[i].name));

        if (err_pos) {
            dzlog_error(
                "invalid utf8: string_pair: %d; name: %s; pos: %d", i, spv[i].name, err_pos
            );

            return -1;
        }

        err_pos = utf8val((uint8_t *)spv[i].value, strlen(spv[i].value));

        if (err_pos) {
            dzlog_error(
                "invalid utf8: string_pair: %d; value: %s; pos: %d", i, spv[i].value, err_pos
            );

            return -1;
        }
    }

    return 0;
}

static int mr_validate_will_topic(mr_connect_will_data *pwd) {
    if (!pwd->will_flag) {
        if (pwd->will_topic) {
            dzlog_error("will_flag false but will_topic is not NULL");
            return -1;
        }
        else {
            return 0;
        }
    }

    int err_pos = utf8val((uint8_t *)pwd->will_topic, strlen(pwd->will_topic));

    if (err_pos) {
        dzlog_error(
            "utf8 validation failed for will_topic: %s, pos: %d",
            (char *)pwd->will_topic, err_pos
        );

        return -1;
    }

    return 0;
}

static int mr_validate_will_payload(mr_connect_will_data *pwd) {
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
