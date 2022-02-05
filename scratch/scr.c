/*
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
 */
#include <string.h>

#include "mister/mister.h"
#include "mister/mrzlog.h"
#include "util.h"

int get_binary_file_content(const char *fixfilename, uint8_t **pu8v, uint32_t *pffsz) {
    FILE *fixfile;
    fixfile = fopen(fixfilename, "r");
    if (fixfile == NULL) return -1;
    fseek(fixfile, 0, SEEK_END);
    *pffsz = ftell(fixfile);
    fseek(fixfile, 0, SEEK_SET);
    *pu8v = (uint8_t *)calloc(*pffsz, 1);
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

int main(void) {
    // *** common test prolog ***

    dzlog_init("", "mr_init");
    int rc;
    mr_packet_ctx *pctx;
    char dump_filename[50];
    char packet_filename[50];

    // init
    mr_init_connect_pctx(&pctx);

    // *** test sections ***

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

    mr_set_connect_will_flag(pctx, true);
    mr_set_connect_will_qos(pctx, 2);
    mr_set_connect_will_retain(pctx, true);
    mr_set_connect_will_delay_interval(pctx, 1000);
    mr_set_connect_payload_format_indicator(pctx, 1);
    mr_set_connect_message_expiry_interval(pctx, 1000);

    mr_set_connect_content_type(pctx, content_type);
    mr_set_connect_response_topic(pctx, response_topic);
    mr_set_connect_correlation_data(pctx, correlation_data, correlation_data_len);
    mr_set_connect_will_user_properties(pctx, will_user_properties, will_user_properties_len);
    mr_set_connect_will_topic(pctx, will_topic);
    mr_set_connect_will_payload(pctx, will_payload, will_payload_len);

    strlcpy(dump_filename, "../tests/fixtures/will_connect_mdata_dump.txt", 50);
    strlcpy(packet_filename, "../tests/fixtures/will_connect_packet.bin", 50);

    // *** common test epilog ***

    // validate
    mr_validate_connect_values(pctx);

    // dump
    mr_connect_printable_mdata(pctx, false);
    // put_binary_file_content(dump_filename, (uint8_t *)pctx->printable_mdata, strlen(pctx->printable_mdata) + 1);

    // check dump
    char *printable_mdata;
    uint32_t mdsz;
    get_binary_file_content(dump_filename, (uint8_t **)&printable_mdata, &mdsz);
    printf("\nmdata_dump (%s)::\n%s\n\npctx->mdata::\n%s\n", dump_filename, printable_mdata, pctx->printable_mdata);
    strcmp(printable_mdata, pctx->printable_mdata);
    free(printable_mdata);

    // pack
    mr_pack_connect_packet(pctx);
    printf("\n\npacket::\n");
    mr_print_hexdump(pctx->u8v0, pctx->u8vlen);
    // put_binary_file_content(packet_filename, pctx->u8v0, pctx->u8vlen);

    // check packet
    uint8_t *u8v0;
    uint32_t u8vlen;
    get_binary_file_content(packet_filename, &u8v0, &u8vlen);
    free(u8v0);

    // free pack context
    mr_free_connect_pctx(pctx);

    // init unpack context / unpack packet
    mr_init_unpack_connect_packet(&pctx, u8v0, u8vlen);

    // unpack dump
    mr_connect_printable_mdata(pctx, false);

    // check unpack dump
    strcmp(printable_mdata, pctx->printable_mdata);
    free(printable_mdata);

    // free unpack context
    mr_free_connect_pctx(pctx);

    zlog_fini();
}
