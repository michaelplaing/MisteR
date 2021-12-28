#include <stdlib.h>
#include <stdbool.h>

#include "validate.h"

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
/* MQTT: error on "Disallowed Unicode code points" (control chars) and 0 */
int utf8val(const uint8_t *u8v, int len) {
    int err_pos = 1;

    while (len) {
        int bytes;
        const uint8_t byte1 = u8v[0];

        if (byte1 <= 0x7F) { /* 00..7F */
            if (byte1 <= 0x1F || byte1 == 0x7F) return err_pos; // MQTT: 0 or control char
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