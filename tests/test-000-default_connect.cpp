#include <unistd.h>
#include <catch2/catch.hpp>
#include "mister/mister.h"
#include "util.h"

TEST_CASE("default CONNECT packet", "[connect]") {
    packet_ctx *pctx;
    int rc00 = mr_init_connect_packet(&pctx);
    int rc10 = mr_connect_mdata_dump(pctx);
    int rc20 = mr_pack_connect_packet(pctx);
    int rc30 = mr_free_connect_packet(pctx);

    SECTION("connect packet initialization succeeds") {
        REQUIRE(rc00 == 0);
    }
    SECTION("connect mdata_dump succeeds") {
        REQUIRE(rc10 == 0);
        // int rc = put_binary_file_content("fixtures/init_mdata_dump.txt", (uint8_t *)pctx->mdata_dump, strlen(pctx->mdata_dump));
    }
    SECTION("connect mdata_dump is correct") {
        char *mdata_dump;
        uint32_t mdsz;
        int rc = get_binary_file_content("fixtures/init_mdata_dump.txt", (uint8_t **)&mdata_dump, &mdsz);
        REQUIRE(rc == 0);
        REQUIRE(mdsz == strlen(pctx->mdata_dump));
        REQUIRE(strncmp(mdata_dump, pctx->mdata_dump, mdsz) == 0);
        free(mdata_dump);
    }
    SECTION("pack connect packet succeeds") {
        mr_print_hexdump(pctx->u8v0, pctx->u8vlen);
        // int rc = put_binary_file_content("fixtures/init_pack_connect.bin", pctx->u8v0, pctx->u8vlen);
        REQUIRE(rc20 == 0);
    }
    SECTION("packed connect packet is correct") {
        uint8_t *u8v0;
        uint32_t u8vlen;
        int rc = get_binary_file_content("fixtures/init_pack_connect.bin", &u8v0, &u8vlen);
        REQUIRE(rc == 0);
        REQUIRE(u8vlen == pctx->u8vlen);
        REQUIRE(memcmp(u8v0, pctx->u8v0, u8vlen) == 0);
        free(u8v0);
    }
    SECTION("free connect packet succeeds") {
        REQUIRE(rc30 == 0);
    }
    SECTION("unpack/free connect packet succeeds") {
        uint8_t *u8v0;
        uint32_t u8vlen;
        int rc = get_binary_file_content("fixtures/init_pack_connect.bin", &u8v0, &u8vlen);
        REQUIRE(rc == 0);
        int rc40 = mr_init_unpack_connect_packet(&pctx, u8v0, u8vlen);
        REQUIRE(rc40 == 0);
        rc = mr_free_connect_packet(pctx);
        REQUIRE(rc == 0);
    }
    SECTION("unpacked connect packet mdata_dump succeeds") {
        uint8_t *u8v0;
        uint32_t u8vlen;
        int rc = get_binary_file_content("fixtures/init_pack_connect.bin", &u8v0, &u8vlen);
        REQUIRE(rc == 0);
        int rc40 = mr_init_unpack_connect_packet(&pctx, u8v0, u8vlen);
        REQUIRE(rc40 == 0);
        int rc10 = mr_connect_mdata_dump(pctx);
        REQUIRE(rc10 == 0);
        rc = mr_free_connect_packet(pctx);
        REQUIRE(rc == 0);
    }
    SECTION("unpacked connect packet mdata_dump is correct") {
        uint8_t *u8v0;
        uint32_t u8vlen;
        int rc = get_binary_file_content("fixtures/init_pack_connect.bin", &u8v0, &u8vlen);
        REQUIRE(rc == 0);
        int rc40 = mr_init_unpack_connect_packet(&pctx, u8v0, u8vlen);
        REQUIRE(rc40 == 0);
        int rc10 = mr_connect_mdata_dump(pctx);
        REQUIRE(rc10 == 0);
        char *mdata_dump;
        uint32_t mdsz;
        rc = get_binary_file_content("fixtures/init_mdata_dump.txt", (uint8_t **)&mdata_dump, &mdsz);
        REQUIRE(rc == 0);
        // puts("mdata_dump:"); mr_print_hexdump((uint8_t *)mdata_dump, mdsz);
        // puts("pctx->mdata_dump:"); mr_print_hexdump((uint8_t *)pctx->mdata_dump, strlen(pctx->mdata_dump));
        REQUIRE(mdsz == strlen(pctx->mdata_dump));
        REQUIRE(strncmp(mdata_dump, pctx->mdata_dump, mdsz) == 0);
        free(mdata_dump);
        rc = mr_free_connect_packet(pctx);
        REQUIRE(rc == 0);
    }
}