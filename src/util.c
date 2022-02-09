#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <zlog.h>

#include "mister/mister.h"
#include "util_internal.h"

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
    //mr_print_hexdump(u8v, len);
    //dzlog_debug("int utf8val:: *ubv: %02hhX; len: %lu", u8v[0], len);
    int err_pos = 1;
    if (len > 65536) return err_pos; // MQTT: vector too large

    while (len) { // 0-length is valid
        int bytes;
        const uint8_t byte1 = u8v[0];

        if (byte1 <= 0x7F) { /* 00..7F */
            if (byte1 <= 0x1F || byte1 == 0x7F) return err_pos; // MQTT: U+0000 or control char
            bytes = 1;
        }
        else if (
            len >= 2
            && byte1 >= 0xC2 && byte1 <= 0xDF   /* C2..DF */
            && u8v[1] >= 0x80 && u8v[1] <= 0xBF /* 80..BF */
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

int mr_print_hexdump(uint8_t *u8v, const size_t u8vlen) {
    if (!(u8v && u8vlen)) return -1;
    int u8vlines = (u8vlen - 1) / 16 + 1;
    if (u8vlines > 60) u8vlines = 60; // 60 lines max - TODO: parameterize
    size_t cvlen = u8vlines * 70 + 1; // trailing 0
    char *cv0 = calloc(cvlen, 1);
    if (!cv0) return -1;
    int rc = mr_get_hexdump(cv0, cvlen, u8v, u8vlen);

    if (rc) {
        free(cv0);
        return rc;
    }
    else {
        puts(cv0);
        free(cv0);
        return 0;
    }
}

int mr_get_hexdump(char *cv0, size_t cvlen, const uint8_t *u8v, size_t u8vlen) {
    if (!(cv0 && cvlen >= 70 && u8v && u8vlen)) return -1;
    int u8vlines = (u8vlen - 1) / 16 + 1;
    if (u8vlines > 60) u8vlines = 60; // 60 lines max - TODO: parameterize?
    int cvlines = (cvlen - 2) / 70 + 1; // trailing \0
    if (u8vlines > cvlines) u8vlines = cvlines;
    if (u8vlen > u8vlines * 16) u8vlen = u8vlines * 16;

    char cv[16 + 1] = {'\0'}; // char vector is our reusable string
    char *pc = cv0; // pointer to next char in output buffer cv0 - increment as we go

    for (int i = 0; i < u8vlen; i++) {
        sprintf(pc, "%02hhX ", (uint8_t)u8v[i]); // hex
        pc += 3;
        cv[i % 16] = isprint((int)u8v[i]) ? u8v[i] : '.'; // either printable char or '.'

        if ((i + 1) % 8 == 0 || i + 1 == u8vlen) { // finished 8 hex chars - or may be done
            sprintf(pc, " ");
            pc++;

            if ((i + 1) % 16 == 0) { // finished 16 hex chars so print the string - might be done
                sprintf(pc, "|  %s \n", cv);
                pc += 21;
            }
            else if (i + 1 == u8vlen) { // done - pad the hex with spaces and finish up the leftovers
                if ((i + 1) % 16 <= 8) {
                    sprintf(pc, " ");
                    pc++;
                }

                for (int j = (i + 1) % 16; j < 16; j++) {
                    sprintf(pc, "   ");
                    pc += 3;
                }

                cv[(i + 1) % 16] = '\0'; // terminate short string left over
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

void mr_compress_spaces_lines(char *cv) {
    if (!*cv) return;
    size_t slen = strlen(cv);
    char *pc_dest = cv;
    char *pc_src = cv;
    bool bs = true;

    // compress spaces in place
    for ( ; *pc_src; pc_src++) {
        if (*pc_src == ' ' && bs) continue;
        bs = *pc_src == ' ';
        *pc_dest++ = *pc_src;
    }

    *pc_dest = '\0';

    for (pc_dest--; *pc_dest == ' ' || *pc_dest == '\n'; pc_dest--) { // remove trailing whitespace
        *pc_dest = '\0';
    }

    // compress lines into a temporary result
    char resultv[slen + 1];
    for (int i = 0; i <= slen; i++) resultv[i] = '\0';
    pc_dest = resultv;
    pc_src = cv;
    size_t replace_pos = 0, compress_pos = slen - strlen(cv);

    for (int i = 0; *pc_src && replace_pos < compress_pos; i++, pc_src++) {
        if (*pc_src == '\n') { // replace newline with padded '/'
            // add a leading space if there is room and it is needed
            if (replace_pos + 1 < compress_pos && i && *(pc_src - 1) != ' ') {
                *pc_dest++ = ' ';
                replace_pos++;
            }

            *pc_dest++ = '/'; // break the line
            replace_pos++;

            // add a trailing space if there is room and it is needed
            if (replace_pos + 1 < compress_pos && *(pc_src + 1) != ' ') {
                *pc_dest++ = ' ';
                replace_pos++;
            }
        }
        else {
            *pc_dest++ = *pc_src;
        }
    }

    strlcpy(cv, resultv, slen);
}
