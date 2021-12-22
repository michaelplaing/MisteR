#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
//#include <stdbool.h>


int mr_make_VBI(uint32_t val32, uint8_t *uint80) {
    if (val32 >> (7 * 4)) { // overflow: too big for 4 bytes
        return -1;
    }

    int i = 0;
    do {
        *uint80 = val32 & 0x7F;
        val32 = val32 >> 7;
        if (val32) *uint80 = *uint80 | 0x80;
        i++; uint80++;
    } while (val32);

    return i;
}

int mr_get_VBI(uint32_t *Pval32, uint8_t *uint80) {
    uint32_t val32, res32 = 0;
    int i;
    for (i = 0; i < 4; uint80++, i++){
        val32 = *uint80;
        res32 += (val32 & 0x7F) << (7 * i);
        if (!(*uint80 & 0x80)) break;
    }

    if (i == 4) { // overflow: byte[3] has a continuation bit
        return -1;
    }
    else {
        *Pval32 = res32;
        return i + 1;
    }
}

int main(void) {
    uint8_t uint80[5];
    uint32_t val32 = 127;
    int rc = mr_make_VBI(val32, uint80);
    printf("mr_make_VBI: val32: %u; rc: %d\n", val32, rc);

    if (rc > 0) {
        for (int i = 0; i < rc; i++) {
            printf(" %02hhX", uint80[i]);
        }

        printf("\n");

        rc = mr_get_VBI(&val32, uint80);
        printf("mr_get_VBI: rc: %d; val32: %u\n", rc, val32);
    }

    uint8_t buf2[] = {0xFF, 0xFF, 0xFF, 0x7F};
    val32 = 0;
    rc = mr_get_VBI(&val32, buf2);
    printf("mr_get_VBI: rc: %d; val32: %u\n", rc, val32);

}