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

void mrPingreqCallback(redisAsyncContext *ctx, void *reply_void, void *private_data_void) {
    REDISMODULE_NOT_USED(ctx);
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

    redisAsyncDisconnect(ctx);
    printf("Redis Disconnect sent\n");
}

void redisAsyncConnectCallback(const redisAsyncContext *ctx, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", ctx->errstr);
        return;
    }

    printf("Connected to Redis...\n");
}

void redisAsyncDisconnectCallback(const redisAsyncContext *ctx, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", ctx->errstr);
        return;
    }

    printf("Disconnected from Redis\n");
}

void mrSendPingreq(redisAsyncContext *ctx) {
    const char PINGREQ_BUF[2] = {CMD_PINGREQ, 0x00};
    size_t i;

    uuid_t *corrid = malloc(sizeof(uuid_t));
    uuid_generate_random(*corrid);
    char corrid_str[UUID_STR_LEN];
    uuid_unparse(*corrid, corrid_str);

    redisAsyncCommand(
        ctx, mrPingreqCallback, corrid,
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

void mrConnectCallback(redisAsyncContext *ctx, void *reply_void, void *private_data_void) {
}

void mrSendConnect(redisAsyncContext *ctx) {
    size_t i;
    uint8_t *connect_packet;
 
    uuid_t *corrid = malloc(sizeof(uuid_t));
    uuid_generate_random(*corrid);
    char corrid_str[UUID_STR_LEN];
    uuid_unparse(*corrid, corrid_str);

    connect_packet_vars *cp_vars = malloc(sizeof(cp_vars_template));
    memcpy(cp_vars, &cp_vars_template, sizeof(cp_vars_template));
    printf("sizeof template: %d\n", sizeof(cp_vars_template));

/*
    size_t buflen = 2;

    redisAsyncCommand(
        ctx, mrConnectCallback, corrid,
        "%s %b", MR_CONNECT, connect_packet, buflen
    );
    
    printf("Async Command sent CONNECT (%s)\n", MR_CONNECT);
    printf("  private data sent (corrid): %s\n", corrid_str);
    printf("  %lu chars sent in buffer: ", buflen);

    for (i = 0; i < 16 && i < buflen; i++) {
        printf("%hhX ", connect_packet[i]);
    }

    printf("\n");
    printf("  Buffer is a valid PINGREQ\n");
*/
}

int main (void) {
    redisAsyncContext *ctx = redisAsyncConnect("127.0.0.1", 6379);
    
    if (ctx->err) {
        /* Let *ctx leak for now... */
        printf("Error: %s\n", ctx->errstr);
        return 1;
    }

    printf("Redis Connect sent\n");

    redisLibevAttach(EV_DEFAULT_ ctx);
    redisAsyncSetConnectCallback(ctx, redisAsyncConnectCallback);
    redisAsyncSetDisconnectCallback(ctx, redisAsyncDisconnectCallback);

    printf("MisteR CONNECT sent\n");
    mrSendConnect(ctx);

    printf("MisteR PINGREQ sent - will terminate when PINGRESP is handled\n");
    mrSendPingreq(ctx);

    printf("Activating event loop...\n");
    ev_loop(EV_DEFAULT_ 0);
    return 0;
}
