#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "util.h"

int get_binary_file_content(const char *fixfilename, uint8_t **pu8v, size_t *pffsz) {
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

int put_binary_file_content(const char *fixfilename, uint8_t *u8v, size_t ffsz) {
    FILE *fixfile;
    fixfile = fopen(fixfilename, "w");
    if (fixfile == NULL) return -1;
    fwrite(u8v, 1, ffsz, fixfile);
    fclose(fixfile);
    return 0;
}
