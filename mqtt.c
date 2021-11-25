/* Mqtt module */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "redismodule.h"
#include "mqtt_protocol.h"

int MqttPingReq_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    const unsigned char PINGRESP_BUF[2] = {CMD_PINGRESP, 0};
    
    REDISMODULE_NOT_USED(argv);
    if (argc != 2) return RedisModule_WrongArity(ctx);
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithSimpleString(ctx, "mqtt.pingresp");
    RedisModule_ReplyWithStringBuffer(ctx, (const  char *)PINGRESP_BUF, 2);
    return REDISMODULE_OK;
}

int MqttConnect_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    const unsigned char CONNACK_BUF[2] = {CMD_CONNACK, 0};
    
    REDISMODULE_NOT_USED(argv);
    if (argc != 2) return RedisModule_WrongArity(ctx);
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithSimpleString(ctx, "mqtt.connack");
    RedisModule_ReplyWithStringBuffer(ctx, (const char *)CONNACK_BUF, 2);
    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (
        RedisModule_Init(ctx, "mqtt", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR
    ) return REDISMODULE_ERR;

    for (int j = 0; j < argc; j++) {
        const char *s = RedisModule_StringPtrLen(argv[j], NULL);
        printf("Module loaded with ARGV[%d] = %s\n", j, s);
    }

    if (
        RedisModule_CreateCommand(
            ctx, "mqtt.pingreq", MqttPingReq_RedisCommand, "", 1, 1, 1
        ) == REDISMODULE_ERR
    ) return REDISMODULE_ERR;

    if (
        RedisModule_CreateCommand(
            ctx, "mqtt.connect", MqttConnect_RedisCommand, "", 1, 1, 1
        ) == REDISMODULE_ERR
    ) return REDISMODULE_ERR;

    return REDISMODULE_OK;
}
