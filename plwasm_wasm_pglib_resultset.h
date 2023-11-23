#ifndef H_PLWASM6_WASM_PGLIB_RESULTSET
#define H_PLWASM6_WASM_PGLIB_RESULTSET
#include "plwasm_types.h"

wasm_trap_t*
plwasm_wasm_pglib_resultset_fetch(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_resultset_close(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_resultset_get_int32(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_resultset_get_int64(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_resultset_get_text_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);
#endif
