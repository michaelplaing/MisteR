#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
    printf("Callback - expecting PINGRESP (%s)\n", MRCMD_PINGRESP);
    printf("  private data rcvd (corrid): %s\n", corrid_str);
    printf("  %lu elements in reply array\n", reply->elements);
    redisReply *ele0 = reply->element[0];
    printf("    %s - MRCMD_PINGRESP ", ele0->str);

    if (strcmp(ele0->str, MRCMD_PINGRESP)) {
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
    printf("Disconnect sent\n");
}

void redisAsyncConnectCallback(const redisAsyncContext *ctx, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", ctx->errstr);
        return;
    }

    printf("Connected...\n");
}

void redisAsyncDisconnectCallback(const redisAsyncContext *ctx, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", ctx->errstr);
        return;
    }

    printf("Disconnected...\n");
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
        "%s %b", MRCMD_PINGREQ, PINGREQ_BUF, sizeof(PINGREQ_BUF)
    );
    
    printf("Async Command sent PINGREQ (%s)\n", MRCMD_PINGREQ);
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

    typedef struct connect_packet_vars {
        uint8_t packet_type;
        size_t remaining_len;
        uint8_t protocol_name[6];
        uint8_t protocol_version;
        bool reserved;
        bool clean_start;
        bool will_flag;
        uint8_t will_qos;
        bool will_retain;
        bool password_flag;
        bool username_flag;
    } cp_vars;

    cp_vars cpv = {
        .packet_type = CMD_CONNECT,
        .remaining_len = 0,
        .protocol_name = {0x00, 0x04, 'M', 'Q', 'T', 'T'},
        .protocol_version = 5,
        .reserved = false,
        .clean_start = false,
        .will_flag = false,
        .will_qos = 0,
        .will_retain = false,
        .password_flag = false,
        .username_flag = false
    };

    uuid_t *corrid = malloc(sizeof(uuid_t));
    uuid_generate_random(*corrid);
    char corrid_str[UUID_STR_LEN];
    uuid_unparse(*corrid, corrid_str);

    size_t buflen = cpv.remaining_len + 2;

    redisAsyncCommand(
        ctx, mrConnectCallback, corrid,
        "%s %b", MRCMD_CONNECT, connect_packet, buflen
    );
    
    printf("Async Command sent CONNECT (%s)\n", MRCMD_CONNECT);
    printf("  private data sent (corrid): %s\n", corrid_str);
    printf("  %lu chars sent in buffer: ", buflen);

    for (i = 0; i < 16 && i < buflen; i++) {
        printf("%hhX ", connect_packet[i]);
    }

    printf("\n");
    printf("  Buffer is a valid PINGREQ\n");

}

int main (void) {
    redisAsyncContext *ctx = redisAsyncConnect("127.0.0.1", 6379);
    
    if (ctx->err) {
        /* Let *ctx leak for now... */
        printf("Error: %s\n", ctx->errstr);
        return 1;
    }

    printf("Connect sent\n");

    redisLibevAttach(EV_DEFAULT_ ctx);
    redisAsyncSetConnectCallback(ctx, redisAsyncConnectCallback);
    redisAsyncSetDisconnectCallback(ctx, redisAsyncDisconnectCallback);

    mrSendPingreq(ctx);

    printf("Activating event loop...\n");
    ev_loop(EV_DEFAULT_ 0);
    return 0;
}
