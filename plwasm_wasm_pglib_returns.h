#ifndef H_PLWASM6_WASM_PGLIB_RETURNS
#define H_PLWASM6_WASM_PGLIB_RETURNS
#include "plwasm_types.h"

wasm_trap_t*
plwasm_wasm_pglib_returns_set_null(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_returns_set_int32(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_returns_set_int64(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_returns_set_float32(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_returns_set_float64(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);
wasm_trap_t*
plwasm_wasm_pglib_returns_set_text_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_returns_set_bytea_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

#endif
