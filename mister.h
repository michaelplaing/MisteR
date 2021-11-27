
#include <stdint.h>

#ifndef MISTER_H
#define MISTER_H

/* MisteR Commands understood by the Redis module mqtt.so */
#define CONNECT_MRCMD       "mqtt.connect"
#define CONNACK_MRCMD       "mqtt.connack"
#define PUBLISH_MRCMD       "mqtt.publish"
#define PUBACK_MRCMD        "mqtt.puback"
#define PUBREC_MRCMD        "mqtt.pubrec"
#define PUBREL_MRCMD        "mqtt.pubrel"
#define PUBCOMP_MRCMD       "mqtt.pubcomp"
#define SUBSCRIBE_MRCMD     "mqtt.subscribe"
#define SUBACK_MRCMD        "mqtt.suback"
#define UNSUBSCRIBE_MRCMD   "mqtt.unsubscribe"
#define UNSUBACK_MRCMD      "mqtt.unsuback"
#define PINGREQ_MRCMD       "mqtt.pingreq"
#define PINGRESP_MRCMD      "mqtt.pingresp"
#define DISCONNECT_MRCMD    "mqtt.disconnect"
#define AUTH_MRCMD          "mqtt.auth"

struct mrpacket{
	uint8_t *payload;
	uint32_t len;
	uint32_t pos;
};

/* Error values */
enum mrerr_t {
	MRERR_AUTH_CONTINUE = -4,
	MRERR_NO_SUBSCRIBERS = -3,
	MRERR_SUB_EXISTS = -2,
	MRERR_CONN_PENDING = -1,
	MRERR_SUCCESS = 0,
	MRERR_NOMEM = 1,
	MRERR_PROTOCOL = 2,
	MRERR_INVAL = 3,
	MRERR_NO_CONN = 4,
	MRERR_CONN_REFUSED = 5,
	MRERR_NOT_FOUND = 6,
	MRERR_CONN_LOST = 7,
	MRERR_TLS = 8,
	MRERR_PAYLOAD_SIZE = 9,
	MRERR_NOT_SUPPORTED = 10,
	MRERR_AUTH = 11,
	MRERR_ACL_DENIED = 12,
	MRERR_UNKNOWN = 13,
	MRERR_ERRNO = 14,
	MRERR_EAI = 15,
	MRERR_PROXY = 16,
	MRERR_PLUGIN_DEFER = 17,
	MRERR_MALFORMED_UTF8 = 18,
	MRERR_KEEPALIVE = 19,
	MRERR_LOOKUP = 20,
	MRERR_MALFORMED_PACKET = 21,
	MRERR_DUPLICATE_PROPERTY = 22,
	MRERR_TLS_HANDSHAKE = 23,
	MRERR_QOS_NOT_SUPPORTED = 24,
	MRERR_OVERSIZE_PACKET = 25,
	MRERR_OCSP = 26,
	MRERR_TIMEOUT = 27,
	MRERR_RETAIN_NOT_SUPPORTED = 28,
	MRERR_TOPIC_ALIAS_INVALID = 29,
	MRERR_ADMINISTRATIVE_ACTION = 30,
	MRERR_ALREADY_EXISTS = 31,
};

#endif
