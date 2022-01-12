#ifndef UTIL_H
#define UTIL_H

int print_hexdump(const uint8_t *u8v, const size_t len);
int get_hexdump(char *cv0, const size_t clen, const uint8_t *u8v, const size_t ulen);
void compress_spaces_lines(char *cv);

#endif /* UTIL_H */