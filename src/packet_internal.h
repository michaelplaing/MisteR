#ifndef PACK_INTERNAL_H
#define PACK_INTERNAL_H

#include "mister/packet.h"

typedef struct mr_mdata {
    char *name;
    int index;      // integer position in the mdata vector
    int link;       // end of range for VBI; byte to stuff for bit mdata
    int (*pack_fn)(struct pack_ctx *pctx, struct mr_mdata *mdata);
    Word_t value;   // can handle any mr_mdata value including pointers
    size_t bitpos;  // for sub-byte values
    size_t vlen;    // for sub-byte values, pointer values, vectors & VBIs
    bool exists;    // for properties
    uint8_t id;     // for properties
    bool isalloc;   // is buf allocated
    size_t buflen;
    uint8_t *buf;   // packed value
} mr_mdata;

pack_ctx *init_pack_context(const mr_mdata *MDATA_TEMPLATE, size_t mdata_count);

int free_pack_context(pack_ctx *pctx);

int set_scalar_value(pack_ctx *pctx, int index, Word_t value);

int get_scalar_value(pack_ctx *pctx, int index, Word_t *Pvalue);

int set_vector_value(pack_ctx *pctx, int index, Word_t value, size_t len);

int get_vector_value(pack_ctx *pctx, int index, Word_t *Pvalue, size_t *Plen);
    
int reset_header_value(pack_ctx *pctx, int index);

int pack_mdata_buffer(pack_ctx *pctx);

int pack_uint8(pack_ctx *pctx, mr_mdata *mdata);

int pack_uint16(pack_ctx *pctx, mr_mdata *mdata);

int pack_uint32(pack_ctx *pctx, mr_mdata *mdata);

int pack_sprop_uint8(pack_ctx *pctx, mr_mdata *mdata);

int pack_sprop_uint16(pack_ctx *pctx, mr_mdata *mdata);

int pack_sprop_uint32(pack_ctx *pctx, mr_mdata *mdata);

int pack_sprop_char_buf(pack_ctx *pctx, mr_mdata *mdata);

int pack_mprop_strpair(pack_ctx *pctx, mr_mdata *mdata);

int pack_str(pack_ctx *pctx, mr_mdata *mdata);

int pack_sprop_str(pack_ctx *pctx, mr_mdata *mdata);

int make_VBI(uint32_t val32, uint8_t *buf);

int pack_VBI(pack_ctx *pctx, mr_mdata *mdata);

int pack_char_buf(pack_ctx *pctx, mr_mdata *mdata);

int pack_flags_alloc(pack_ctx *pctx, mr_mdata *mdata);

int pack_in_parent(pack_ctx *pctx, mr_mdata *mdata);

#endif // PACK_INTERNAL_H
