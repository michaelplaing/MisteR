#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <uuid/uuid.h>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libev.h>

#include "redismodule.h"
#include "mister.h"
#include "pack.h"

void mrPingreqCallback(redisAsyncContext *rctx, void *reply_void, void *private_data_void) {
    REDISMODULE_NOT_USED(rctx);
    redisReply *reply = reply_void;
    uuid_t *corrid = private_data_void;
    size_t i;
    char corrid_str[UUID_STR_LEN];
    uuid_unparse(*corrid, corrid_str);
    free(corrid);

    if (reply == NULL) return;
    printf("Callback - expecting PINGRESP (%s)\n", MR_PINGRESP);
    printf("  private data rcvd (corrid): %s\n", corrid_str);
    printf("  %lu elements in reply array\n", reply->elements);
    redisReply *ele0 = reply->element[0];
    printf("    %s - MR_PINGRESP ", ele0->str);

    if (strcmp(ele0->str, MR_PINGRESP)) {
        printf("does not match\n");
    }
    else {
        printf("matches\n");
    }

    redisReply *ele1 = reply->element[1];
    printf("    %lu chars received in buffer: ", ele1->len);

    for (i = 0; i < ele1->len; i++) {
        printf("%hhX ", ele1->str[i]);
    }

    if (ele1->str[0] == CMD_PINGRESP && ele1->str[1] == 0 && ele1->len == 2) {
        printf("\n    Confirmed that buffer is a valid PINGRESP\n");
    }
    else {
        printf("\n    Buffer is not a valid PINGRESP\n");
    }

    redisAsyncDisconnect(rctx);
    printf("Redis Disconnect sent\n");
}

void redisAsyncConnectCallback(const redisAsyncContext *rctx, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", rctx->errstr);
        return;
    }

    printf("Connected to Redis...\n");
}

void redisAsyncDisconnectCallback(const redisAsyncContext *rctx, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", rctx->errstr);
        return;
    }

    printf("Disconnected from Redis\n");
}

void mrSendPingreq(redisAsyncContext *rctx) {
    const char PINGREQ_BUF[2] = {CMD_PINGREQ, 0x00};
    size_t i;

    uuid_t *corrid = malloc(sizeof(uuid_t));
    uuid_generate_random(*corrid);
    char corrid_str[UUID_STR_LEN];
    uuid_unparse(*corrid, corrid_str);

    redisAsyncCommand(
        rctx, mrPingreqCallback, corrid,
        "%s %b", MR_PINGREQ, PINGREQ_BUF, sizeof(PINGREQ_BUF)
    );

    printf("Async Command sent PINGREQ (%s)\n", MR_PINGREQ);
    printf("  private data sent (corrid): %s\n", corrid_str);
    printf("  %lu chars sent in buffer: ", sizeof(PINGREQ_BUF));

    for (i = 0; i < sizeof(PINGREQ_BUF); i++) {
        printf("%hhX ", PINGREQ_BUF[i]);
    }

    printf("\n");
    printf("  Buffer is a valid PINGREQ\n");
}

void mrConnectCallback(redisAsyncContext *rctx, void *reply_void, void *private_data_void) {
}

void mrSendConnect(redisAsyncContext *rctx) {
    size_t i;
    Pvoid_t PJSLArray = (Pvoid_t)NULL;  // initialize JudySL array
    connect_hv hv, *chv, **Pchv;

    pack_ctx *pctx = initPackContext(1000);

    const uint8_t protocol_name[] = {0x00, 0x04, 'M', 'Q', 'T', 'T'};

connect_hv connect_hvs[] = {
/*  name, function, value, bitpos, exists, id, len */
    {"packet_type", pack_uint8, CMD_CONNECT, 0},
    {"remaining_length", pack_VBI, 0},
    {"protocol_name", pack_buffer, (Word_t)protocol_name, 0, 0, 0, sizeof(protocol_name)},
    {"protocol_version", pack_uint8, 5, 0},
    {"reserved", pack_bool_in_uint8, false, 0, 0},
    {"clean_start", pack_bool_in_uint8, false, 1, 0},
    {"will_flag", pack_bool_in_uint8, false, 2, 0},
    {"will_qos", pack_uint8_in_uint8, 0, 3, 0},
    {"will_retain", pack_bool_in_uint8, false, 5, 0},
    {"password_flag", pack_bool_in_uint8, false, 6, 0},
    {"username_flag", pack_bool_in_uint8, false, 7, 0},
    {"keep_alive", pack_uint16, 0},
    {"property_length", pack_VBI, 0},
    {"session_expiry", pack_prop_uint32, 0, 0, false, 0x11, 0}
};

/*
    uint8_t receive_maximum_id;
    uint16_t receive_maximum;
    uint8_t maximum_packet_size_id;
    uint32_t maximum_packet_size;
    uint8_t topic_alias_maximum_id;
    uint16_t topic_alias_maximum;
    uint8_t request_response_information_id;
    uint8_t request_response_information;
    uint8_t request_problem_information_id;
    uint8_t request_problem_information;
    uint8_t user_property_id;
    uint8_t *user_properties;
    uint8_t authentication_method_id;
    uint8_t *authentication_method;
    uint8_t authentication_data_id;
    uint8_t *authentication_data;
*/

    size_t hv_count = sizeof(connect_hvs) / sizeof(connect_hv);

    /*  build an associative array of name to header variable
        via a handle
    */

    for (i = 0; i < hv_count; i++) {
        JSLI(Pchv, PJSLArray, connect_hvs[i].name);
        *Pchv = &connect_hvs[i];
    }

    /*  for example, set remaining_length to 42 */
    JSLG(Pchv, PJSLArray, "remaining_length"); // get the handle
    (*Pchv)->value = 42; // reset the value for the header variable

    JSLG(Pchv, PJSLArray, "clean_start"); // set clean_start boolean
    (*Pchv)->value = true;

    JSLG(Pchv, PJSLArray, "will_qos"); // set will_qos to 3
    (*Pchv)->value = 3;

    (*Pchv)->value = 1; // reset will_qos to 1
    /*
        Values are all set so pack into a buffer using the
        packing function for each header var
    */
    for (i = 0; i < hv_count; i++) {
        chv = &connect_hvs[i];
        chv->pack_fn(pctx, chv);
    }



    printf("Connect Buf:");
    for (i = 0; i < pctx->pos; i++) printf(" %02hhX", pctx->buf[i]);
    printf("\n");

}

int main (void) {
    redisAsyncContext *rctx = redisAsyncConnect("127.0.0.1", 6379);

    if (rctx->err) {
        /* Let *rctx leak for now... */
        printf("Error: %s\n", rctx->errstr);
        return 1;
    }

    printf("Redis Connect sent\n");

    redisLibevAttach(EV_DEFAULT_ rctx);
    redisAsyncSetConnectCallback(rctx, redisAsyncConnectCallback);
    redisAsyncSetDisconnectCallback(rctx, redisAsyncDisconnectCallback);

    mrSendConnect(rctx);
    printf("MisteR CONNECT sent\n");

    mrSendPingreq(rctx);
    printf("MisteR PINGREQ sent - will terminate when PINGRESP is handled\n");

    printf("Activating event loop...\n");
    ev_loop(EV_DEFAULT_ 0);
    return 0;
}
