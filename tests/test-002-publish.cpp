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

    SECTION("complex packet") {
        // *** section prolog ***

        // build vector values
        char topic_name[] = "topic_name";
        char response_topic[] = "response_topic";
        uint8_t correlation_data[] = {'a', 'b', 'c'};
        size_t correlation_data_len = 3;
        char baz[] = "baz";
        char bip[] = "bip";
        mr_string_pair spbaz = {baz, bip};
        char bam[] = "bam";
        char boop[] = "boop";
        mr_string_pair spbam = {bam, boop};
        mr_string_pair user_properties[] = {spbaz, spbam};
        uint32_t subscription_identifiers[] = {1,1000000};
        size_t subscription_identifiers_len = 2;
        size_t user_properties_len = 2;
        char content_type[] = "content_type";
        uint8_t payload[] = {'d', 'e', 'f'};
        size_t payload_len = 3;

        REQUIRE(mr_set_publish_dup(pctx, true) == 0);
        REQUIRE(mr_set_publish_qos(pctx, 2) == 0);
        REQUIRE(mr_set_publish_retain(pctx, true) == 0);
        REQUIRE(mr_set_publish_topic_name(pctx, topic_name) == 0);
        REQUIRE(mr_set_publish_packet_identifier(pctx, 1000) == 0);
        REQUIRE(mr_set_publish_payload_format_indicator(pctx, 1) == 0);
        REQUIRE(mr_set_publish_message_expiry_interval(pctx, 1000000) == 0);
        REQUIRE(mr_set_publish_topic_alias(pctx, 1000) == 0);
        REQUIRE(mr_set_publish_response_topic(pctx, response_topic) == 0);
        REQUIRE(mr_set_publish_correlation_data(pctx, correlation_data, correlation_data_len) == 0);
        REQUIRE(mr_set_publish_user_properties(pctx, user_properties, user_properties_len) == 0);
        REQUIRE(mr_set_publish_subscription_identifiers(pctx, subscription_identifiers, subscription_identifiers_len) == 0);
        REQUIRE(mr_set_publish_content_type(pctx, content_type) == 0);
        REQUIRE(mr_set_publish_payload(pctx, payload, payload_len) == 0);

        SECTION("+remaining") { // default + remaining
            strlcpy(printable_filename, "fixtures/complex_publish_printable.txt", 50);
            strlcpy(packet_filename, "fixtures/complex_publish_packet.bin", 50);
        }

        SECTION("-remaining") { // default + remaining - remaining
            strlcpy(printable_filename, "fixtures/default_publish_printable.txt", 50);
            strlcpy(packet_filename, "fixtures/default_publish_packet.bin", 50);

            REQUIRE(mr_set_publish_dup(pctx, false) == 0);
            REQUIRE(mr_set_publish_qos(pctx, 0) == 0);
            REQUIRE(mr_set_publish_retain(pctx, false) == 0);
            REQUIRE(mr_set_publish_topic_name(pctx, "") == 0);
            REQUIRE(mr_reset_publish_packet_identifier(pctx) == 0);
            REQUIRE(mr_reset_publish_payload_format_indicator(pctx) == 0);
            REQUIRE(mr_reset_publish_message_expiry_interval(pctx) == 0);
            REQUIRE(mr_reset_publish_topic_alias(pctx) == 0);
            REQUIRE(mr_reset_publish_response_topic(pctx) == 0);
            REQUIRE(mr_reset_publish_correlation_data(pctx) == 0);
            REQUIRE(mr_reset_publish_user_properties(pctx) == 0);
            REQUIRE(mr_reset_publish_subscription_identifiers(pctx) == 0);
            REQUIRE(mr_reset_publish_content_type(pctx) == 0);
            REQUIRE(mr_set_publish_payload(pctx, NULL, 0) == 0);
        }
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
