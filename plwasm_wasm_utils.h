#ifndef H_PLWASM_WASM_UTILS
#define H_PLWASM_WASM_UTILS
#include "plwasm_types.h"

int
plwasm_wasm_vals_to_str(
  char *buf,
  int buf_sz,
  const wasmtime_val_t *vals,
  int n,
  const char *sep,
  const char *default_string
);

int
plwasm_wasm_val_to_str(
  char *buf,
  int buf_sz,
  const wasmtime_val_t *val
);

plwasm_call_context_t*
plwasm_wasm_func_begin(
  wasmtime_caller_t *caller,
  const char* func_name,
  const wasmtime_val_t *args,
  int nargs
);

void
plwasm_wasm_func_end(
  plwasm_call_context_t *cctx,
  const char* func_name,
  const wasmtime_val_t *results,
  int nresults
);

void
plwasm_wasm_define_func_0(
  plwasm_extension_context_t *ectx,
  char *module_name,
  int  module_name_len,
  char *func_name,
  void *func,
  int  argsn,
  ...
);

void
plwasm_wasm_define_func_1(
  plwasm_extension_context_t *ectx,
  char *module_name,
  int  module_name_len,
  char *func_name,
  void *func,
  wasm_valtype_t *result_type,
  int  argsn,
  ...
);

void
plwasm_wasm_define_func(
  plwasm_extension_context_t *ectx,
  char *module_name,
  int  module_name_len,
  char *func_name,
  void *func,
  wasm_functype_t *func_type
);
#endif
