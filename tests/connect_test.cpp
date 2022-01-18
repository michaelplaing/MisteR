#include <unistd.h>
#include <catch2/catch.hpp>
#include "mister/mister.h"

int get_binary_file_content(const char *fixfilename, uint8_t **pu8v, uint32_t *pffsz) {
    FILE *fixfile;
    fixfile = fopen(fixfilename, "r");
    if (fixfile == NULL) return -1;
    fseek(fixfile, 0, SEEK_END);
    *pffsz = ftell(fixfile);
    fseek(fixfile, 0, SEEK_SET);
    *pu8v = (uint8_t *)malloc(*pffsz);
    if (*pu8v == NULL) return -2;
    fread(*pu8v, 1, *pffsz, fixfile);
    fclose(fixfile);
    return 0;
}

int put_binary_file_content(const char *fixfilename, uint8_t *u8v, uint32_t ffsz) {
    FILE *fixfile;
    fixfile = fopen(fixfilename, "w");
    if (fixfile == NULL) return -1;
    fwrite(u8v, 1, ffsz, fixfile);
    fclose(fixfile);
    return 0;
}

TEST_CASE("CONNECT packet", "[connect]") {
    packet_ctx *pctx;
    int rc0 = mr_init_connect_packet(&pctx);
    int rc1 = mr_connect_mdata_dump(pctx);
    int rc2 = mr_pack_connect_packet(pctx);

    SECTION("initialization succeeds") {
        REQUIRE(rc0 == 0);
    }
    SECTION("mdata_dump succeeds") {
        REQUIRE(rc1 == 0);
    }
    SECTION("mdata_dump is correct") {
        char *mdata_dump;
        uint32_t mdsz;
        int rc = get_binary_file_content("init_mdata_dump.txt", (uint8_t **)&mdata_dump, &mdsz);
        REQUIRE(rc == 0);
        REQUIRE(strncmp(mdata_dump, pctx->mdata_dump, strlen(pctx->mdata_dump)) == 0);
    }
    SECTION("pack connect packet succeeds") {
        print_hexdump(pctx->u8v0, pctx->u8vlen);
        // int rc = put_binary_file_content("init_pack_connect.bin", pctx->u8v0, pctx->u8vlen);
        REQUIRE(rc2 == 0);
    }
    SECTION("connect packet is correct") {
        uint8_t *u8v0;
        uint32_t u8vlen;
        int rc = get_binary_file_content("init_pack_connect.bin", &u8v0, &u8vlen);
        REQUIRE(rc == 0);
        REQUIRE(memcmp(u8v0, pctx->u8v0, u8vlen) == 0);
    }
}