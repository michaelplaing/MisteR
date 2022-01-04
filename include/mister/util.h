#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdbool.h>

#include "mister/connect.h"

int utf8val(const uint8_t *u8v, size_t len);
int mr_make_VBI(uint32_t u32, uint8_t *u8v0);
int mr_get_VBI(uint32_t *pu32, uint8_t *u8v);
int print_hexdump(const uint8_t *u8v, const size_t len);
int get_hexdump(char *cv0, const size_t clen, const uint8_t *u8v, const size_t ulen);

#endif /* UTIL_H */