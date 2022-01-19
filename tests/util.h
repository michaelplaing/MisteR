// util.h

#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

int get_binary_file_content(const char *fixfilename, uint8_t **pu8v, uint32_t *pffsz);
int put_binary_file_content(const char *fixfilename, uint8_t *u8v, uint32_t ffsz);

#ifdef __cplusplus
}
#endif

#endif // UTIL_H
