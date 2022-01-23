#include <catch2/catch.hpp>
#include "mister/mister.h"
#include "mister/mrzlog.h"
#include "util.h"

TEST_CASE("default CONNECT packet", "[connect happy]") {
    dzlog_init("", "mr_init");
    int rc;
    packet_ctx *pctx;

    // init
    int rc00 = mr_init_connect_pctx(&pctx);
    REQUIRE(rc00 == 0);

    // dump
    int rc10 = mr_connect_mdata_dump(pctx);
    REQUIRE(rc10 == 0);
    // rc = put_binary_file_content("fixtures/default_connect_mdata_dump.txt", (uint8_t *)pctx->mdata_dump, strlen(pctx->mdata_dump));
    // REQUIRE(rc == 0);

    // check dump
    char *mdata_dump;
    uint32_t mdsz;
    rc = get_binary_file_content("fixtures/default_connect_mdata_dump.txt", (uint8_t **)&mdata_dump, &mdsz);
    REQUIRE(rc == 0);
    REQUIRE(mdsz == strlen(pctx->mdata_dump));
    REQUIRE(strncmp(mdata_dump, pctx->mdata_dump, mdsz) == 0);
    free(mdata_dump);

    // pack
    int rc20 = mr_pack_connect_packet(pctx);
    mr_print_hexdump(pctx->u8v0, pctx->u8vlen);
    // rc = put_binary_file_content("fixtures/default_connect_packet.bin", pctx->u8v0, pctx->u8vlen);
    // REQUIRE(rc == 0);
    REQUIRE(rc20 == 0);

    // check packet
    uint8_t *u8v0;
    uint32_t u8vlen;
    rc = get_binary_file_content("fixtures/default_connect_packet.bin", &u8v0, &u8vlen);
    REQUIRE(rc == 0);
    REQUIRE(u8vlen == pctx->u8vlen);
    REQUIRE(memcmp(u8v0, pctx->u8v0, u8vlen) == 0);
    free(u8v0);

    // free context
    int rc30 = mr_free_connect_pctx(pctx);
    REQUIRE(rc30 == 0);

    // init context / unpack packet
    int rc40 = mr_init_unpack_connect_packet(&pctx, u8v0, u8vlen);
    REQUIRE(rc40 == 0);

    // dump
    int rc50 = mr_connect_mdata_dump(pctx);
    REQUIRE(rc50 == 0);

    // check dump
    REQUIRE(mdsz == strlen(pctx->mdata_dump));
    REQUIRE(strncmp(mdata_dump, pctx->mdata_dump, mdsz) == 0);
    free(mdata_dump);

    // free context
    int rc60 = mr_free_connect_pctx(pctx);
    REQUIRE(rc60 == 0);

    zlog_fini();
}