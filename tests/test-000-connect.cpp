#include <catch2/catch.hpp>
#include <zlog.h>

#include "mister/mister.h"
#include "test_util.h"

static char _S0L[] = "";

TEST_CASE("happy CONNECT packet", "[connect][happy]") {
    dzlog_init("", "mr_init"); // enables logging from the mister library and here
    // dzlog_info("common test prolog");

    // *** common test prolog ***

//    int rc;
    mr_packet_ctx *pctx;
    char printable_filename[50];
    char packet_filename[50];

    // init
    REQUIRE(mr_init_connect_packet(&pctx) == 0);

    // *** test sections ***

    SECTION("default packet") {
        // dzlog_info("section: test default");

        strlcpy(printable_filename, "fixtures/default_connect_printable.txt", 50);
        strlcpy(packet_filename, "fixtures/default_connect_packet.bin", 50);
    }

    SECTION("will/complex packet") {
        // dzlog_info("section: set will");

        // *** section prolog ***

        // build will vector values
        char content_type[] = "content_type";
        char response_topic[] = "response_topic";
        uint8_t correlation_data[] = {'1', '2', '3'};
        size_t correlation_data_len = 3;
        char baz[] = "baz";
        char bip[] = "bip";
        mr_string_pair spbaz = {baz, bip};
        char bam[] = "bam";
        char boop[] = "boop";
        mr_string_pair spbam = {bam, boop};
        mr_string_pair will_user_properties[] = {spbaz, spbam};
        size_t will_user_properties_len = 2;
        char will_topic[] = "will_topic";
        uint8_t will_payload[] = {'a', 'b', 'c'};
        size_t will_payload_len = 3;

        REQUIRE(mr_set_connect_will_flag(pctx, true) == 0);
        REQUIRE(mr_set_connect_will_qos(pctx, 2) == 0);
        REQUIRE(mr_set_connect_will_retain(pctx, true) == 0);
        REQUIRE(mr_set_connect_will_delay_interval(pctx, 1000) == 0);
        REQUIRE(mr_set_connect_payload_format_indicator(pctx, 1) == 0);
        REQUIRE(mr_set_connect_message_expiry_interval(pctx, 1000) == 0);

        REQUIRE(mr_set_connect_content_type(pctx, content_type) == 0);
        REQUIRE(mr_set_connect_response_topic(pctx, response_topic) == 0);
        REQUIRE(mr_set_connect_correlation_data(pctx, correlation_data, correlation_data_len) == 0);
        REQUIRE(mr_set_connect_will_user_properties(pctx, will_user_properties, will_user_properties_len) == 0);
        REQUIRE(mr_set_connect_will_topic(pctx, will_topic) == 0);
        REQUIRE(mr_set_connect_will_payload(pctx, will_payload, will_payload_len) == 0);

        SECTION("+will packet") { // default + will
            // dzlog_info("section: test will");

            strlcpy(printable_filename, "fixtures/will_connect_printable.txt", 50);
            strlcpy(packet_filename, "fixtures/will_connect_packet.bin", 50);
        }

        SECTION("-will packet") { // default + will - will
            // dzlog_info("section: reset/test will");

            strlcpy(printable_filename, "fixtures/default_connect_printable.txt", 50);
            strlcpy(packet_filename, "fixtures/default_connect_packet.bin", 50);

            // reset all the will-related values set above by setting the will_flag to false
            REQUIRE(mr_set_connect_will_flag(pctx, false) == 0);
        }

        SECTION("complex packet") { // default + will + all remaining
            // dzlog_info("section: complex packet");

            // build additional vector values
            char flim[] = "flim";
            char flop[] = "flop";
            mr_string_pair spflim = {flim, flop};
            char flam[] = "flam";
            char floop[] = "floop";
            mr_string_pair spflam = {flam, floop};
            mr_string_pair user_properties[] = {spflim, spflam};
            char authentication_method[] = "authentication_method";
            uint8_t authentication_data[] = {'d', 'e', 'f'};
            char client_identifier[] = "client_identifier";
            char user_name[] = "user_name";
            uint8_t password[] = {'g', 'h', 'i'};

            REQUIRE(mr_set_connect_clean_start(pctx, true) == 0);
            REQUIRE(mr_set_connect_keep_alive(pctx, 5) == 0);
            REQUIRE(mr_set_connect_session_expiry_interval(pctx, 3600) == 0);
            REQUIRE(mr_set_connect_receive_maximum(pctx, 1000) == 0);
            REQUIRE(mr_set_connect_maximum_packet_size(pctx, 32768) == 0);
            REQUIRE(mr_set_connect_topic_alias_maximum(pctx, 10) == 0);
            REQUIRE(mr_set_connect_request_response_information(pctx, 1) == 0);
            REQUIRE(mr_set_connect_request_problem_information(pctx, 1) == 0);

            REQUIRE(mr_set_connect_user_properties(pctx, user_properties, sizeof(user_properties) / sizeof(mr_string_pair)) == 0);
            REQUIRE(mr_set_connect_authentication_method(pctx, authentication_method) == 0);
            REQUIRE(mr_set_connect_authentication_data(pctx, authentication_data, sizeof(authentication_data)) == 0);
            REQUIRE(mr_set_connect_client_identifier(pctx, client_identifier) == 0);
            REQUIRE(mr_set_connect_user_name(pctx, user_name) == 0);
            REQUIRE(mr_set_connect_password(pctx, password, sizeof(password)) == 0);

            SECTION("+complex packet") { // default + will + all remaining
                // dzlog_info("section: set/test complex");

                strlcpy(printable_filename, "fixtures/complex_connect_printable.txt", 50);
                strlcpy(packet_filename, "fixtures/complex_connect_packet.bin", 50);
            }

            SECTION("-complex packet") { // default + will + all remaining - will - all remaining
                // dzlog_info("section: reset/test will & complex");

                strlcpy(printable_filename, "fixtures/default_connect_printable.txt", 50);
                strlcpy(packet_filename, "fixtures/default_connect_packet.bin", 50);

                // reset all the will-related values set above by setting the will_flag to false
                REQUIRE(mr_set_connect_will_flag(pctx, false) == 0);

                // reset the additional values
                REQUIRE(mr_set_connect_clean_start(pctx, false) == 0); // bit_flags always exist ∴ cannot be reset
                REQUIRE(mr_set_connect_keep_alive(pctx, 0) == 0); // keep_alive always exists ∴ cannot be reset
                REQUIRE(mr_reset_connect_session_expiry_interval(pctx) == 0);
                REQUIRE(mr_reset_connect_receive_maximum(pctx) == 0);
                REQUIRE(mr_reset_connect_maximum_packet_size(pctx) == 0);
                REQUIRE(mr_reset_connect_topic_alias_maximum(pctx) == 0);
                REQUIRE(mr_reset_connect_request_response_information(pctx) == 0);
                REQUIRE(mr_reset_connect_request_problem_information(pctx) == 0);

                REQUIRE(mr_reset_connect_user_properties(pctx) == 0);
                REQUIRE(mr_reset_connect_authentication_method(pctx) == 0);
                REQUIRE(mr_reset_connect_authentication_data(pctx) == 0);
                REQUIRE(mr_set_connect_client_identifier(pctx, _S0L) == 0);
                REQUIRE(mr_reset_connect_user_name(pctx) == 0);
                REQUIRE(mr_reset_connect_password(pctx) == 0);
            }
        }
    }

    // dzlog_info("common test epilog");
    // *** common test epilog ***

    // dump
    char *packet_printable;
    REQUIRE(mr_get_connect_printable(pctx, false, &packet_printable) == 0);
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
    REQUIRE(mr_pack_connect_packet(pctx, &packet_u8v0, &packet_u8vlen) == 0);
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
    REQUIRE(mr_free_connect_packet(pctx) == 0);

    // init unpack context / unpack packet
    REQUIRE(mr_init_unpack_connect_packet(&pctx, u8v0, u8vlen) == 0);

    // unpack dump
    REQUIRE(mr_get_connect_printable(pctx, false, &packet_printable) == 0);

    // check unpack dump
    REQUIRE(mdsz == strlen(packet_printable) + 1);
    REQUIRE(strcmp(file_printable, packet_printable) == 0);
    free(file_printable);

    REQUIRE(mr_get_connect_printable(pctx, true, &packet_printable) == 0); // test true flag
    printf("\npacket_printable::\n%s\n", packet_printable);

    // free unpack context
    REQUIRE(mr_free_connect_packet(pctx) == 0);

    zlog_fini();
}

TEST_CASE("unhappy CONNECT packet", "[connect][unhappy]") {
    dzlog_init("", "mr_init");
    // dzlog_info("common test prolog");

    // *** common test prolog ***

    // get the complex packet and unpack it so we have a full deck to play with
    mr_packet_ctx *pctx;
    uint8_t *u8v0;
    size_t u8vlen;
    REQUIRE(get_binary_file_content("fixtures/complex_connect_packet.bin", &u8v0, &u8vlen) == 0);
    REQUIRE(mr_init_unpack_connect_packet(&pctx, u8v0, u8vlen) == 0);
    // puts("");
    // mr_print_hexdump(u8v0, u8vlen);

    // *** test sections ***

    SECTION("will_qos") {
        CHECK(mr_set_connect_will_qos(pctx, -1) == -1);
        CHECK(mr_set_connect_will_qos(pctx, 0) == 0);
        CHECK(mr_set_connect_will_qos(pctx, 1) == 0);
        CHECK(mr_set_connect_will_qos(pctx, 2) == 0);
        CHECK(mr_set_connect_will_qos(pctx, 3) == -1);
    }

    SECTION("receive_maximum") {
        CHECK(mr_set_connect_receive_maximum(pctx, -1) == 0);
        CHECK(mr_set_connect_receive_maximum(pctx, 0) == -1);
        CHECK(mr_set_connect_receive_maximum(pctx, 1) == 0);
    }

    SECTION("request_response_information") {
        CHECK(mr_set_connect_request_response_information(pctx, -1) == -1);
        CHECK(mr_set_connect_request_response_information(pctx, 0) == 0);
        CHECK(mr_set_connect_request_response_information(pctx, 1) == 0);
        CHECK(mr_set_connect_request_response_information(pctx, 2) == -1);
    }

    SECTION("request_problem_information") {
        CHECK(mr_set_connect_request_problem_information(pctx, -1) == -1);
        CHECK(mr_set_connect_request_problem_information(pctx, 0) == 0);
        CHECK(mr_set_connect_request_problem_information(pctx, 1) == 0);
        CHECK(mr_set_connect_request_problem_information(pctx, 2) == -1);
    }

    SECTION("payload_format_indicator & will_payload") {
        // already checked above:
        // . will_flag == false && not payload_format_indicator exists
        // . will_flag == true  && payload_format_indicator exists && == 1 && will_payload is valid utf8
        REQUIRE(mr_set_connect_payload_format_indicator(pctx, -1) == 0);
        CHECK(mr_pack_connect_packet(pctx, &u8v0, &u8vlen) == -1);
        REQUIRE(mr_set_connect_payload_format_indicator(pctx, 0) == 0);
        CHECK(mr_pack_connect_packet(pctx, &u8v0, &u8vlen) == 0);
        REQUIRE(mr_set_connect_payload_format_indicator(pctx, 2) == 0);
        CHECK(mr_pack_connect_packet(pctx, &u8v0, &u8vlen) == -1);

        uint8_t u8v[] = {'\0'}; // invalid utf8 for mqtt
        REQUIRE(mr_set_connect_will_payload(pctx, u8v, 1) == 0);
        REQUIRE(mr_set_connect_payload_format_indicator(pctx, 0) == 0);
        CHECK(mr_pack_connect_packet(pctx, &u8v0, &u8vlen) == 0);
        REQUIRE(mr_set_connect_payload_format_indicator(pctx, 1) == 0); // will_payload is utf8
        CHECK(mr_pack_connect_packet(pctx, &u8v0, &u8vlen) == -1);
    }

    SECTION("authentication_method & authentication_data") {
        REQUIRE(mr_reset_connect_authentication_method(pctx) == 0);
        CHECK(mr_pack_connect_packet(pctx, &u8v0, &u8vlen) == -1);
        REQUIRE(mr_reset_connect_authentication_data(pctx) == 0);
        CHECK(mr_pack_connect_packet(pctx, &u8v0, &u8vlen) == 0);
        REQUIRE(mr_set_connect_authentication_method(pctx, _S0L) == 0);
        CHECK(mr_pack_connect_packet(pctx, &u8v0, &u8vlen) == 0); // ok to have a method and no data
    }

    // common test epilog

    // free packet context
    REQUIRE(mr_free_connect_packet(pctx) == 0);
    free(u8v0);

    zlog_fini();
}
