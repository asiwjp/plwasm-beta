#ifndef H_PLWASM6_WASM_PGLIB_CORE
#define H_PLWASM6_WASM_PGLIB_CORE
#include "plwasm_types.h"

wasm_trap_t*
plwasm_wasm_pglib_log_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);
#endif
