#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>
// #include <ctype.h>
// #include <stdbool.h>
#include <math.h>

#include <string.h>
// #include <zlog.h>

#define _BASE 62

void itoa(uint64_t u64, char* cv) {
    if (u64 == 0) {
        cv[0] = '0';
        cv[1] = '\0';
    }

    int pos;
    const int len = (uint64_t)floor(log(u64) / log(_BASE)) + 1;
    for (pos = 0; pos < len; pos++) {
        uint64_t base = (uint64_t)pow(_BASE, len - 1 - pos);
        uint64_t offset = u64 / base;

        char offset_char;
        if (offset < 10) {
            offset_char = offset + '0';
        }
        else if (offset < 36) {
            offset_char = offset - 10 + 'A';
        }
        else {
            offset_char = offset - 36 + 'a';
        }

        cv[pos] = offset_char;
        u64 -= base * offset;
    }

    cv[pos] = '\0';
}

int main() {
    char cv[256];

    uint64_t d = 62;
    itoa(d, cv);
    printf("Input = %llu, cv = %s\n", d, cv);

    uint64_t a = UINT64_MAX; // 18446744073709551615
    itoa(a, cv);
    printf("Input = %llu, cv = %s\n", a, cv);

    uint64_t b = 0;
    itoa(b, cv);
    printf("Input = %llu, cv = %s\n", b, cv);

    uuid_t uuidv;
    uuid_generate(uuidv);
    uint64_t slice_u64;
    char uuid_cv[256] = "";

    for (int i = 0; i < 2; i++) {
        slice_u64 = 0;
        for (int j = 0; j < 8; j++) slice_u64 += (uuidv[i * 4 + j] << j * 8);
        itoa(slice_u64, cv);
        strcat(uuid_cv, cv);
        printf("uuid slice %d: %llu; cv: %s\n", i, slice_u64, cv);
    }

    printf("uuid_cv: %s\n", uuid_cv);
    return 0;
}
