#include <unistd.h>
#include <catch2/catch.hpp>
#include "mister/mister.h"
#include "util.h"

TEST_CASE("complex CONNECT packet", "[connect]") {
    packet_ctx *pctx;
    int rc00 = mr_init_connect_packet(&pctx);
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

    int rc05 = mr_set_connect_will_values(pctx, &wd);

    // int rc10 = mr_connect_mdata_dump(pctx);
    // int rc20 = mr_pack_connect_packet(pctx);
    // int rc30 = mr_free_connect_packet(pctx);

    SECTION("connect packet set will values succeeds") {
        REQUIRE(rc00 == 0);
        REQUIRE(rc05 == 0);
    }
    SECTION("connect packet validate will values succeeds") {
        REQUIRE(rc00 == 0);
        REQUIRE(rc05 == 0);
        int rc07 = mr_validate_connect_will_values(pctx);
        REQUIRE(rc07 == 0);
    }
    SECTION("connect will mdata_dump succeeds") {
        int rc10 = mr_connect_mdata_dump(pctx);
        REQUIRE(rc10 == 0);
        // int rc = put_binary_file_content(
        //    "fixtures/complex_connect_will_mdata_dump.txt", (uint8_t *)pctx->mdata_dump, strlen(pctx->mdata_dump)
        //);
}
    SECTION("connect will mdata_dump is correct") {
        int rc10 = mr_connect_mdata_dump(pctx);
        REQUIRE(rc10 == 0);
        char *mdata_dump;
        uint32_t mdsz;
        int rc = get_binary_file_content("fixtures/will_connect_mdata_dump.txt", (uint8_t **)&mdata_dump, &mdsz);
        REQUIRE(rc == 0);
        REQUIRE(mdsz == strlen(pctx->mdata_dump));
        REQUIRE(strncmp(mdata_dump, pctx->mdata_dump, mdsz) == 0);
        free(mdata_dump);
    }
    SECTION("pack connect packet succeeds") {
        int rc20 = mr_pack_connect_packet(pctx);
        mr_print_hexdump(pctx->u8v0, pctx->u8vlen);
        // int rc = put_binary_file_content("fixtures/will_connect_packet.bin", pctx->u8v0, pctx->u8vlen);
        REQUIRE(rc20 == 0);
    }
    SECTION("packed connect packet is correct") {
        int rc20 = mr_pack_connect_packet(pctx);
        REQUIRE(rc20 == 0);
        uint8_t *u8v0;
        uint32_t u8vlen;
        int rc = get_binary_file_content("fixtures/will_connect_packet.bin", &u8v0, &u8vlen);
        REQUIRE(rc == 0);
        REQUIRE(u8vlen == pctx->u8vlen);
        REQUIRE(memcmp(u8v0, pctx->u8v0, u8vlen) == 0);
        free(u8v0);
    }
    SECTION("free connect packet succeeds") {
        int rc30 = mr_free_connect_packet(pctx);
        REQUIRE(rc30 == 0);
    }
}