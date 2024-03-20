#include "plwasm_wasm_pglib_returns.h"
#include "plwasm_wasm_memory.h"
#include "plwasm_wasm_utils.h"
#include "plwasm_utils_pg.h"
#include "plwasm_utils_str.h"
#include "plwasm_log.h"
#include <postgres.h>
#include <mb/pg_wchar.h>

wasm_trap_t* plwasm_wasm_pglib_returns_set_null(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.returns_set_null";

  plwasm_call_context_t *cctx;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);

  cctx->ret.type = VOIDOID;
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t* plwasm_wasm_pglib_returns_set_int32(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.returns_set_int32";

  plwasm_call_context_t *cctx;
  int32_t arg1_retval;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_retval = args[0].of.i32;

  cctx->ret.type = INT4OID;
  cctx->ret.of.i32 = arg1_retval;
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t* plwasm_wasm_pglib_returns_set_int64(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.returns_set_int64";

  plwasm_call_context_t *cctx;
  int64_t arg1_retval;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_retval = args[0].of.i64;

  cctx->ret.type = INT8OID;
  cctx->ret.of.i64 = arg1_retval;
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t* plwasm_wasm_pglib_returns_set_float32(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.returns_set_float32";

  plwasm_call_context_t *cctx;
  float arg1_retval;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_retval = args[0].of.f32;

  cctx->ret.type = FLOAT4OID;
  cctx->ret.of.f32 = arg1_retval;
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t* plwasm_wasm_pglib_returns_set_float64(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.returns_set_float64";

  plwasm_call_context_t *cctx;
  double arg1_retval;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_retval = args[0].of.f64;

  cctx->ret.type = FLOAT8OID;
  cctx->ret.of.f64 = arg1_retval;
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t* plwasm_wasm_pglib_returns_set_text_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char* FUNC_NAME = "pg.returns_set_text_unsafe";

  plwasm_call_context_t *cctx;
  size_t arg1_mem_idx;
  size_t arg2_str_sz;
  char   *arg_str;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_mem_idx = args[0].of.i32;
  arg2_str_sz = args[1].of.i32;

  arg_str = plwasm_wasm_mem_get_string(
    cctx,
    arg1_mem_idx,
    arg2_str_sz);

  cctx->ret.type = TEXTOID;
  cctx->ret.of.text = cstring_to_text(arg_str);
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t* plwasm_wasm_pglib_returns_set_bytea_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.returns_set_bytea_unsafe";

  plwasm_call_context_t *cctx;
  int    arg1_mem_idx;
  int    arg2_mem_offset;
  int    arg3_n;
  char   *mem_ptr;
  bytea  *ret_bytea;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_mem_idx = args[0].of.i32;
  arg2_mem_offset = args[1].of.i32;
  arg3_n = args[2].of.i32;

  mem_ptr = plwasm_wasm_mem_offset(cctx, arg1_mem_idx + arg2_mem_offset, arg3_n, true, NULL);
  ret_bytea = palloc((Size)arg3_n + VARHDRSZ);
  memcpy(VARDATA(ret_bytea), mem_ptr, arg3_n);

  cctx->ret.type = BYTEAOID;
  cctx->ret.of.bytea = ret_bytea;
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

