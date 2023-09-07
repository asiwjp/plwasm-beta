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


wasmtime_instance_t*
plwasm_wasm_module_instantiate_with_cache(
  plwasm_call_context_t *cctx,
  Oid fn_oid,
  wasmtime_module_t *module
);

void
plwasm_wasm_module_instantiate(
  plwasm_call_context_t *cctx,
  wasmtime_instance_t *instance,
  wasmtime_module_t *module
);

void
plwasm_wasm_module_extra_init(
  plwasm_call_context_t *cctx
);

void
plwasm_wasm_module_extra_release(
  plwasm_call_context_t *cctx
);

void
plwasm_wasm_load_wasm(
  plwasm_call_context_t *cctx,
  wasm_byte_vec_t *wasm,
  char *proname
);
#endif
