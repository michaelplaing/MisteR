#ifndef PACK_INTERNAL_H
#define PACK_INTERNAL_H

#include "mister/packet.h"

typedef struct mr_mdata {
    char *name;
    int index;      // integer position in the mdata vector
    int link;       // end of range for VBI; byte to stuff for bit mdata
    int (*pack_fn)(struct packet_ctx *pctx, struct mr_mdata *mdata);
    int (*unpack_fn)(struct packet_ctx *pctx, struct mr_mdata *mdata);
    Word_t value;   // can handle any mr_mdata value including pointers
    size_t bitpos;  // for sub-byte values
    size_t vlen;    // for sub-byte values, pointer values, vectors & VBIs
    bool exists;    // for properties
    uint8_t id;     // for properties
    bool isalloc;   // is buf allocated
    size_t blen;
    uint8_t *buf;   // packed value
} mr_mdata;

int init_packet_context(
    packet_ctx **Ppctx, const mr_mdata *MDATA_TEMPLATE, size_t mdata_count
);

int free_packet_context(packet_ctx *pctx);

int set_scalar_value(packet_ctx *pctx, int index, Word_t value);

int get_scalar_value(packet_ctx *pctx, int index, Word_t *Pvalue);

int set_vector_value(packet_ctx *pctx, int index, Word_t value, size_t len);

int get_vector_value(packet_ctx *pctx, int index, Word_t *Pvalue, size_t *Plen);

int reset_header_value(packet_ctx *pctx, int index);

int pack_mdata_buffer(packet_ctx *pctx);
int unpack_mdata_buffer(packet_ctx *pctx);

int pack_uint8(packet_ctx *pctx, mr_mdata *mdata);
int unpack_uint8(packet_ctx *pctx, mr_mdata *mdata);

int pack_uint16(packet_ctx *pctx, mr_mdata *mdata);

int pack_uint32(packet_ctx *pctx, mr_mdata *mdata);

int pack_sprop_uint8(packet_ctx *pctx, mr_mdata *mdata);

int pack_sprop_uint16(packet_ctx *pctx, mr_mdata *mdata);

int pack_sprop_uint32(packet_ctx *pctx, mr_mdata *mdata);

int pack_sprop_char_buf(packet_ctx *pctx, mr_mdata *mdata);

int pack_mprop_strpair(packet_ctx *pctx, mr_mdata *mdata);

int pack_str(packet_ctx *pctx, mr_mdata *mdata);

int pack_sprop_str(packet_ctx *pctx, mr_mdata *mdata);

int make_VBI(uint32_t val32, uint8_t *buf);

int pack_VBI(packet_ctx *pctx, mr_mdata *mdata);
int unpack_VBI(packet_ctx *pctx, mr_mdata *mdata);

int pack_char_buf(packet_ctx *pctx, mr_mdata *mdata);

int pack_flags_alloc(packet_ctx *pctx, mr_mdata *mdata);

int pack_in_parent(packet_ctx *pctx, mr_mdata *mdata);

#endif // PACK_INTERNAL_H
