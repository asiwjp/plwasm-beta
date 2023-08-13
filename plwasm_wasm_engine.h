#ifndef H_PLWASM_WASM_ENGINE
#define H_PLWASM_WASM_ENGINE
#include "plwasm_types.h"

void plwasm_wasm_engine_new(
  plwasm_extension_context_t *ectx
);

void plwasm_wasm_engine_delete(
  plwasm_extension_context_t *ectx
);

#endif
