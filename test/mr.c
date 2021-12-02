#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <uuid/uuid.h>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libev.h>

/* #define MAXLINELEN 1000000           // define maximum string length */
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
    uint8_t buf[1000] = {0};
    size_t pos = 0;
    Pvoid_t PJSLArray = (Pvoid_t)NULL;  // initialize JudySL array
    connect_hv hv, *Phv, **PPhv;
    size_t hv_count = sizeof(connect_hvs) / sizeof(connect_hv);

    // build an associative array of name to header variable (hv) via a handle
    for (i = 0; i < hv_count; i++) {
        JSLI(PPhv, PJSLArray, connect_hvs[i].name); 
        *PPhv = &connect_hvs[i];
    }

    // for example, set remaining_length to 42
    JSLG(PPhv, PJSLArray, "remaining_length"); // get the handle
    Phv = *PPhv; // dereference to get a pointer to the hv
    Phv->value = 42; // then reset the value for the header variable

    // Values should all have been set - pack into a buffer using the packing function for each header var
    for (i = 0; i < hv_count; i++) {
        hv = connect_hvs[i];
        hv.pack_fn(hv.value, buf, &pos);
    }

    printf("Connect Buf:");
    for (i = 0; i < pos; i++) printf(" %02hhX", buf[i]);
    printf("\n");
  
/*
    uuid_t *corrid = malloc(sizeof(uuid_t));
    uuid_generate_random(*corrid);
    char corrid_str[UUID_STR_LEN];
    uuid_unparse(*corrid, corrid_str);

    size_t ch_vars_size = sizeof(connect_header_vars_template);
    connect_header_vars *ch_vars = malloc(ch_vars_size);
    memcpy(ch_vars, &connect_header_vars_template, ch_vars_size);
    printf("sizeof connect_header_vars_template: %d\n", ch_vars_size);

    memcpy(buf + pos, &ch_vars->packet_type, 1); pos++;
*/

/*    
    char *hdr_nms[] = {
        "foo",
        "bar"
    };

    typedef struct hdr {
        char *name;
        char type;
    } hdr;

    hdr hdrs[] = {
        {"foo", 10},
        {"bar", 11}
    };

    hdr **PPhdr;
    JSLI(PPhdr, PJSLArray, hdrs[0].name);
    printf("Phdr: %d\n", *PPhdr);
    *PPhdr = &hdrs[0];
    printf("Phdr: %d\n", *PPhdr);
    printf("insert type: %d\n", (**PPhdr).type);
    JSLG(PPhdr, PJSLArray, hdrs[0].name);
    printf("Phdr: %d\n", *PPhdr);
    printf("lookup type: %d\n", (**PPhdr).type);


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

    mrSendConnect(ctx);
    printf("MisteR CONNECT sent\n");

    mrSendPingreq(ctx);
    printf("MisteR PINGREQ sent - will terminate when PINGRESP is handled\n");

    printf("Activating event loop...\n");
    ev_loop(EV_DEFAULT_ 0);
    return 0;
}
