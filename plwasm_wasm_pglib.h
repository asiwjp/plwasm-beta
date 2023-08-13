#ifndef H_PLWASM_WASM_PGLIB
#define H_PLWASM_WASM_PGLIB
#include "plwasm_types.h"

void plwasm_wasm_pglib_init(
    plwasm_extension_context_t *ectx
);

void plwasm_wasm_pglib_bind(
    plwasm_call_context_t *cctx
);

#endif
