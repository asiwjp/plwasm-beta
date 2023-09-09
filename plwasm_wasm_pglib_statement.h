#ifndef H_PLWASM6_WASM_PGLIB_COMMAND
#define H_PLWASM6_WASM_PGLIB_COMMAND
#include "plwasm_types.h"

wasm_trap_t*
plwasm_wasm_pglib_query_int32_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_query_text_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_statement_new_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_statement_prepare(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_statement_execute(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);

wasm_trap_t*
plwasm_wasm_pglib_statement_close(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
);
#endif
