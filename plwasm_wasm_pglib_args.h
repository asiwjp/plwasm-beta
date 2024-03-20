#ifndef H_PLWASM6_WASM_PGLIB_ARGS
#define H_PLWASM6_WASM_PGLIB_ARGS
#include "plwasm_types.h"

wasm_trap_t*
plwasm_wasm_pglib_args_is_null(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_args_get_int32(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_args_get_int64(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_args_get_float32(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_args_get_float64(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_args_get_text_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_args_get_bytea_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);
#endif
