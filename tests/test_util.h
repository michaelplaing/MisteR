// test_util.h

#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

int get_binary_file_content(const char *fixfilename, uint8_t **pu8v, size_t *pffsz);
int put_binary_file_content(const char *fixfilename, uint8_t *u8v, size_t ffsz);

#ifdef __cplusplus
}
#endif

#endif // TEST_UTIL_H
