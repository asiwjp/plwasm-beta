#ifndef H_PLWASM_WASM
#define H_PLWASM_WASM

#include "plwasm_types.h"
#include "postgres.h"

Datum plwasm_wasm_invoke(
    plwasm_call_context_t *cctx,
    char* proname,
    Oid expect_ret_type);

#endif
