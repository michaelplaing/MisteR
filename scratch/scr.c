#include <stdio.h>
#include <stdlib.h>
// #include <uuid/uuid.h>
// #include <ctype.h>
// #include <stdbool.h>
// #include <math.h>

#include <string.h>
// #include <zlog.h>

uint32_t unpack_u16(uint8_t *u8v) {
    uint16_t u16v[] = {u8v[0], u8v[1]};
    uint32_t value = (u16v[0] << 8) + u16v[1];
    return value;
}

uint32_t unpack_u16_2(uint8_t *u8v) {
    uint32_t value = (u8v[0] << 8) + u8v[1];
    return value;
}

int main() {
    uint8_t u8v[2] = {'a', 'b'};
    uint32_t v1 = unpack_u16(u8v);
    uint32_t v2 = unpack_u16_2(u8v);
    printf("v1: %u; v2: %u\n", v1, v2);
    return 0;
}
