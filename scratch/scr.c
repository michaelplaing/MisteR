#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>
// #include <ctype.h>
// #include <stdbool.h>
#include <math.h>

#include <string.h>
// #include <zlog.h>

void u64tobase62cv(uint64_t u64, char* cv) {
    if (u64 == 0) {
        cv[0] = '0';
        cv[1] = '\0';
        return;
    }

    const size_t cvlen = floor(log(u64) / log(62)) + 1;
    for (size_t pos = 0; pos < cvlen; pos++) {
        uint64_t power = pow(62, cvlen - 1 - pos);
        uint64_t offset = u64 / power;

        if (offset < 10) {
            cv[pos] = offset + '0';
        }
        else if (offset < 36) {
            cv[pos] = offset - 10 + 'A';
        }
        else { // offset < 62
            cv[pos] = offset - 36 + 'a';
        }

        u64 -= power * offset;
    }

    cv[cvlen] = '\0';
}

void get_uuidbase62cv(char *uuidbase62cv) {
    char base62cv[12];
    uuid_t uuidu8v;
    uuid_generate(uuidu8v);

    uint64_t u64;
    uuidbase62cv[0] = '\0';
    for (int i = 0; i < 2; i++) {
        u64 = 0;
        for (int j = 0; j < 8; j++) u64 += (uuidu8v[i * 8 + j] << 8 * (7 - j));
        u64tobase62cv(u64, base62cv);
        strlcat(uuidbase62cv, base62cv, 23);
    }
}

int main() {
    char base62cv[12];

    uint64_t a = UINT64_MAX; // 18446744073709551615
    u64tobase62cv(a, base62cv);
    printf("Input = %llu, base62cv = %s\n", a, base62cv);

    uuid_t uuidu8v;
    uuid_generate(uuidu8v);
    uuid_string_t uuidcv;
    uuid_unparse(uuidu8v, uuidcv);
    printf("uuidcv: %s\n", uuidcv);

    uint64_t u64;
    char uuidbase62cv[23] = "";
    for (int i = 0; i < 2; i++) {
        u64 = 0;
        for (int j = 0; j < 8; j++) u64 += (uuidu8v[i * 8 + j] << 8 * (7 - j));
        u64tobase62cv(u64, base62cv);
        strlcat(uuidbase62cv, base62cv, 23);
        printf("uuid slice %d: %llu; base62cv: %s\n", i, u64, base62cv);
    }

    printf("uuidbase62cv: %s\n", uuidbase62cv);

    get_uuidbase62cv(uuidbase62cv);
    printf("uuidbase62cv: %s\n", uuidbase62cv);

    return 0;
}
