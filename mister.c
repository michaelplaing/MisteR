#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libev.h>

void mqttPingreqCallback(redisAsyncContext *ctx, void *reply_void, void *private_data_void) {
    size_t i;

    redisReply *reply = reply_void;
    if (reply == NULL) return;
    printf("%s: %lu elements in reply array\n", (char*)private_data_void, reply->elements);
    printf("  %s\n", reply->element[0]->str);
    redisReply *ele1 = reply->element[1];
    printf("  %lu chars received in buffer: ", ele1->len);

    for (i = 0; i < ele1->len; i++) {
        printf("%02hhX ", ele1->str[i]);
    }

    printf("\n");
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
    const unsigned char PINGREQ_BUF[2] = {0xc0, 0x00};
    const size_t PINGREQ_BUF_LEN = 2;
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
        PINGREQ_BUF_LEN
    );
    
    printf("Async Command sent: %s\n", MQTT_PINGREQ);
    printf("  private data: %s\n", MQTT_PINGREQ);
    printf("  %lu chars sent in buffer: ", PINGREQ_BUF_LEN);

    for (i = 0; i < PINGREQ_BUF_LEN; i++) {
        printf("%02hhX ", PINGREQ_BUF[i]);
    }

    printf("\n");
    ev_loop(EV_DEFAULT_ 0);
    return 0;
}
