#ifndef H_PLWASM_WASM_LOADER
#define H_PLWASM_WASM_LOADER

#include "plwasm_types.h"

wasmtime_module_t*
plwasm_wasm_module_load_with_cache(
  plwasm_call_context_t *cctx,
  Oid fn_oid
  //char *proname,
  //char *source
);

wasmtime_module_t*
plwasm_wasm_module_create(
  plwasm_call_context_t *cctx,
  wasm_byte_vec_t *wasm
);

void
plwasm_wasm_module_instantiate(
  plwasm_call_context_t *cctx,
  wasmtime_module_t *module
);

void
plwasm_wasm_load_wasm(
  plwasm_call_context_t *cctx,
  wasm_byte_vec_t *wasm,
  char *proname
);
#endif
