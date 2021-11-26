#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <uuid/uuid.h>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libev.h>

#include "redismodule.h"
#include "mqtt_protocol.h"
#include "mister.h"

void mqttPingreqCallback(redisAsyncContext *ctx, void *reply_void, void *private_data_void) {
    REDISMODULE_NOT_USED(ctx);
    redisReply *reply = reply_void;
    uuid_t *pcorrid = private_data_void;
    size_t i;
    char corrid_str[UUID_STR_LEN];
    uuid_unparse(*pcorrid, corrid_str);
    free(pcorrid);

    if (reply == NULL) return;
    printf("Callback - expecting PINGRESP (%s)\n", PINGRESP_MRCMD);
    printf("  private data rcvd (corrid): %s\n", corrid_str);
    printf("  %lu elements in reply array\n", reply->elements);
    redisReply *ele0 = reply->element[0];
    printf("    %s - PINGRESP_MRCMD ", ele0->str);

    if (strcmp(ele0->str, PINGRESP_MRCMD)) {
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
/*
    redisAsyncDisconnect(ctx);
    printf("Disconnect sent\n");
*/
}

void connectCallback(const redisAsyncContext *ctx, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", ctx->errstr);
        return;
    }

    printf("Connected...\n");
}

void disconnectCallback(const redisAsyncContext *ctx, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", ctx->errstr);
        return;
    }

    printf("Disconnected...\n");
}

void send_pingreq(redisAsyncContext *ctx) {
    const unsigned char PINGREQ_BUF[2] = {CMD_PINGREQ, 0x00};
    // const char PINGREQ_CMD[] = "mqtt.pingreq";
    size_t i;

    uuid_t *pcorrid = malloc(sizeof(uuid_t));
    uuid_generate_random(*pcorrid);
    char corrid_str[UUID_STR_LEN];
    uuid_unparse(*pcorrid, corrid_str);


    redisAsyncCommand(
        ctx, mqttPingreqCallback, pcorrid,
        "%s %b", PINGREQ_MRCMD, PINGREQ_BUF, sizeof(PINGREQ_BUF)
    );
    
    printf("Async Command sent PINGREQ (%s)\n", PINGREQ_MRCMD);
    printf("  private data sent (corrid): %s\n", corrid_str);
    printf("  %lu chars sent in buffer: ", sizeof(PINGREQ_BUF));

    for (i = 0; i < sizeof(PINGREQ_BUF); i++) {
        printf("%hhX ", PINGREQ_BUF[i]);
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
    redisAsyncSetConnectCallback(ctx, connectCallback);
    redisAsyncSetDisconnectCallback(ctx, disconnectCallback);

    send_pingreq(ctx);

    printf("Activating event loop...\n");
    ev_loop(EV_DEFAULT_ 0);
    return 0;
}
