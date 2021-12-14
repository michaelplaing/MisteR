#ifndef CONNECT_H
#define CONNECT_H

#include "mister/pack.h"

pack_ctx *init_connect_pctx(void);
int pack_connect_buffer(pack_ctx *pctx);
int free_connect_pctx(pack_ctx *pctx);

int set_connect_clean_start(pack_ctx *pctx, bool value);
int set_connect_user_properties(pack_ctx *pctx, string_pair *sp0, size_t sp_count);
int set_connect_authentication_data(pack_ctx *pctx, uint8_t *authdata, size_t len);

#endif /* CONNECT_H */
