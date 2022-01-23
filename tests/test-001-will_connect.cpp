#include <catch2/catch.hpp>
#include "mister/mister.h"
#include "mister/mrzlog.h"
#include "util.h"

TEST_CASE("will CONNECT packet", "[connect]") {
    dzlog_init("", "mr_init");
    int rc;
    packet_ctx *pctx;

    // init
    int rc00 = mr_init_connect_pctx(&pctx);
    REQUIRE(rc00 == 0);

    // set values
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
    REQUIRE(rc05 == 0);

    // validate
    int rc07 = mr_validate_connect_will_values(pctx);
    REQUIRE(rc07 == 0);

    // dump
    int rc10 = mr_connect_mdata_dump(pctx);
    REQUIRE(rc10 == 0);
    // int rc = put_binary_file_content("fixtures/will_connect_mdata_dump.txt", (uint8_t *)pctx->mdata_dump, strlen(pctx->mdata_dump));
    // REQUIRE(rc == 0);

    // check dump
    char *mdata_dump;
    uint32_t mdsz;
    rc = get_binary_file_content("fixtures/will_connect_mdata_dump.txt", (uint8_t **)&mdata_dump, &mdsz);
    REQUIRE(rc == 0);
    REQUIRE(mdsz == strlen(pctx->mdata_dump));
    REQUIRE(strncmp(mdata_dump, pctx->mdata_dump, mdsz) == 0);
    free(mdata_dump);

    // pack
    int rc20 = mr_pack_connect_packet(pctx);
    mr_print_hexdump(pctx->u8v0, pctx->u8vlen);
    // int rc = put_binary_file_content("fixtures/will_connect_packet.bin", pctx->u8v0, pctx->u8vlen);
    // REQUIRE(rc == 0);
    REQUIRE(rc20 == 0);

    // check packet
    uint8_t *u8v0;
    uint32_t u8vlen;
    rc = get_binary_file_content("fixtures/will_connect_packet.bin", &u8v0, &u8vlen);
    REQUIRE(rc == 0);
    REQUIRE(u8vlen == pctx->u8vlen);
    REQUIRE(memcmp(u8v0, pctx->u8v0, u8vlen) == 0);
    free(u8v0);

    // free context
    int rc30 = mr_free_connect_pctx(pctx);
    REQUIRE(rc30 == 0);

    // init context / unpack packet
    int rc40 = mr_init_unpack_connect_packet(&pctx, u8v0, u8vlen);
    REQUIRE(rc40 == 0);

    // dump
    int rc50 = mr_connect_mdata_dump(pctx);
    REQUIRE(rc50 == 0);

    // check dump
    REQUIRE(mdsz == strlen(pctx->mdata_dump));
    REQUIRE(strncmp(mdata_dump, pctx->mdata_dump, mdsz) == 0);
    free(mdata_dump);

    // free context
    int rc60 = mr_free_connect_pctx(pctx);
    REQUIRE(rc60 == 0);

    zlog_fini();
}