#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "mister/util.h"

// MQTT unicode validation using a reasonably fast and portable naïve method
/*
 * http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - page 94
 *
 * Table 3-7. Well-Formed UTF-8 Byte Sequences
 *
 * +--------------------+------------+-------------+------------+-------------+
 * | Code Points        | First Byte | Second Byte | Third Byte | Fourth Byte |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+0000..U+007F     | 00..7F     |             |            |             |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+0080..U+07FF     | C2..DF     | 80..BF      |            |             |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+0800..U+0FFF     | E0         | A0..BF      | 80..BF     |             |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+1000..U+CFFF     | E1..EC     | 80..BF      | 80..BF     |             |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+D000..U+D7FF     | ED         | 80..9F      | 80..BF     |             |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+E000..U+FFFF     | EE..EF     | 80..BF      | 80..BF     |             |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+10000..U+3FFFF   | F0         | 90..BF      | 80..BF     | 80..BF      |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+40000..U+FFFFF   | F1..F3     | 80..BF      | 80..BF     | 80..BF      |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+100000..U+10FFFF | F4         | 80..8F      | 80..BF     | 80..BF      |
 * +--------------------+------------+-------------+------------+-------------+
 */

/* Return 0 - success,  >0 - index(1-based) of first error char */
/* MQTT: error on "Disallowed Unicode code points" (control chars) and U+0000 */
int utf8val(const uint8_t *u8v, size_t len) {
    int err_pos = 1;
    if (len > 65536) return err_pos; // MQTT: too large

    while (len) { // 0-length is valid
        int bytes;
        const uint8_t byte1 = u8v[0];

        if (byte1 <= 0x7F) { /* 00..7F */
            if (byte1 <= 0x1F || byte1 == 0x7F) return err_pos; // MQTT: U+0000 or control char
            bytes = 1;
        }
        else if ( /* C2..DF, 80..BF */
                len >= 2
                && byte1 >= 0xC2
                && byte1 <= 0xDF
                && (int8_t)u8v[1] <= (int8_t)0xBF
        ) {
            if (byte1 == 0xC2 && u8v[1] <= 0x9F) return err_pos; // MQTT: control char
            bytes = 2;
        }
        else if (len >= 3) {
            const uint8_t byte2 = u8v[1];
            const uint8_t byte3 = u8v[2];
            const bool byte2_ok = byte2 >= 0x80 && byte2 <= 0xBF; /* 80..BF */
            const bool byte3_ok = byte3 >= 0x80 && byte3 <= 0xBF; /* 80..BF */

            if (
                byte2_ok
                && byte3_ok
                && (
                    (byte1 == 0xE0 && byte2 >= 0xA0)    /* E0, A0..BF, 80..BF */
                    || (byte1 >= 0xE1 && byte1 <= 0xEC) /* E1..EC, 80..BF, 80..BF */
                    || (byte1 == 0xED && byte2 <= 0x9F) /* ED, 80..9F, 80..BF */
                    || (byte1 >= 0xEE && byte1 <= 0xEF) /* EE..EF, 80..BF, 80..BF */
                )
            ) {
                bytes = 3;
            }
            else if (len >= 4) {
                const uint8_t byte4 = u8v[3];
                const bool byte4_ok = byte4 >= 0x80 && byte4 <= 0xBF; /* 80..BF */

                if (
                    byte2_ok
                    && byte3_ok
                    && byte4_ok
                    && (
                        (byte1 == 0xF0 && byte2 >= 0x90)    /* F0, 90..BF, 80..BF, 80..BF */
                        || (byte1 >= 0xF1 && byte1 <= 0xF3) /* F1..F3, 80..BF, 80..BF, 80..BF */
                        || (byte1 == 0xF4 && byte2 <= 0x8F) /* F4, 80..8F, 80..BF, 80..BF */
                    )
                ) {
                    bytes = 4;
                }
                else {
                    return err_pos;
                }
            }
            else {
                return err_pos;
            }
        }
        else {
            return err_pos;
        }

        len -= bytes;
        err_pos += bytes;
        u8v += bytes;
    }

    return 0;
}

int mr_bytecount_VBI(uint32_t u32) {
    if (u32 >> (7 * 4)) { // overflow: too big for 4 bytes
        return -1;
    }

    int i = 0;
    do {
        u32 = u32 >> 7;
        i++;
    } while (u32);

    return i;
}

int mr_make_VBI(uint32_t u32, uint8_t *u8v0) {
    if (u32 >> (7 * 4)) { // overflow: too big for 4 bytes
        return -1;
    }

    uint8_t *pu8 = u8v0;
    int i = 0;
    do {
        *pu8 = u32 & 0x7F;
        u32 = u32 >> 7;
        if (u32) *pu8 |= 0x80;
        i++; pu8++;
    } while (u32);

    return i;
}

int mr_get_VBI(uint32_t *pu32, uint8_t *u8v) {
    uint8_t *pu8 = u8v;
    uint32_t u32, result_u32 = 0;
    int i;

    for (i = 0; i < 4; pu8++, i++){
        u32 = *pu8;
        result_u32 += (u32 & 0x7F) << (7 * i);
        if (!(*pu8 & 0x80)) break;
    }

    if (i == 4) { // overflow: byte[3] has a continuation bit
        return -1;
    }
    else {
        *pu32 = result_u32;
        return i + 1;
    }
}

int print_hexdump(const uint8_t *u8v, const size_t ulen) {
    if (!(u8v && ulen)) return -1;
    int ulines = (ulen - 1) / 16 + 1;
    if (ulines > 40) ulines = 40; // 40 lines max - TODO: parameterize
    size_t clen = ulines * 70 + 1; // trailing 0
    char *pc = calloc(clen, 1);
    if (!pc) return -1;
    int rc = get_hexdump(pc, clen, u8v, ulen);

    if (rc) {
        free(pc);
        return rc;
    }
    else {
        printf("%s\n", pc);
        free(pc);
        return 0;
    }
}

int get_hexdump(char *cv0, size_t clen, const uint8_t *u8v, size_t ulen) {
    if (!(cv0 && clen >= 70 && u8v && ulen)) return -1;
    int ulines = (ulen - 1) / 16 + 1;
    if (ulines > 40) ulines = 40; // 40 lines max - TODO: parameterize?
    int clines = (clen - 2) / 70 + 1; // trailing \0
    if (ulines > clines) ulines = clines;
    if (ulen > ulines * 16) ulen = ulines * 16;

    char cv[17]; // char vector is our reusable string
    cv[16] = '\0'; // and needs termination
    char *pc = cv0; // pointer to next position in output buffer cv0 - increment as we go

    for (int i = 0; i < ulen; i++) {
        sprintf(pc, "%02hhX ", (uint8_t)u8v[i]); // hex
        pc += 3;
        cv[i % 16] = isprint((int)u8v[i]) ? u8v[i] : '.'; // printable char or '.'

        if ((i + 1) % 8 == 0 || i + 1 == ulen) { // finished 8 hex chars - or may be done
            sprintf(pc, " ");
            pc++;

            if ((i + 1) % 16 == 0) { // finished 16 hex chars so print the string - might be done
                sprintf(pc, "|  %s \n", cv);
                pc += 21;
            }
            else if (i + 1 == ulen) { // done - pad the hex and finish up the leftovers
                if ((i + 1) % 16 <= 8) {
                    sprintf(pc, " ");
                    pc++;
                }

                for (int j = (i + 1) % 16; j < 16; j++) {
                    sprintf(pc, "   ");
                    pc += 3;
                }

                cv[(i + 1) % 16] = '\0'; // short string left over
                sprintf(pc, "|  %s \n", cv);
                pc += 3 + (i + 1) % 16 + 2;
            }
            else { // not done
                ; // noop - loop again
            }
        }
    }

    *(pc - 1) = '\0'; // replace trailing '\n'
    return 0;
}

void compress_spaces(char *dst) {
    char *d = dst;
    char *s = dst;
    bool bs = true;

    for ( ; *s; s++) {
        if (*s == ' ' && bs) continue; // only increment source s
        bs = *s == ' '; // is source s a space or not (for next round)
        *d++ = *s; // set dest d to source s then increment both
    }

    if (*dst && *(d - 1) == ' ') { // trailing space to overwrite?
        *(d - 1) = '\0';
    }
    else {
        *d = '\0';
    }
}
