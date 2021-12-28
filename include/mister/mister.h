// mister.h

#ifndef MISTER_H
#define MISTER_H

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

enum mqtt_packet_type {
    MQTT_CONNECT        = 0x10U,
    MQTT_CONNACK        = 0x20U,
    MQTT_PUBLISH        = 0x30U,
    MQTT_PUBACK         = 0x40U,
    MQTT_PUBREC         = 0x50U,
    MQTT_PUBREL         = 0x60U,
    MQTT_PUBCOMP        = 0x70U,
    MQTT_SUBSCRIBE      = 0x80U,
    MQTT_SUBACK         = 0x90U,
    MQTT_UNSUBSCRIBE    = 0xA0U,
    MQTT_UNSUBACK       = 0xB0U,
    MQTT_PINGREQ        = 0xC0U,
    MQTT_PINGRESP       = 0xD0U,
    MQTT_DISCONNECT     = 0xE0U,
    MQTT_AUTH           = 0xF0U
};

/* copied from mosquitto & spec */
enum mqtt_property {
    MQTT_PROP_PAYLOAD_FORMAT_INDICATOR = 1,		/* Byte :				PUBLISH, Will Properties */
    MQTT_PROP_MESSAGE_EXPIRY_INTERVAL = 2,		/* 4 byte int :			PUBLISH, Will Properties */
    MQTT_PROP_CONTENT_TYPE = 3,					/* UTF-8 string :		PUBLISH, Will Properties */
    MQTT_PROP_RESPONSE_TOPIC = 8,				/* UTF-8 string :		PUBLISH, Will Properties */
    MQTT_PROP_CORRELATION_DATA = 9,				/* Binary Data :		PUBLISH, Will Properties */
    MQTT_PROP_SUBSCRIPTION_IDENTIFIER = 11,		/* Variable byte int :	PUBLISH, SUBSCRIBE */
    MQTT_PROP_SESSION_EXPIRY_INTERVAL = 17,		/* 4 byte int :			CONNECT, CONNACK, DISCONNECT */
    MQTT_PROP_ASSIGNED_CLIENT_IDENTIFIER = 18,	/* UTF-8 string :		CONNACK */
    MQTT_PROP_SERVER_KEEP_ALIVE = 19,			/* 2 byte int :			CONNACK */
    MQTT_PROP_AUTHENTICATION_METHOD = 21,		/* UTF-8 string :		CONNECT, CONNACK, AUTH */
    MQTT_PROP_AUTHENTICATION_DATA = 22,			/* Binary Data :		CONNECT, CONNACK, AUTH */
    MQTT_PROP_REQUEST_PROBLEM_INFORMATION = 23,	/* Byte :				CONNECT */
    MQTT_PROP_WILL_DELAY_INTERVAL = 24,			/* 4 byte int :			Will properties */
    MQTT_PROP_REQUEST_RESPONSE_INFORMATION = 25,/* Byte :				CONNECT */
    MQTT_PROP_RESPONSE_INFORMATION = 26,		/* UTF-8 string :		CONNACK */
    MQTT_PROP_SERVER_REFERENCE = 28,			/* UTF-8 string :		CONNACK, DISCONNECT */
    MQTT_PROP_REASON_STRING = 31,				/* UTF-8 string :		All except Will properties */
    MQTT_PROP_RECEIVE_MAXIMUM = 33,				/* 2 byte int :			CONNECT, CONNACK */
    MQTT_PROP_TOPIC_ALIAS_MAXIMUM = 34,			/* 2 byte int :			CONNECT, CONNACK */
    MQTT_PROP_TOPIC_ALIAS = 35,					/* 2 byte int :			PUBLISH */
    MQTT_PROP_MAXIMUM_QOS = 36,					/* Byte :				CONNACK */
    MQTT_PROP_RETAIN_AVAILABLE = 37,			/* Byte :				CONNACK */
    MQTT_PROP_USER_PROPERTY = 38,				/* UTF-8 string pair :	All */
    MQTT_PROP_MAXIMUM_PACKET_SIZE = 39,			/* 4 byte int :			CONNECT, CONNACK */
    MQTT_PROP_WILDCARD_SUB_AVAILABLE = 40,		/* Byte :				CONNACK */
    MQTT_PROP_SUBSCRIPTION_ID_AVAILABLE = 41,	/* Byte :				CONNACK */
    MQTT_PROP_SHARED_SUB_AVAILABLE = 42,		/* Byte :				CONNACK */
};


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


/* from spec: The Reason Codes used for Malformed Packet and Protocol Errors */
enum mqtt_reason_codes {
    MQTT_MALFORMED_PACKET                       = 0x81,
    MQTT_PROTOCOL_ERROR                         = 0x82,
    MQTT_RECEIVE_MAXIMUM_EXCEEDED               = 0x93,
    MQTT_PACKET_TOO_LARGE                       = 0x95,
    MQTT_RETAIN_NOT_SUPPORTED                   = 0x9A,
    MQTT_QOS_NOT_SUPPORTED                      = 0x9B,
    MQTT_SHARED_SUBSCRIPTIONS_NOT_SUPPORTED     = 0x9E,
    MQTT_SUBSCRIPTION_IDENTIFIERS_NOT_SUPPORTED = 0xA1,
    MQTT_WILDCARD_SUBSCRIPTIONS_NOT_SUPPORTED   = 0xA2
};

#endif /* MISTER_H */
