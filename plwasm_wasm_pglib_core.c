#include "plwasm_wasm_pglib_core.h"
#include "plwasm_wasm_memory.h"
#include "plwasm_wasm_utils.h"
#include "plwasm_utils_pg.h"
#include "plwasm_utils_str.h"
#include "plwasm_log.h"
#include <postgres.h>
#include <mb/pg_wchar.h>

wasm_trap_t* plwasm_wasm_pglib_log_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.log_unsafe";

  plwasm_call_context_t *cctx;
  int    arg1_log_level;
  int    arg2_mem_idx;
  size_t arg3_str_sz;
  char   *arg_str;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_log_level = args[0].of.i32;
  arg2_mem_idx = args[1].of.i32;
  arg3_str_sz = args[2].of.i32;

  arg_str = plwasm_wasm_mem_get_string(
    cctx,
    arg2_mem_idx,
    arg3_str_sz);

  CALL_INFO(cctx, "%s", arg_str);
  pfree(arg_str);

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t* plwasm_wasm_pglib_args_is_null(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.args_is_null";

  plwasm_call_context_t *cctx;
  FunctionCallInfo fcinfo;
  int32_t arg1_pgarg_idx;
  bool is_null;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  fcinfo = cctx->fcinfo;
  arg1_pgarg_idx= args[0].of.i32;

  plwasm_utils_pg_proc_check_arg_index(cctx, fcinfo, arg1_pgarg_idx);

  is_null = PG_ARGISNULL(arg1_pgarg_idx);

  results[0].of.i32 = is_null ? 1 : 0;

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t* plwasm_wasm_pglib_args_get_int32(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.args_get_int32";

  plwasm_call_context_t *cctx;
  FunctionCallInfo fcinfo;
  int32_t arg1_pgarg_idx;
  int32_t pgarg_int;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  fcinfo = cctx->fcinfo;
  arg1_pgarg_idx= args[0].of.i32;

  plwasm_utils_pg_proc_check_arg_index(cctx, fcinfo, arg1_pgarg_idx);

  pgarg_int = PG_GETARG_INT32(arg1_pgarg_idx);

  results[0].of.i32 = pgarg_int;

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t* plwasm_wasm_pglib_args_get_text_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.args_get_text_unsafe";

  plwasm_call_context_t *cctx;
  FunctionCallInfo fcinfo;
  int32_t arg1_mem_idx;
  int32_t arg2_mem_sz;
  int32_t arg3_pgarg_idx;
  int32_t arg4_pgarg_cp_sz;
  char *mem_ptr;
  text *pgarg_txt;
  char *pgarg_cstr;
  int pgarg_cstrlen;
  char *pgarg_cstr_encoded;
  size_t pgarg_cstr_encoded_sz;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  fcinfo = cctx->fcinfo;
  arg1_mem_idx = args[0].of.i32;
  arg2_mem_sz = args[1].of.i32;
  arg3_pgarg_idx= args[2].of.i32;
  arg4_pgarg_cp_sz= args[3].of.i32;

  plwasm_utils_pg_proc_check_arg_index(cctx, fcinfo, arg3_pgarg_idx);

  pgarg_txt = PG_GETARG_TEXT_PP(arg3_pgarg_idx);
  pgarg_cstr = text_to_cstring(pgarg_txt);
  pgarg_cstrlen = strlen(pgarg_cstr);
  pgarg_cstr_encoded = plwasm_utils_str_enc(
    cctx,
    pgarg_cstr,
    pgarg_cstrlen, 
    GetDatabaseEncoding(),
    cctx->func_config.string_enc,
    false,
    &pgarg_cstr_encoded_sz);
  if (arg2_mem_sz < pgarg_cstrlen) {
    CALL_ERROR(cctx,
      "%s failed. buffer too small. buffer size=%d, required=%d",
      FUNC_NAME, arg2_mem_sz, pgarg_cstrlen);
  }

  mem_ptr = plwasm_wasm_mem_offset(cctx, arg1_mem_idx, arg4_pgarg_cp_sz, true, NULL);
  memcpy(mem_ptr, pgarg_cstr_encoded, pgarg_cstr_encoded_sz);
  pfree(pgarg_cstr);

  results[0].of.i32 = pgarg_cstr_encoded_sz;

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t* plwasm_wasm_pglib_args_get_bytea_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.args_get_bytea_unsafe";

  plwasm_call_context_t *cctx;
  FunctionCallInfo fcinfo;
  int32_t arg1_mem_idx;
  int32_t arg2_mem_sz;
  int32_t arg3_mem_offset;
  int32_t arg4_pgarg_idx;
  int32_t arg5_bytea_offset;
  int32_t arg6_n;
  char *mem_ptr;
  int32_t tmp_bytea_sz;
  bytea *tmp_bytea;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  fcinfo = cctx->fcinfo;
  arg1_mem_idx = args[0].of.i32;
  arg2_mem_sz = args[1].of.i32;
  arg3_mem_offset = args[2].of.i32;
  arg4_pgarg_idx= args[3].of.i32;
  arg5_bytea_offset = args[4].of.i32;
  arg6_n = args[5].of.i32;

  plwasm_utils_pg_proc_check_arg_index(cctx, fcinfo, arg4_pgarg_idx);

  tmp_bytea = PG_GETARG_BYTEA_PP(arg4_pgarg_idx);
  tmp_bytea_sz = VARSIZE(tmp_bytea);
  if (arg6_n == -1) {
    arg6_n = tmp_bytea_sz;
  }
  mem_ptr = plwasm_wasm_mem_offset(cctx, arg1_mem_idx + arg3_mem_offset, arg6_n, true, NULL);
  tmp_bytea = PG_GETARG_BYTEA_P_SLICE(arg4_pgarg_idx, arg5_bytea_offset, arg6_n);

  memcpy(mem_ptr, VARDATA(tmp_bytea), arg6_n);

  results[0].of.i32 = arg6_n;

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

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

