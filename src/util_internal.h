#ifndef UTIL_INTERNAL_H
#define UTIL_INTERNAL_H

//#include "mister/mister.h"

int utf8val(const uint8_t *u8v, size_t len);
int mr_bytecount_VBI(uint32_t u32);
int mr_make_VBI(uint32_t u32, uint8_t *u8v0);
int mr_get_VBI(uint32_t *pu32, uint8_t *u8v);

#endif /* UTIL_INTERNAL_H */