#ifndef UTIL_INTERNAL_H
#define UTIL_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

int utf8val(const uint8_t *u8v, size_t len);
int mr_bytecount_VBI(uint32_t u32);
int mr_make_VBI(uint32_t u32, uint8_t *u8v0);
int mr_get_VBI(uint32_t *pu32, uint8_t *u8v);

#ifdef __cplusplus
}
#endif

#endif /* UTIL_INTERNAL_H */