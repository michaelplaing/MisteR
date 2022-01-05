#ifndef CONNACK_INTERNAL_H
#define CONNACK_INTERNAL_H

#include "mister/connack.h"

enum CONNACK_MDATA_FIELDS { // Same order as CONNACK_MDATA_TEMPLATE
    CONNACK_PACKET_TYPE,
    CONNACK_REMAINING_LENGTH,
};

typedef struct mr_connack_values { // may or may not be useful
    const uint8_t packet_type;
    uint32_t remaining_length;
} mr_connack_values;

#endif // CONNACK_INTERNAL_H
