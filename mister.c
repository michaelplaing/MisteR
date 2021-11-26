#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libev.h>

#include "mqtt_protocol.h"

void mqttPingreqCallback(redisAsyncContext *ctx, void *reply_void, void *private_data_void) {
    redisReply *reply = reply_void;
    char *private_data = private_data_void;
    size_t i;

    if (reply == NULL) return;
    printf("Expecting PINGRESP\n");
    printf("  private data: %s\n", private_data);
    printf("  %lu elements in reply array\n", reply->elements);
    printf("    %s\n", reply->element[0]->str);
    redisReply *ele1 = reply->element[1];
    printf("    %lu chars received in buffer: ", ele1->len);
    
    for (i = 0; i < ele1->len; i++) {
        printf("%hhX ", ele1->str[i]);
    }

    if (*(ele1->str) == CMD_PINGRESP) {
        printf("\n    Confirmed that buffer is a valid PINGRESP\n");
    }
    else {
        printf("\n    Buffer is not a valid PINGRESP\n");
    }
    redisAsyncDisconnect(ctx);
    printf("Disconnect sent\n");
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

int main (void) {
    const unsigned char PINGREQ_BUF[2] = {CMD_PINGREQ, 0x00};
    const char MQTT_PINGREQ[] = "mqtt.pingreq";
    size_t i;

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

    redisAsyncCommand(
        ctx, 
        mqttPingreqCallback, 
        (char *)MQTT_PINGREQ, 
        "%s %b", 
        MQTT_PINGREQ, 
        PINGREQ_BUF, 
        sizeof(PINGREQ_BUF)
    );
    
    printf("Async Command sent: %s\n", MQTT_PINGREQ);
    printf("  private data: %s\n", MQTT_PINGREQ);
    printf("  %lu chars sent in buffer: ", sizeof(PINGREQ_BUF));

    for (i = 0; i < sizeof(PINGREQ_BUF); i++) {
        printf("%hhX ", PINGREQ_BUF[i]);
    }

    printf("\n");
    ev_loop(EV_DEFAULT_ 0);
    return 0;
}
