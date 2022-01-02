#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdbool.h>

#include "mister/connect.h"

int utf8val(const uint8_t *u8v, int len);
int mr_make_VBI(uint32_t u32, uint8_t *u8v0);
int mr_get_VBI(uint32_t *pu32, uint8_t *u8v);

#endif /* UTIL_H */