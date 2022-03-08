#include <catch2/catch.hpp>
#include <zlog.h>

#include "mister/mister.h"
#include "test_util.h"

TEST_CASE("happy SUBACK packet", "[suback][happy]") {
    dzlog_init("", "mr_init"); // enables logging from the mister library and here

    // *** common test prolog ***

    mr_packet_ctx *pctx;
    char printable_filename[50];
    char packet_filename[50];

    // init
    REQUIRE(mr_init_suback_packet(&pctx) == 0);
    uint8_t *default_u8v0;
    size_t default_u8v0len;
    bool exists_flag;
    REQUIRE(mr_get_suback_subscribe_reason_codes(pctx, &default_u8v0, &default_u8v0len, &exists_flag) == 0);

    // *** test sections ***

    SECTION("default packet") {
        strlcpy(printable_filename, "fixtures/default_suback_printable.txt", 50);
        strlcpy(packet_filename, "fixtures/default_suback_packet.bin", 50);
    }

    SECTION("complex packet") {
        // *** section prolog ***

        // build vector values
        char baz[] = "baz";
        char bip[] = "bip";
        mr_string_pair spbaz = {baz, bip};
        char bam[] = "bam";
        char boop[] = "boop";
        mr_string_pair spbam = {bam, boop};
        mr_string_pair user_properties[] = {spbaz, spbam};
        size_t user_properties_len = 2;
        uint8_t subscribe_reason_codes[] = {MQTT_RC_TOPIC_FILTER_INVALID, MQTT_RC_PACKET_ID_IN_USE};
        size_t subscribe_reason_codes_len = 2;

        REQUIRE(mr_set_suback_user_properties(pctx, user_properties, user_properties_len) == 0);
        REQUIRE(mr_set_suback_subscribe_reason_codes(pctx, subscribe_reason_codes, subscribe_reason_codes_len) == 0);

        SECTION("+remaining") { // default + remaining
            strlcpy(printable_filename, "fixtures/complex_suback_printable.txt", 50);
            strlcpy(packet_filename, "fixtures/complex_suback_packet.bin", 50);
        }

        SECTION("-remaining") { // default + remaining - remaining
            strlcpy(printable_filename, "fixtures/default_suback_printable.txt", 50);
            strlcpy(packet_filename, "fixtures/default_suback_packet.bin", 50);

            REQUIRE(mr_reset_suback_user_properties(pctx) == 0);
            REQUIRE(mr_set_suback_subscribe_reason_codes(pctx, default_u8v0, default_u8v0len) == 0);
        }
    }

    // *** common test epilog ***

    // printable
    char *packet_printable;
    REQUIRE(mr_get_suback_printable(pctx, false, &packet_printable) == 0);

    // REQUIRE(put_binary_file_content(printable_filename, (uint8_t *)packet_printable, strlen(packet_printable) + 1) == 0);

    // check printable
    // puts("check printable");
    char *file_printable;
    size_t mdsz;
    REQUIRE(get_binary_file_content(printable_filename, (uint8_t **)&file_printable, &mdsz) == 0);
    // printf("\nfile printable (%s)::\n%s\n\npacket printable::\n%s\n", printable_filename, file_printable, packet_printable);
    // printf("\nfile printable (%s) :: mdsz: %lu; packet_printable:: strlen: %lu\n", printable_filename, mdsz, strlen(packet_printable));
    // printf("\nfile_printable:: mdsz: %lu; hexdump:\n", mdsz);
    // mr_print_hexdump((uint8_t *)file_printable, mdsz);
    // printf("packet_printable:: strlen: %lu; hexdump:\n", strlen(packet_printable));
    // mr_print_hexdump((uint8_t *)packet_printable, strlen(packet_printable) + 1);
    REQUIRE(mdsz == strlen(packet_printable) + 1);
    REQUIRE(strcmp(file_printable, packet_printable) == 0);

    // pack
    uint8_t *packet_u8v0;
    size_t packet_u8vlen;
    REQUIRE(mr_pack_suback_packet(pctx, &packet_u8v0, &packet_u8vlen) == 0);
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

    // free pack context
    REQUIRE(mr_free_suback_packet(pctx) == 0);

    // init unpack context / unpack packet
    // printf("***************** mr_init_unpack_suback_packet:: u8vlen: %lu\n", u8vlen);
    REQUIRE(mr_init_unpack_suback_packet(&pctx, u8v0, u8vlen) == 0);
    // uint8_t u8;
    // mr_get_suback_reserved_header(pctx, &u8);
    // printf("reserved_header:\n");
    // mr_print_hexdump(&u8, 1);

    // unpack printable
    REQUIRE(mr_get_suback_printable(pctx, false, &packet_printable) == 0);
    // printf("packet_printable (unpack):: strlen: %lu; hexdump:\n", strlen(packet_printable));
    // mr_print_hexdump((uint8_t *)packet_printable, strlen(packet_printable) + 1);

    // check unpack printable
    // puts("check unpack printable");
    REQUIRE(mdsz == strlen(packet_printable) + 1);
    REQUIRE(strcmp(file_printable, packet_printable) == 0);

    REQUIRE(mr_get_suback_printable(pctx, true, &packet_printable) == 0); // test true flag
    // printf("\npacket_printable::\n%s\n", packet_printable);

    // free packet context
    REQUIRE(mr_free_suback_packet(pctx) == 0);

    zlog_fini();
}

TEST_CASE("unhappy SUBACK packet", "[suback][unhappy]") {
    dzlog_init("", "mr_init");

    // *** common test prolog ***

    // get the complex packet and unpack it so we have a full deck to play with
    mr_packet_ctx *pctx;
    uint8_t *u8v0;
    size_t u8vlen;
    REQUIRE(get_binary_file_content("fixtures/complex_suback_packet.bin", &u8v0, &u8vlen) == 0);
    REQUIRE(mr_init_unpack_suback_packet(&pctx, u8v0, u8vlen) == 0);
    // puts("");
    // mr_print_hexdump(u8v0, u8vlen);

    // *** test sections ***

    SECTION("suback_topic_filters") {
        CHECK(mr_set_suback_subscribe_reason_codes(pctx, NULL, 0) == -1);
        uint8_t subscribe_reason_codes[] = {0xFF};
        size_t subscribe_reason_codes_len = 1;
        CHECK(mr_set_suback_subscribe_reason_codes(pctx, subscribe_reason_codes, subscribe_reason_codes_len) == -1);
    }

    // common test epilog

    // free packet context
    REQUIRE(mr_free_suback_packet(pctx) == 0);

    zlog_fini();
}
