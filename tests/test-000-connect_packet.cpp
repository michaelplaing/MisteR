#include <catch2/catch.hpp>
#include "mister/mister.h"
#include "mister/mrzlog.h"
#include "util.h"

TEST_CASE("default CONNECT packet", "[connect][happy]") {
    // *** common test prolog ***

    dzlog_init("", "mr_init");
    int rc;
    mr_packet_ctx *pctx;
    char dump_filename[50];
    char packet_filename[50];

    // init
    int rc00 = mr_init_connect_pctx(&pctx);
    REQUIRE(rc00 == 0);

    // *** test sections ***

    SECTION("default packet") {
        strlcpy(dump_filename, "fixtures/default_connect_mdata_dump.txt", 50);
        strlcpy(packet_filename, "fixtures/default_connect_packet.bin", 50);
    }

    SECTION("will/complex packet") {
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
        REQUIRE(mr_set_connect_will_property_length(pctx, 0) == 0); // sets vexists; value always calculated
        REQUIRE(mr_set_connect_will_delay_interval(pctx, 1000) == 0);
        REQUIRE(mr_set_connect_payload_format_indicator(pctx, 1) == 0);
        REQUIRE(mr_set_connect_message_expiry_interval(pctx, 1000) == 0);

        REQUIRE(mr_set_connect_content_type(pctx, content_type) == 0);
        REQUIRE(mr_set_connect_response_topic(pctx, response_topic) == 0);
        REQUIRE(mr_set_connect_correlation_data(pctx, correlation_data, correlation_data_len) == 0);
        REQUIRE(mr_set_connect_will_user_properties(pctx, will_user_properties, will_user_properties_len) == 0);
        REQUIRE(mr_set_connect_will_topic(pctx, will_topic) == 0);
        REQUIRE(mr_set_connect_will_payload(pctx, will_payload, will_payload_len) == 0);

        SECTION("will packet") { // default + will
            strlcpy(dump_filename, "fixtures/will_connect_mdata_dump.txt", 50);
            strlcpy(packet_filename, "fixtures/will_connect_packet.bin", 50);
        }

        SECTION("complex packet") { // default + will + all remaining
            strlcpy(dump_filename, "fixtures/complex_connect_mdata_dump.txt", 50);
            strlcpy(packet_filename, "fixtures/complex_connect_packet.bin", 50);

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
            REQUIRE(mr_set_connect_password_flag(pctx, true) == 0);
            REQUIRE(mr_set_connect_username_flag(pctx, true) == 0);
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
        }
    }

    // *** common test epilog ***

    // dump
    int rc10 = mr_connect_mdata_dump(pctx);
    REQUIRE(rc10 == 0);

    // if (strcmp(dump_filename, "fixtures/will_connect_mdata_dump.txt") == 0) {
    //     rc = put_binary_file_content(dump_filename, (uint8_t *)pctx->mdata_dump, strlen(pctx->mdata_dump));
    //     REQUIRE(rc == 0);
    // }

    // check dump
    char *mdata_dump;
    uint32_t mdsz;
    rc = get_binary_file_content(dump_filename, (uint8_t **)&mdata_dump, &mdsz);
    REQUIRE(rc == 0);
    printf("\npctx->mdata_dump (packet)::\n");
    mr_print_hexdump((uint8_t *)pctx->mdata_dump, strlen(pctx->mdata_dump));
    printf("\n\nmdata_dump (%s)::\n", dump_filename);
    mr_print_hexdump((uint8_t *)mdata_dump, mdsz);
    REQUIRE(mdsz == strlen(pctx->mdata_dump));
    REQUIRE(strncmp(mdata_dump, pctx->mdata_dump, mdsz) == 0);
    free(mdata_dump);

    // pack
    int rc20 = mr_pack_connect_packet(pctx);
    mr_print_hexdump(pctx->u8v0, pctx->u8vlen);
    // rc = put_binary_file_content(packet_filename, pctx->u8v0, pctx->u8vlen);
    // REQUIRE(rc == 0);
    REQUIRE(rc20 == 0);

    // check packet
    uint8_t *u8v0;
    uint32_t u8vlen;
    rc = get_binary_file_content(packet_filename, &u8v0, &u8vlen);
    REQUIRE(rc == 0);
    REQUIRE(u8vlen == pctx->u8vlen);
    REQUIRE(memcmp(u8v0, pctx->u8v0, u8vlen) == 0);
    free(u8v0);

    // free pack context
    int rc30 = mr_free_connect_pctx(pctx);
    REQUIRE(rc30 == 0);

    // init unpack context / unpack packet
    int rc40 = mr_init_unpack_connect_packet(&pctx, u8v0, u8vlen);
    REQUIRE(rc40 == 0);

    // unpack dump
    int rc50 = mr_connect_mdata_dump(pctx);
    REQUIRE(rc50 == 0);

    // check unpack dump
    REQUIRE(mdsz == strlen(pctx->mdata_dump));
    REQUIRE(strncmp(mdata_dump, pctx->mdata_dump, mdsz) == 0);
    free(mdata_dump);

    // free unpack context
    int rc60 = mr_free_connect_pctx(pctx);
    REQUIRE(rc60 == 0);

    zlog_fini();
}