#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libuv.h>

#include "mister/redismodule.h"
#include "mister/mister.h"
#include "mister/connect.h"

void mrPingreqCallback(redisAsyncContext *rctx, void *reply_void, void *private_data_void) {
    REDISMODULE_NOT_USED(rctx);
    redisReply *reply = reply_void;
    size_t i;

    if (reply == NULL) return;
    printf("Callback - expecting PINGRESP (%s)\n", MR_PINGRESP);
    printf("  %lu elements in reply array\n", reply->elements);
    redisReply *reply0 = reply->element[0];
    printf("    %s - MR_PINGRESP ", reply0->str);

    if (strcmp(reply0->str, MR_PINGRESP)) {
        printf("does not match\n");
    }
    else {
        printf("matches\n");
    }

    redisReply *reply1 = reply->element[1];
    printf("    %lu chars received in buffer: ", reply1->len);

    for (i = 0; i < reply1->len; i++) {
        printf("%hhX ", reply1->str[i]);
    }

    if (
        (uint8_t)reply1->str[0] == CMD_PINGRESP
        && (uint8_t)reply1->str[1] == 0
        && reply1->len == 2
    ) {
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
    uint8_t PINGREQ_BUF[2] = {CMD_PINGREQ, 0x00};
    size_t i;

    redisAsyncCommand(
        rctx, mrPingreqCallback, NULL,
        "%s %b", MR_PINGREQ, PINGREQ_BUF, sizeof(PINGREQ_BUF)
    );

    printf("Async Command sent PINGREQ (%s)\n", MR_PINGREQ);
    printf("  %lu chars sent in buffer: ", sizeof(PINGREQ_BUF));

    for (i = 0; i < sizeof(PINGREQ_BUF); i++) {
        printf("%hhX ", PINGREQ_BUF[i]);
    }

    printf("\n");
    printf("  Buffer is a valid PINGREQ\n");
}

void mrConnectCallback(redisAsyncContext *rctx, void *reply_void, void *private_data_void) {
}

void mr_send_connect(redisAsyncContext *rctx) {
    packet_ctx *pctx;
    int rc = init_connect_pctx(&pctx);
    // set_scalar_value(pctx, "clean_start", true);
    set_connect_clean_start(pctx, true);
    bool clean_start;
    get_connect_clean_start(pctx, &clean_start);
    printf("get_clean_start: %u\n", clean_start);
    // set_scalar_value(pctx, "will_qos", 3);
    // set_scalar_value(pctx, "session_expiry", 42);
    string_pair foobar = {(uint8_t *)"fool", (uint8_t *)"barr"};
    string_pair sp0[] = {foobar, foobar};
    size_t sp_count = sizeof(sp0) / sizeof(string_pair);
    // set_vector_value(pctx, "user_properties", (Word_t)sps, 2);
    set_connect_user_properties(pctx, sp0, sp_count);
    string_pair *mysp0;
    size_t mysp0len;
    rc = get_connect_user_properties(pctx, &mysp0, &mysp0len);

    if (!rc) {
        printf("user_properties:\n");
        for (int i = 0; i < mysp0len; i++, mysp0++) {
            printf("  name: %s; value: %s\n", mysp0->name, mysp0->value);
        }
    }

    uint8_t bambaz[] = {0x01, 0x02};
    // set_vector_value(pctx, "authentication_data", (Word_t)bambaz, 2);
    set_connect_authentication_data(pctx, bambaz, sizeof(bambaz));
    uint8_t *myauth;
    size_t myauthlen;
    rc = get_connect_authentication_data(pctx, &myauth, &myauthlen);

    if (!rc) {
        printf("authentication_data:");
        for (int i = 0; i < myauthlen; i++, myauth++) printf(" %02hhX", *myauth);
        puts("\n");
    }

    // set_scalar_value(pctx, "client_identifier", (Word_t)"Snoopy");
    pack_connect_buffer(pctx);

    printf("Connect Buf:");
    for (int i = 0; i < pctx->len; i++) printf(" %02hhX", pctx->buf[i]);
    printf("\n");

    unpack_connect_buffer(pctx);

    uint8_t ptype = 0;
    rc = get_connect_packet_type(pctx, &ptype);
    printf("packet_type: rc: %d; ptype: %u\n", rc, ptype);

    uint32_t remlen = 0;
    rc = get_connect_remaining_length(pctx, &remlen);
    printf("remaining_length: rc: %d; remlen: %u\n", rc, remlen);

    free_connect_pctx(pctx);
}

int main(void) {
    uv_loop_t* loop = uv_default_loop();

    redisAsyncContext *rctx = redisAsyncConnect("127.0.0.1", 6379);

    if (rctx->err) {
        /* Let *rctx leak for now... */
        printf("Error: %s\n", rctx->errstr);
        return 1;
    }

    printf("Redis Connect sent\n");

    redisLibuvAttach(rctx,loop);
    redisAsyncSetConnectCallback(rctx, redisAsyncConnectCallback);
    redisAsyncSetDisconnectCallback(rctx, redisAsyncDisconnectCallback);

    puts("Preparing to send mr connect\n");
    mr_send_connect(rctx);
    puts("MisteR CONNECT sent\n");

    mrSendPingreq(rctx);
    printf("MisteR PINGREQ sent - will terminate when PINGRESP is handled\n");

    printf("Activating event loop...\n");
    uv_run(loop, UV_RUN_DEFAULT);
    return 0;
}
