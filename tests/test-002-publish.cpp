#include <catch2/catch.hpp>
#include <zlog.h>

#include "mister/mister.h"
#include "test_util.h"

TEST_CASE("happy PUBLISH packet", "[publish][happy]") {
    dzlog_init("", "mr_init"); // enables logging from the mister library and here

    // *** common test prolog ***

    mr_packet_ctx *pctx;

    // init
    REQUIRE(mr_init_publish_packet(&pctx) == 0);

    // *** test sections ***

    SECTION("default packet") {
        const uint32_t u32v0[2] = {1,1000000};
        uint32_t *pu32v0;
        size_t len = 0;
        bool exists_flag = false;

        CHECK(mr_set_publish_subscription_identifiers(pctx, u32v0, 2) == 0);
        CHECK(mr_get_publish_subscription_identifiers(pctx, &pu32v0, &len, &exists_flag) == 0);
        printf("subscription_identifiers:: len: %lu; values:", len);
        for (int i = 0; i < len; i++) printf(" %u;", pu32v0[i]);
        puts("");
    }
    // free packet context
    REQUIRE(mr_free_publish_packet(pctx) == 0);

    zlog_fini();
}
