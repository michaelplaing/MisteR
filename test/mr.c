#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libev.h>

#include "redismodule.h"
#include "mister.h"
#include "connect.h"

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
    pack_ctx *pctx = init_connect_pctx();
    set_scalar_value(pctx, "clean_start", true);
    // set_scalar_value(pctx, "will_qos", 3);
    set_scalar_value(pctx, "session_expiry", 42);
    string_pair foobar = {"foo", "bar"};
    string_pair sps[] = {foobar, foobar};
    set_vector_value(pctx, "user_properties", (Word_t)sps, 2);
    uint8_t bambaz[] = {0x01, 0x02};
    set_vector_value(pctx, "authentication_data", (Word_t)bambaz, 2);
    set_scalar_value(pctx, "client_identifier", (Word_t)"Snoopy");
    pack_mdata_buffer(pctx);

    printf("Connect Buf:");
    for (int i = 0; i < pctx->len; i++) printf(" %02hhX", pctx->buf[i]);
    printf("\n");
    // free_pack_context(pctx);
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

    puts("Preparing to send mr connect\n");
    mr_send_connect(rctx);
    puts("MisteR CONNECT sent\n");

    mrSendPingreq(rctx);
    printf("MisteR PINGREQ sent - will terminate when PINGRESP is handled\n");

    printf("Activating event loop...\n");
    ev_loop(EV_DEFAULT_ 0);
    return 0;
}
