#include <catch2/catch.hpp>
#include <zlog.h>

#include "mister/mister.h"
#include "test_util.h"

TEST_CASE("happy PUBLISH packet", "[publish][happy]") {
    dzlog_init("", "mr_init"); // enables logging from the mister library and here
    // *** common test prolog ***

    mr_packet_ctx *pctx;
    char printable_filename[50];
    char packet_filename[50];

    // init
    REQUIRE(mr_init_publish_packet(&pctx) == 0);

    // *** test sections ***

    SECTION("default packet") {
        strlcpy(printable_filename, "fixtures/default_publish_printable.txt", 50);
        strlcpy(packet_filename, "fixtures/default_publish_packet.bin", 50);
    }
/*
    SECTION("sub_ids") {
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
 */
    // *** common test epilog ***

    // dump
    char *packet_printable;
    REQUIRE(mr_get_publish_printable(pctx, false, &packet_printable) == 0);
    // REQUIRE(put_binary_file_content(printable_filename, (uint8_t *)packet_printable, strlen(packet_printable) + 1) == 0);
    printf("packet_printable:: strlen: %lu; hexdump:\n", strlen(packet_printable));
    mr_print_hexdump((uint8_t *)packet_printable, strlen(packet_printable) + 1);

    // check dump
    char *file_printable;
    size_t mdsz;
    REQUIRE(get_binary_file_content(printable_filename, (uint8_t **)&file_printable, &mdsz) == 0);
    // printf("\nfile printable (%s)::\n%s\n\npacket printable::\n%s\n", printable_filename, file_printable, packet_printable);
    // printf("packet_printable:: strlen: %lu\n", strlen(packet_printable));
    REQUIRE(mdsz == strlen(packet_printable) + 1);
    REQUIRE(strcmp(file_printable, packet_printable) == 0);
    free(file_printable);

    // pack
    uint8_t *packet_u8v0;
    size_t packet_u8vlen;
    REQUIRE(mr_pack_publish_packet(pctx, &packet_u8v0, &packet_u8vlen) == 0);
    printf("\npacket::\n");
    mr_print_hexdump(packet_u8v0, packet_u8vlen);
    puts("");
    // REQUIRE(put_binary_file_content(packet_filename, packet_u8v0, packet_u8vlen) == 0);

    // check packet
    uint8_t *u8v0;
    size_t u8vlen;
    REQUIRE(get_binary_file_content(packet_filename, &u8v0, &u8vlen) == 0);
    REQUIRE(u8vlen == packet_u8vlen);
    REQUIRE(memcmp(u8v0, packet_u8v0, u8vlen) == 0);
    free(u8v0);

    // free pack context
    REQUIRE(mr_free_publish_packet(pctx) == 0);

    // init unpack context / unpack packet
    REQUIRE(mr_init_unpack_publish_packet(&pctx, u8v0, u8vlen) == 0);
    // uint8_t u8;
    // mr_get_publish_reserved_header(pctx, &u8);
    // printf("reserved_header:\n");
    // mr_print_hexdump(&u8, 1);

    // unpack dump
    REQUIRE(mr_get_publish_printable(pctx, false, &packet_printable) == 0);
    // printf("packet_printable (unpack):: strlen: %lu; hexdump:\n", strlen(packet_printable));
    // mr_print_hexdump((uint8_t *)packet_printable, strlen(packet_printable) + 1);

    // check unpack dump
    REQUIRE(mdsz == strlen(packet_printable) + 1);
    REQUIRE(strcmp(file_printable, packet_printable) == 0);
    free(file_printable);

    REQUIRE(mr_get_publish_printable(pctx, true, &packet_printable) == 0); // test true flag
    // printf("\npacket_printable::\n%s\n", packet_printable);

    // free packet context
    REQUIRE(mr_free_publish_packet(pctx) == 0);

    zlog_fini();
}
