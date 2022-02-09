#ifndef PACKET_INTERNAL_H
#define PACKET_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

// spec
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

// from mosquitto & spec
enum mqtt_property {
    MQTT_PROP_PAYLOAD_FORMAT_INDICATOR = 1,             /* Byte :               PUBLISH, Will Properties */
    MQTT_PROP_MESSAGE_EXPIRY_INTERVAL = 2,              /* 4 byte int :         PUBLISH, Will Properties */
    MQTT_PROP_CONTENT_TYPE = 3,                         /* UTF-8 string :       PUBLISH, Will Properties */
    MQTT_PROP_RESPONSE_TOPIC = 8,                       /* UTF-8 string :       PUBLISH, Will Properties */
    MQTT_PROP_CORRELATION_DATA = 9,                     /* Binary Data :        PUBLISH, Will Properties */
    MQTT_PROP_SUBSCRIPTION_IDENTIFIER = 11,             /* Variable byte int :  PUBLISH, SUBSCRIBE */
    MQTT_PROP_SESSION_EXPIRY_INTERVAL = 17,             /* 4 byte int :         CONNECT, CONNACK, DISCONNECT */
    MQTT_PROP_ASSIGNED_CLIENT_IDENTIFIER = 18,          /* UTF-8 string :       CONNACK */
    MQTT_PROP_SERVER_KEEP_ALIVE = 19,                   /* 2 byte int :         CONNACK */
    MQTT_PROP_AUTHENTICATION_METHOD = 21,               /* UTF-8 string :       CONNECT, CONNACK, AUTH */
    MQTT_PROP_AUTHENTICATION_DATA = 22,                 /* Binary Data :        CONNECT, CONNACK, AUTH */
    MQTT_PROP_REQUEST_PROBLEM_INFORMATION = 23,         /* Byte :               CONNECT */
    MQTT_PROP_WILL_DELAY_INTERVAL = 24,                 /* 4 byte int :         Will properties */
    MQTT_PROP_REQUEST_RESPONSE_INFORMATION = 25,        /* Byte :               CONNECT */
    MQTT_PROP_RESPONSE_INFORMATION = 26,                /* UTF-8 string :       CONNACK */
    MQTT_PROP_SERVER_REFERENCE = 28,                    /* UTF-8 string :       CONNACK, DISCONNECT */
    MQTT_PROP_REASON_STRING = 31,                       /* UTF-8 string :       All except Will properties */
    MQTT_PROP_RECEIVE_MAXIMUM = 33,                     /* 2 byte int :         CONNECT, CONNACK */
    MQTT_PROP_TOPIC_ALIAS_MAXIMUM = 34,                 /* 2 byte int :         CONNECT, CONNACK */
    MQTT_PROP_TOPIC_ALIAS = 35,                         /* 2 byte int :         PUBLISH */
    MQTT_PROP_MAXIMUM_QOS = 36,                         /* Byte :               CONNACK */
    MQTT_PROP_RETAIN_AVAILABLE = 37,                    /* Byte :               CONNACK */
    MQTT_PROP_USER_PROPERTY = 38,                       /* UTF-8 string pair :  All */
    MQTT_PROP_MAXIMUM_PACKET_SIZE = 39,                 /* 4 byte int :         CONNECT, CONNACK */
    MQTT_PROP_WILDCARD_SUBSCRIPTION_AVAILABLE = 40,     /* Byte :               CONNACK */
    MQTT_PROP_SUBSCRIPTION_IDENTIFIERS_AVAILABLE = 41,  /* Byte :               CONNACK */
    MQTT_PROP_SHARED_SUBSCRIPTION_AVAILABLE = 42,       /* Byte :               CONNACK */
};

/// a type that can be cast to/from a pointer or an int up to uint32_t
typedef unsigned long mr_mvalue_t;

typedef struct mr_mdata {
    const char *name;       ///< field name from spec
    const int dtype;        ///< data type
    const uint8_t bitpos;   ///< bit position
    mr_mvalue_t value;      ///< for any mdata scalar value or pointer
    bool valloc;            ///< value allocated?
    size_t vlen;            ///< integer byte size OR sub-byte # of bits OR vector byte length
    size_t u8vlen;          ///< byte count of packed value as a uint8_t vector
    bool vexists;           ///< value set?
    const int link;         ///< end of range for VBI; byte to stuff for sub-byte scalar
    const int propid;       ///< property id if any
    const int flagid;       ///< controlling flag id if any
    const int idx;          ///< offset of the mdata in the packet's mdata0 vector
    char *printable;        ///< c-string printable value
} mr_mdata;

typedef int (*mr_mdata_fn)(struct mr_packet_ctx *pctx, struct mr_mdata *mdata);

typedef struct mr_packet_ctx {
    uint8_t mqtt_packet_type;
    const char *mqtt_packet_name;
    char *printable_mdata;
    uint8_t *u8v0;
    bool u8valloc;
    size_t u8vlen;
    size_t u8vpos;
    struct mr_mdata *mdata0;
    size_t mdata_count;
} mr_packet_ctx;

typedef struct mr_dtype {
    const int idx;
    const char *name;
    const mr_mdata_fn count_fn;
    const mr_mdata_fn pack_fn;
    const mr_mdata_fn unpack_fn;
    const mr_mdata_fn output_fn;
    const mr_mdata_fn validate_fn;
    const mr_mdata_fn free_fn;
} mr_dtype;

enum mr_data_types {
    MR_U8_DTYPE,
    MR_U16_DTYPE,
    MR_U32_DTYPE,
    MR_VBI_DTYPE,
    MR_BITS_DTYPE,
    MR_U8V_DTYPE,
    MR_STR_DTYPE,
    MR_SPV_DTYPE,
    MR_FLAGS_DTYPE,
    MR_PROPERTIES_DTYPE
};

typedef int (*mr_ptype_fn)(struct mr_packet_ctx *pctx);

typedef struct mr_ptype {
    const int mqtt_packet_type;
    const char *mqtt_packet_name;
    const mr_ptype_fn ptype_fn;
} mr_ptype;

int mr_init_packet(
    mr_packet_ctx **ppctx, const mr_mdata *MDATA_TEMPLATE, const size_t mdata_count
);

static int mr_unpack_packet(mr_packet_ctx *pctx);

int mr_init_unpack_packet(
    mr_packet_ctx **ppctx,
    const mr_mdata *MDATA_TEMPLATE,
    const size_t mdata_count,
    const uint8_t *u8v0,
    const size_t ulen
);

int mr_pack_packet(mr_packet_ctx *pctx, uint8_t **pu8v0, size_t *pu8vlen);
int mr_free_packet_context(mr_packet_ctx *pctx);

static int mr_get_scalar(mr_packet_ctx *pctx, const int idx, mr_mvalue_t *pvalue, bool *pexists);
int mr_set_scalar(mr_packet_ctx *pctx, const int idx, const mr_mvalue_t value);
int mr_reset_scalar(mr_packet_ctx *pctx, const int idx);

int mr_get_boolean(mr_packet_ctx *pctx, const int idx, bool *pflag_value, bool *pexists);

int mr_get_u8(mr_packet_ctx *pctx, const int idx, uint8_t *pu8, bool *pexists);
static int mr_pack_u8(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u8(mr_packet_ctx *pctx, mr_mdata *mdata);

int mr_get_u16(mr_packet_ctx *pctx, const int idx, uint16_t *pu16, bool *pexists);
static int mr_pack_u16(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u16(mr_packet_ctx *pctx, mr_mdata *mdata);

int mr_get_u32(mr_packet_ctx *pctx, const int idx, uint32_t *pu32, bool *pexists);
static int mr_pack_u32(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u32(mr_packet_ctx *pctx, mr_mdata *mdata);

static int mr_count_VBI(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_VBI(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_VBI(mr_packet_ctx *pctx, mr_mdata *mdata);

static int mr_pack_bits_in_value(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_bits(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_incr1(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_bits(mr_packet_ctx *pctx, mr_mdata *mdata);

static int mr_get_vector(mr_packet_ctx *pctx, const int idx, mr_mvalue_t *ppvoid, size_t *plen, bool *pexists);
int mr_set_vector(mr_packet_ctx *pctx, const int idx, const void *pvoid, const size_t len);
int mr_reset_vector(mr_packet_ctx *pctx, const int idx);
static int mr_free_vector(mr_packet_ctx *pctx, mr_mdata *mdata);

int mr_get_u8v(mr_packet_ctx *pctx, const int idx, uint8_t **pu8v0, size_t *plen, bool *pexists);
static int mr_count_u8v(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_u8v(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_u8v(mr_packet_ctx *pctx, mr_mdata *mdata);
int mr_validate_u8vutf8(mr_packet_ctx *pctx, const int idx);

int mr_get_str(mr_packet_ctx *pctx, const int idx, char **pcv0, bool *pexists);
static int mr_count_str(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_validate_str(mr_packet_ctx *pctx, mr_mdata *mdata);

int mr_get_spv(mr_packet_ctx *pctx, const int idx, mr_string_pair **pspv0, size_t *plen, bool *pexists);
static int mr_count_spv(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_pack_spv(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_unpack_spv(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_validate_spv(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_free_spv(mr_packet_ctx *pctx, mr_mdata *mdata);

static int mr_unpack_properties(mr_packet_ctx *pctx, mr_mdata *mdata);

int mr_validate_utf8_values(mr_packet_ctx *pctx);

static int mr_printable_scalar(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_printable_hexvalue(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_printable_hexdump(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_printable_string(mr_packet_ctx *pctx, mr_mdata *mdata);
static int mr_printable_spv(mr_packet_ctx *pctx, mr_mdata *mdata);
int mr_get_printable(mr_packet_ctx *pctx, const bool all_flag, char **pcv);

// CONNECT

static int mr_check_connect_packet(mr_packet_ctx *pctx);
static int mr_validate_connect_cross(mr_packet_ctx *pctx);
int mr_validate_connect_extra(mr_packet_ctx *pctx);

// memory

int mr_calloc(void **ppv, size_t count, size_t sz);
int mr_malloc(void **ppv, size_t sz);
int mr_realloc(void **ppv, size_t sz);
int mr_free(void *pv);

// util

int utf8val(const uint8_t *u8v, size_t len);
int mr_bytecount_VBI(uint32_t u32);
int mr_make_VBI(uint32_t u32, uint8_t *u8v0);
int mr_get_VBI(uint32_t *pu32, uint8_t *u8v);

#ifdef __cplusplus
}
#endif

#endif // PACKET_INTERNAL_H
