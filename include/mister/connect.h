#ifndef CONNECT_H
#define CONNECT_H

#include "mister/packet.h"

pack_ctx *init_connect_pctx(void);
int pack_connect_buffer(pack_ctx *pctx);
int free_connect_pctx(pack_ctx *pctx);

int set_connect_clean_start(pack_ctx *pctx, bool value);
int get_connect_clean_start(pack_ctx *pctx, bool *flag);

int set_connect_user_properties(pack_ctx *pctx, string_pair *sp0, size_t len);
int get_connect_user_properties(pack_ctx *pctx, string_pair **Psp0, size_t *Plen);

int set_connect_authentication_data(pack_ctx *pctx, uint8_t *authdata, size_t len);
int get_connect_authentication_data(pack_ctx *pctx, uint8_t **Pauthdata, size_t *Plen);

#endif /* CONNECT_H */
