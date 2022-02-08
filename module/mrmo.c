/* mister module */

#include <redis/redismodule.h>

#include "mister/mister.h"
#include "mister/mrzlog.h"
/*
int misterPingReq_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    const uint8_t PINGRESP_BUF[2] = {MQTT_PINGRESP, 0};

    REDISMODULE_NOT_USED(argv);
    if (argc != 2) return RedisModule_WrongArity(ctx);
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithCString(ctx, MR_PINGRESP);
    RedisModule_ReplyWithStringBuffer(ctx, (const char *)PINGRESP_BUF, 2);
    return REDISMODULE_OK;
}

int misterConnect_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    const uint8_t CONNACK_BUF[2] = {MQTT_CONNACK, 0};

    REDISMODULE_NOT_USED(argv);
    if (argc != 2) return RedisModule_WrongArity(ctx);
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithCString(ctx, MR_CONNACK);
    RedisModule_ReplyWithStringBuffer(ctx, (const char *)CONNACK_BUF, 2);
    return REDISMODULE_OK;
}
 */
int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (
        RedisModule_Init(ctx, "mister", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR
    ) return REDISMODULE_ERR;

    const char *mrzlog_conf = RedisModule_StringPtrLen(argv[0], NULL);
    if (dzlog_init(mrzlog_conf, "mr_init")) return REDISMODULE_ERR;
    dzlog_info("mrzlog logging initialized"); // to stdout probably (see configs)
    const char *mrzlog_category = RedisModule_StringPtrLen(argv[1], NULL);
    dzlog_set_category(mrzlog_category);
    dzlog_info("mrzlog logging initialized"); // to log probably (see configs)

    for (int j = 0; j < argc; j++) {
        const char *s = RedisModule_StringPtrLen(argv[j], NULL);
        printf("Module loaded with ARGV[%d] = %s\n", j, s);
    }
/*
    if (
        RedisModule_CreateCommand(
            ctx, MR_PINGREQ, misterPingReq_RedisCommand, "", 1, 1, 1
        ) == REDISMODULE_ERR
    ) return REDISMODULE_ERR;

    if (
        RedisModule_CreateCommand(
            ctx, MR_CONNECT, misterConnect_RedisCommand, "", 1, 1, 1
        ) == REDISMODULE_ERR
    ) return REDISMODULE_ERR;
 */
    return REDISMODULE_OK;
}
