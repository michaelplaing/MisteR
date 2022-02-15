#include <catch2/catch.hpp>
#include <zlog.h>

#include "mister/mister.h"
#include "test_util.h"

static char _S0L[] = "";

TEST_CASE("happy CONNACK packet", "[connack][happy]") {
    dzlog_init("", "mr_init"); // enables logging from the mister library and here
    // *** common test prolog ***

    mr_packet_ctx *pctx;
    char printable_filename[50];
    char packet_filename[50];

    // init
    REQUIRE(mr_init_connack_packet(&pctx) == 0);

    // *** test sections ***

    SECTION("default packet") {
        strlcpy(printable_filename, "fixtures/default_connack_printable.txt", 50);
        strlcpy(packet_filename, "fixtures/default_connack_packet.bin", 50);
    }

    SECTION("complex packet") {
        // *** section prolog ***

        // build vector values
        char assigned_client_identifier[] = "assigned_client_identifier";
        char reason_string[] = "reason_string";
        char baz[] = "baz";
        char bip[] = "bip";
        mr_string_pair spbaz = {baz, bip};
        char bam[] = "bam";
        char boop[] = "boop";
        mr_string_pair spbam = {bam, boop};
        mr_string_pair user_properties[] = {spbaz, spbam};
        size_t user_properties_len = 2;
        char response_information[] = "response_information";
        char server_reference[] = "server_reference";
        char authentication_method[] = "authentication_method";
        uint8_t authentication_data[] = {'a', 'b', 'c'};
        size_t authentication_data_len = 3;

        REQUIRE(mr_set_connack_session_present(pctx, true) == 0);
        REQUIRE(mr_set_connack_connect_reason_code(pctx, MQTT_RC_CONNECTION_RATE_EXCEEDED) == 0);
        REQUIRE(mr_set_connack_session_expiry_interval(pctx, 1000) == 0);
        REQUIRE(mr_set_connack_receive_maximum(pctx, 10) == 0);
        REQUIRE(mr_set_connack_maximum_qos(pctx, 1) == 0);
        REQUIRE(mr_set_connack_retain_available(pctx, 1) == 0);
        REQUIRE(mr_set_connack_maximum_packet_size(pctx, 10000) == 0);
        REQUIRE(mr_set_connack_assigned_client_identifier(pctx, assigned_client_identifier) == 0);
        REQUIRE(mr_set_connack_topic_alias_maximum(pctx, 10) == 0);
        REQUIRE(mr_set_connack_reason_string(pctx, reason_string) == 0);
        REQUIRE(mr_set_connack_user_properties(pctx, user_properties, user_properties_len) == 0);
        REQUIRE(mr_set_connack_wildcard_subscription_available(pctx, 1) == 0);
        REQUIRE(mr_set_connack_subscription_identifiers_available(pctx, 1) == 0);
        REQUIRE(mr_set_connack_shared_subscription_available(pctx, 1) == 0);
        REQUIRE(mr_set_connack_server_keep_alive(pctx, 1000) == 0);
        REQUIRE(mr_set_connack_response_information(pctx, response_information) == 0);
        REQUIRE(mr_set_connack_server_reference(pctx, server_reference) == 0);
        REQUIRE(mr_set_connack_authentication_method(pctx, authentication_method) == 0);
        REQUIRE(mr_set_connack_authentication_data(pctx, authentication_data, authentication_data_len) == 0);

        SECTION("+remaining") { // default + remaining
            strlcpy(printable_filename, "fixtures/complex_connack_printable.txt", 50);
            strlcpy(packet_filename, "fixtures/complex_connack_packet.bin", 50);
        }

        SECTION("-remaining") { // default + remaining - remaining
            strlcpy(printable_filename, "fixtures/default_connack_printable.txt", 50);
            strlcpy(packet_filename, "fixtures/default_connack_packet.bin", 50);

            REQUIRE(mr_set_connack_session_present(pctx, false) == 0);
            REQUIRE(mr_set_connack_connect_reason_code(pctx, 0) == 0);
            REQUIRE(mr_reset_connack_session_expiry_interval(pctx) == 0);
            REQUIRE(mr_reset_connack_receive_maximum(pctx) == 0);
            REQUIRE(mr_reset_connack_maximum_qos(pctx) == 0);
            REQUIRE(mr_reset_connack_retain_available(pctx) == 0);
            REQUIRE(mr_reset_connack_maximum_packet_size(pctx) == 0);
            REQUIRE(mr_reset_connack_assigned_client_identifier(pctx) == 0);
            REQUIRE(mr_reset_connack_topic_alias_maximum(pctx) == 0);
            REQUIRE(mr_reset_connack_reason_string(pctx) == 0);
            REQUIRE(mr_reset_connack_user_properties(pctx) == 0);
            REQUIRE(mr_reset_connack_wildcard_subscription_available(pctx) == 0);
            REQUIRE(mr_reset_connack_subscription_identifiers_available(pctx) == 0);
            REQUIRE(mr_reset_connack_shared_subscription_available(pctx) == 0);
            REQUIRE(mr_reset_connack_server_keep_alive(pctx) == 0);
            REQUIRE(mr_reset_connack_response_information(pctx) == 0);
            REQUIRE(mr_reset_connack_server_reference(pctx) == 0);
            REQUIRE(mr_reset_connack_authentication_method(pctx) == 0);
            REQUIRE(mr_reset_connack_authentication_data(pctx) == 0);
        }
    }
    // *** common test epilog ***

    // dump
    char *packet_printable;
    REQUIRE(mr_get_connack_printable(pctx, false, &packet_printable) == 0);
    // REQUIRE(put_binary_file_content(printable_filename, (uint8_t *)packet_printable, strlen(packet_printable) + 1) == 0);

    // check dump
    char *file_printable;
    size_t mdsz;
    REQUIRE(get_binary_file_content(printable_filename, (uint8_t **)&file_printable, &mdsz) == 0);
    // printf("\nfile printable (%s)::\n%s\n\npacket printable::\n%s\n", printable_filename, file_printable, packet_printable);
    REQUIRE(mdsz == strlen(packet_printable) + 1);
    REQUIRE(strcmp(file_printable, packet_printable) == 0);
    free(file_printable);

    // pack
    uint8_t *packet_u8v0;
    size_t packet_u8vlen;
    REQUIRE(mr_pack_connack_packet(pctx, &packet_u8v0, &packet_u8vlen) == 0);
    // printf("\npacket::\n");
    // mr_print_hexdump(packet_u8v0, packet_u8vlen);
    // puts("");
    // REQUIRE(put_binary_file_content(packet_filename, packet_u8v0, packet_u8vlen) == 0);

    // check packet
    uint8_t *u8v0;
    size_t u8vlen;
    REQUIRE(get_binary_file_content(packet_filename, &u8v0, &u8vlen) == 0);
    REQUIRE(u8vlen == packet_u8vlen);
    REQUIRE(memcmp(u8v0, packet_u8v0, u8vlen) == 0);
    free(u8v0);

    // free pack context
    REQUIRE(mr_free_connack_packet(pctx) == 0);

    // init unpack context / unpack packet
    REQUIRE(mr_init_unpack_connack_packet(&pctx, u8v0, u8vlen) == 0);

    // unpack dump
    REQUIRE(mr_get_connack_printable(pctx, false, &packet_printable) == 0);

    // check unpack dump
    REQUIRE(mdsz == strlen(packet_printable) + 1);
    REQUIRE(strcmp(file_printable, packet_printable) == 0);
    free(file_printable);

    REQUIRE(mr_get_connack_printable(pctx, true, &packet_printable) == 0); // test true flag
    printf("\npacket_printable::\n%s\n", packet_printable);

    // free unpack context
    REQUIRE(mr_free_connack_packet(pctx) == 0);

    zlog_fini();
}

TEST_CASE("unhappy CONNACK packet", "[connack][unhappy]") {
    dzlog_init("", "mr_init");

    // *** common test prolog ***

    // get the complex packet and unpack it so we have a full deck to play with
    mr_packet_ctx *pctx;
    uint8_t *u8v0;
    size_t u8vlen;
    REQUIRE(get_binary_file_content("fixtures/complex_connack_packet.bin", &u8v0, &u8vlen) == 0);
    REQUIRE(mr_init_unpack_connack_packet(&pctx, u8v0, u8vlen) == 0);
    // puts("");
    // mr_print_hexdump(u8v0, u8vlen);

    // *** test sections ***

    SECTION("connect_reason_code") {
        CHECK(mr_set_connack_connect_reason_code(pctx, -1) == -1);
        CHECK(mr_set_connack_connect_reason_code(pctx, 0) == 0);
    }

    SECTION("receive_maximum") {
        CHECK(mr_set_connack_receive_maximum(pctx, 0) == -1);
        CHECK(mr_set_connack_receive_maximum(pctx, 1) == 0);
    }

    SECTION("maximum_qos") {
        CHECK(mr_set_connack_maximum_qos(pctx, -1) == -1);
        CHECK(mr_set_connack_maximum_qos(pctx, 0) == 0);
        CHECK(mr_set_connack_maximum_qos(pctx, 1) == 0);
        CHECK(mr_set_connack_maximum_qos(pctx, 2) == -1);
    }

    SECTION("retain_available") {
        CHECK(mr_set_connack_retain_available(pctx, -1) == -1);
        CHECK(mr_set_connack_retain_available(pctx, 0) == 0);
        CHECK(mr_set_connack_retain_available(pctx, 1) == 0);
        CHECK(mr_set_connack_retain_available(pctx, 2) == -1);
    }

    SECTION("maximum_packet_size") {
        CHECK(mr_set_connack_maximum_packet_size(pctx, 0) == -1);
        CHECK(mr_set_connack_maximum_packet_size(pctx, 1) == 0);
    }

    SECTION("wildcard_subscription_available") {
        CHECK(mr_set_connack_wildcard_subscription_available(pctx, -1) == -1);
        CHECK(mr_set_connack_wildcard_subscription_available(pctx, 0) == 0);
        CHECK(mr_set_connack_wildcard_subscription_available(pctx, 1) == 0);
        CHECK(mr_set_connack_wildcard_subscription_available(pctx, 2) == -1);
    }

    SECTION("subscription_identifiers_available") {
        CHECK(mr_set_connack_subscription_identifiers_available(pctx, -1) == -1);
        CHECK(mr_set_connack_subscription_identifiers_available(pctx, 0) == 0);
        CHECK(mr_set_connack_subscription_identifiers_available(pctx, 1) == 0);
        CHECK(mr_set_connack_subscription_identifiers_available(pctx, 2) == -1);
    }

    SECTION("shared_subscription_available") {
        CHECK(mr_set_connack_shared_subscription_available(pctx, -1) == -1);
        CHECK(mr_set_connack_shared_subscription_available(pctx, 0) == 0);
        CHECK(mr_set_connack_shared_subscription_available(pctx, 1) == 0);
        CHECK(mr_set_connack_shared_subscription_available(pctx, 2) == -1);
    }

    // common test epilog

    // free packet context
    REQUIRE(mr_free_connack_packet(pctx) == 0);
    free(u8v0);

    zlog_fini();
}
