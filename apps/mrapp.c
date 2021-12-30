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
#include "mister/mrzlog.h"

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
        (uint8_t)reply1->str[0] == MQTT_PINGRESP
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
    uint8_t PINGREQ_BUF[2] = {MQTT_PINGREQ, 0x00};
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
    int rc = mr_init_connect_pctx(&pctx);
    mr_set_connect_clean_start(pctx, true);
    mr_set_connect_will_qos(pctx, 3);
    bool clean_start;
    mr_get_connect_clean_start(pctx, &clean_start);
    printf("get_clean_start: %u\n", clean_start);
    string_pair foobar = {(uint8_t *)"fool", (uint8_t *)"barr"};
    string_pair spv0[] = {foobar, foobar};
    size_t sp_count = sizeof(spv0) / sizeof(string_pair);
    mr_set_connect_user_properties(pctx, spv0, sp_count);
    string_pair *myspv0;
    size_t myspvlen;
    rc = mr_get_connect_user_properties(pctx, &myspv0, &myspvlen);

    if (!rc) {
        string_pair *pmysp = myspv0;
        printf("user_properties:\n");

        for (int i = 0; i < myspvlen; i++, pmysp++) {
            printf("  name: %s; value: %s\n", pmysp->name, pmysp->value);
        }
    }

    uint32_t session_expiry = 42;
    rc = mr_set_connect_session_expiry_interval(pctx, session_expiry);

    uint8_t bambaz[] = {0x01, 0x02};
    mr_set_connect_authentication_data(pctx, bambaz, sizeof(bambaz));
    uint8_t *myauthv0;
    size_t myauthvlen;
    rc = mr_get_connect_authentication_data(pctx, &myauthv0, &myauthvlen);

    if (!rc) {
        uint8_t *pmyauth = myauthv0;
        printf("authentication_data:");
        for (int i = 0; i < myauthvlen; i++) printf(" %02hhX", *pmyauth++);
        puts("\n");
    }

    mr_pack_connect_u8v0(pctx);

    printf("Connect Buf:");
    for (int i = 0; i < pctx->len; i++) printf(" %02hhX", pctx->u8v0[i]);
    printf("\n");

    mr_unpack_connect_u8v0(pctx);

    uint8_t type = 0;
    rc = mr_get_connect_packet_type(pctx, &type);
    printf("packet_type: rc: %d; type: %u\n", rc, type);

    uint32_t remlen = 0;
    rc = mr_get_connect_remaining_length(pctx, &remlen);
    printf("remaining_length: rc: %d; remlen: %u\n", rc, remlen);

    uint8_t *proto_nmv0;
    size_t protovlen;
    rc = mr_get_connect_protocol_name(pctx, &proto_nmv0, &protovlen);
    uint8_t *pproto_nm = proto_nmv0;
    printf("protocol_name: rc: %d; len: %lu; bytes:", rc, protovlen);
    for (int i = 0; i < protovlen; i++) printf(" %02hhX", *pproto_nm++);
    printf("\n");

    uint8_t version = 0;
    rc = mr_get_connect_protocol_version(pctx, &version);
    printf("protocol_version: rc: %d; version: %u\n", rc, version);

    mr_get_connect_clean_start(pctx, &clean_start);
    printf("get_clean_start: %u\n", clean_start);

    uint8_t will_qos;
    mr_get_connect_will_qos(pctx, &will_qos);
    printf("get_will_qos: %u\n", will_qos);

    uint32_t property_length = 0;
    mr_get_connect_property_length(pctx, &property_length);
    printf("property_length: %u\n", property_length);

    session_expiry = 0;
    rc = mr_get_connect_session_expiry_interval(pctx, &session_expiry);
    printf("session_expiry: rc: %d; %u\n", rc, session_expiry);

    myspv0 = NULL;
    rc = mr_get_connect_user_properties(pctx, &myspv0, &myspvlen);

    if (!rc) {
        string_pair *pmysp = myspv0;
        printf("user_properties:\n");

        for (int i = 0; i < myspvlen; i++, pmysp++) {
            printf("  name: %s; value: %s\n", pmysp->name, pmysp->value);
        }
    }

    myauthv0 = NULL;
    rc = mr_get_connect_authentication_data(pctx, &myauthv0, &myauthvlen);

    if (!rc) {
        uint8_t *pmyauth = myauthv0;
        printf("authentication_data:");
        for (int i = 0; i < myauthvlen; i++) printf(" %02hhX", *pmyauth++);
        puts("\n");
    }
    mr_free_connect_pctx(pctx);
}

int main(void) {
    int rc = dzlog_init("", "mr_init");
    printf("dzlog_init: rc: %d\n", rc);

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

    dzlog_info("hello & goodbye, zlog");
    zlog_fini();
    return 0;
}
