#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
//#include <string.h>
//#include <stdbool.h>
#include <jemalloc/jemalloc.h>


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

void DumpHex(const void* data, size_t size) {
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i) {
        printf("%02X ", ((unsigned char*)data)[i]);
        if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char*)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i+1) % 8 == 0 || i+1 == size) {
            printf(" ");
            if ((i+1) % 16 == 0) {
                printf("|  %s \n", ascii);
            } else if (i+1 == size) {
                ascii[(i+1) % 16] = '\0';
                if ((i+1) % 16 <= 8) {
                    printf(" ");
                }
                for (j = (i+1) % 16; j < 16; ++j) {
                    printf("   ");
                }
                printf("|  %s \n", ascii);
            }
        }
    }
}

int print_hexdump(const uint8_t *u8v, size_t len) {
    char strv[17];
    strv[16] = '\0';

    for (int i = 0; i < len; i++) {
        printf("%02hhX ", (uint8_t)u8v[i]);

        if (isprint((int)u8v[i])) {
            strv[i % 16] = u8v[i];
        }
        else {
            strv[i % 16] = '.';
        }

        if ((i + 1) % 8 == 0 || i + 1 == len) {
            printf(" ");

            if ((i + 1) % 16 == 0) {
                printf("|  %s \n", strv);
            }
            else if (i + 1 == len) {
                strv[(i + 1) % 16] = '\0';
                if ((i + 1) % 16 <= 8) printf(" ");
                for (int j = (i + 1) % 16; j < 16; j++) printf("   ");
                printf("|  %s \n", strv);
            }
        }
    }

    return 0;
}

void do_something(size_t i) {
    // Leak some memory.
    void *v = malloc(i * 100);
}

void compress_spaces_orig(char *dst) {
    char *d = dst;
    char *s = dst;
    bool bs = true;

    for ( ; *s; s++) {
        if (*s == ' ') {
            if (bs) {
                ;
            }
            else {
                bs = true;
                *d++ = ' ';
            }
        }
        else {
            bs = false;
            *d++ = *s;
        }
    }

    if (*dst && *(d - 1) == ' ') {
        *(d - 1) = '\0';
    }
    else {
        *d = '\0';
    }
}

void compress_spaces_tweak(char *dst) {
    char *d = dst;
    char *s = dst;
    bool bs = true;

    for ( ; *s; s++) {
        if (*s == ' ' && bs) continue;
        bs = *s == ' ';
        *d++ = *s;
    }

    if (*dst && *(d - 1) == ' ') {
        *(d - 1) = '\0';
    }
    else {
        *d = '\0';
    }
}

int main(int argc, char **argv) {
/*
    for (size_t i = 0; i < 1000; i++) {
        do_something(i);
    }

    // Dump allocator statistics to stderr.
    malloc_stats_print(NULL, NULL, NULL);
*/
    char cv[] = "        foo                     bar          ";
    int l1 = strlen(cv);
    compress_spaces_tweak(cv);
    int l2 = strlen(cv);
    printf("l1: %d; l2: %d, cv: '%s'\n", l1, l2, cv);
    char nv[] = "";
    compress_spaces_tweak(nv);
    printf("nv: '%s'\n", nv);
    return 0;
}
