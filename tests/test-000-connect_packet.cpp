#include <catch2/catch.hpp>
#include "mister/mister.h"
#include "mister/mrzlog.h"
#include "util.h"

TEST_CASE("default CONNECT packet", "[connect][happy]") {
    // *** common test prolog ***

    dzlog_init("", "mr_init");
    int rc;
    packet_ctx *pctx;
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

    SECTION("will packet") {
        strlcpy(dump_filename, "fixtures/will_connect_mdata_dump.txt", 50);
        strlcpy(packet_filename, "fixtures/will_connect_packet.bin", 50);

        // build will values
        char content_type[] = "content_type";
        char response_topic[] = "response_topic";
        uint8_t correlation_data[] = {'1', '2', '3'};
        char baz[] = "baz";
        char bip[] = "bip";
        string_pair spbaz = {baz, bip};
        char bam[] = "bam";
        char boop[] = "boop";
        string_pair spbam = {bam, boop};
        string_pair will_user_properties[] = {spbaz, spbam};
        char will_topic[] = "will_topic";
        uint8_t will_payload[] = {'a', 'b', 'c'};

        // set will data
        mr_connect_will_data wd = {
            .will_flag = true,
            .will_qos = 2,
            .will_retain = true,
            .will_property_length = 0,
            .will_delay_interval = 1000,
            .payload_format_indicator = 1,
            .message_expiry_interval = 1000,
            .content_type = content_type,
            .response_topic = response_topic,
            .correlation_data = correlation_data,
            .correlation_data_len = sizeof(correlation_data),
            .will_user_properties = will_user_properties,
            .will_user_properties_len = sizeof(will_user_properties) / sizeof(string_pair),
            .will_topic = will_topic,
            .will_payload = will_payload,
            .will_payload_len = sizeof(will_payload)
        };

        // validate will data
        int rc02 = mr_validate_connect_will_data(pctx, &wd);
        REQUIRE(rc02 == 0);
        int rc03 = mr_validate_connect_will_data_utf8(pctx, &wd);
        REQUIRE(rc03 == 0);

        // set will values
        int rc04 = mr_set_connect_will_values(pctx, &wd);
        REQUIRE(rc04 == 0);

        // get will values
        mr_connect_will_data wd2;
        int rc06 = mr_get_connect_will_values(pctx, &wd2);
        REQUIRE(rc06 == 0);

        // revalidate will data
        int rc08 = mr_validate_connect_will_data(pctx, &wd2);
        REQUIRE(rc08 == 0);
        int rc09 = mr_validate_connect_will_data_utf8(pctx, &wd2);
        REQUIRE(rc09 == 0);

        SECTION("complex packet") {
            strlcpy(dump_filename, "fixtures/complex_connect_mdata_dump.txt", 50);
            strlcpy(packet_filename, "fixtures/complex_connect_packet.bin", 50);

            int rc100 = mr_set_connect_clean_start(pctx, true);
            REQUIRE(rc100 == 0);

            int rc110 = mr_set_connect_keep_alive(pctx, 5);
            REQUIRE(rc110 == 0);

            int rc120 = mr_set_connect_session_expiry_interval(pctx, 3600);
            REQUIRE(rc120 == 0);

            int rc130 = mr_set_connect_receive_maximum(pctx, 1000);
            REQUIRE(rc130 == 0);

            int rc140 = mr_set_connect_maximum_packet_size(pctx, 32768);
            REQUIRE(rc140 == 0);

            int rc150 = mr_set_connect_topic_alias_maximum(pctx, 10);
            REQUIRE(rc150 == 0);

            int rc160 = mr_set_connect_request_response_information(pctx, 1);
            REQUIRE(rc160 == 0);

            int rc170 = mr_set_connect_request_problem_information(pctx, 1);
            REQUIRE(rc170 == 0);

            char flim[] = "flim";
            char flop[] = "flop";
            string_pair spflim = {flim, flop};
            char flam[] = "flam";
            char floop[] = "floop";
            string_pair spflam = {flam, floop};
            string_pair user_properties[] = {spflim, spflam};
            int rc180 = mr_set_connect_user_properties(pctx, user_properties, sizeof(user_properties) / sizeof(string_pair));
            REQUIRE(rc180 == 0);

            char authentication_method[] = "authentication_method";
            int rc190 = mr_set_connect_authentication_method(pctx, authentication_method);
            REQUIRE(rc190 == 0);

            uint8_t authentication_data[] = {'d', 'e', 'f'};
            int rc200 = mr_set_connect_authentication_data(pctx, authentication_data, sizeof(authentication_data));
            REQUIRE(rc200 == 0);

            char client_identifier[] = "client_identifier";
            int rc210 = mr_set_connect_client_identifier(pctx, client_identifier);
            REQUIRE(rc210 == 0);

            char user_name[] = "user_name";
            int rc220 = mr_set_connect_user_name(pctx, user_name);
            REQUIRE(rc220 == 0);

            uint8_t password[] = {'g', 'h', 'i'};
            int rc230 = mr_set_connect_password(pctx, password, sizeof(password));
            REQUIRE(rc230 == 0);
        }
    }

    // *** common test epilog ***

    // dump
    int rc10 = mr_connect_mdata_dump(pctx);
    REQUIRE(rc10 == 0);
    // rc = put_binary_file_content(dump_filename, (uint8_t *)pctx->mdata_dump, strlen(pctx->mdata_dump));
    // REQUIRE(rc == 0);

    // check dump
    char *mdata_dump;
    uint32_t mdsz;
    rc = get_binary_file_content(dump_filename, (uint8_t **)&mdata_dump, &mdsz);
    REQUIRE(rc == 0);
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