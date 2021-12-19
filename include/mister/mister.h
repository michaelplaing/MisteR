// mister.h

#ifndef MISTER_H
#define MISTER_H

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/* Message types */
#define CMD_CONNECT         0x10U
#define CMD_CONNACK         0x20U
#define CMD_PUBLISH         0x30U
#define CMD_PUBACK          0x40U
#define CMD_PUBREC          0x50U
#define CMD_PUBREL          0x60U
#define CMD_PUBCOMP         0x70U
#define CMD_SUBSCRIBE       0x80U
#define CMD_SUBACK          0x90U
#define CMD_UNSUBSCRIBE     0xA0U
#define CMD_UNSUBACK        0xB0U
#define CMD_PINGREQ         0xC0U
#define CMD_PINGRESP        0xD0U
#define CMD_DISCONNECT      0xE0U
#define CMD_AUTH            0xF0U

/* MisteR Commands understood by the Redis mister module */
#define MR_CONNECT          "mr.connect"
#define MR_CONNACK          "mr.connack"
#define MR_PUBLISH	        "mr.publish"
#define MR_PUBACK           "mr.puback"
#define MR_PUBREC           "mr.pubrec"
#define MR_PUBREL           "mr.pubrel"
#define MR_PUBCOMP          "mr.pubcomp"
#define MR_SUBSCRIBE        "mr.subscribe"
#define MR_SUBACK           "mr.suback"
#define MR_UNSUBSCRIBE      "mr.unsubscribe"
#define MR_UNSUBACK         "mr.unsuback"
#define MR_PINGREQ          "mr.pingreq"
#define MR_PINGRESP         "mr.pingresp"
#define MR_DISCONNECT       "mr.disconnect"
#define MR_AUTH             "mr.auth"

/* Message types */
#define CMD_CONNECT         0x10U
#define CMD_CONNACK         0x20U
#define CMD_PUBLISH         0x30U
#define CMD_PUBACK          0x40U
#define CMD_PUBREC          0x50U
#define CMD_PUBREL          0x60U
#define CMD_PUBCOMP         0x70U
#define CMD_SUBSCRIBE       0x80U
#define CMD_SUBACK          0x90U
#define CMD_UNSUBSCRIBE     0xA0U
#define CMD_UNSUBACK        0xB0U
#define CMD_PINGREQ         0xC0U
#define CMD_PINGRESP        0xD0U
#define CMD_DISCONNECT      0xE0U
#define CMD_AUTH            0xF0U

#endif /* MISTER_H */
