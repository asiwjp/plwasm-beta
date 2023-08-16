#include "plwasm_wasm_pglib_core.h"
#include "plwasm_wasm_memory.h"
#include "plwasm_wasm_utils.h"
#include "plwasm_utils_pg.h"
#include "plwasm_utils_str.h"
#include "plwasm_log.h"
#include <postgres.h>
#include <mb/pg_wchar.h>

#include <iconv.h>

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
  int    log_level;
  int    mem_offset;
  size_t arg_str_sz;
  char   *arg_str;
  char   *converted;
  size_t converted_sz;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  log_level = args[0].of.i32;
  mem_offset = args[1].of.i32;
  arg_str_sz = args[2].of.i32;

  arg_str = plwasm_wasm_mem_offset(cctx, mem_offset, arg_str_sz, true, NULL);

  converted = plwasm_utils_str_enc(
    cctx,
    arg_str,
    arg_str_sz,
    cctx->func_config.string_enc,
    GetDatabaseEncoding(),
    true,
    &converted_sz);

  CALL_INFO(cctx, "%s", converted);

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
  int32_t pgarg_idx;
  int32_t pgarg_int;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  fcinfo = cctx->fcinfo;
  pgarg_idx= args[0].of.i32;

  plwasm_utils_pg_proc_check_arg_index(cctx, fcinfo, pgarg_idx);

  pgarg_int = PG_GETARG_INT32(pgarg_idx);

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
  int32_t mem_idx;
  int32_t mem_sz;
  int32_t pgarg_idx;
  int32_t pgarg_cp_sz;
  char *mem_ptr;
  text *pgarg_txt;
  char *pgarg_cstr;
  int pgarg_cstrlen;
  char *pgarg_cstr_encoded;
  size_t pgarg_cstr_encoded_sz;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  fcinfo = cctx->fcinfo;
  mem_idx = args[0].of.i32;
  mem_sz = args[1].of.i32;
  pgarg_idx= args[2].of.i32;
  pgarg_cp_sz= args[3].of.i32;

  plwasm_utils_pg_proc_check_arg_index(cctx, fcinfo, pgarg_idx);

  pgarg_txt = PG_GETARG_TEXT_PP(pgarg_idx);
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
  if (mem_sz < pgarg_cstrlen) {
    CALL_ERROR(cctx,
      "%s failed. buffer too small. buffer size=%d, required=%d",
      FUNC_NAME, mem_sz, pgarg_cstrlen);
  }

  mem_ptr = plwasm_wasm_mem_offset(cctx, mem_idx, pgarg_cp_sz, true, NULL);
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
  int32_t mem_idx;
  int32_t mem_sz;
  int32_t mem_offset;
  int32_t pgarg_idx;
  int32_t bytea_offset;
  int32_t n;
  char *mem_ptr;
  int32_t tmp_bytea_sz;
  bytea *tmp_bytea;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  fcinfo = cctx->fcinfo;
  mem_idx = args[0].of.i32;
  mem_sz = args[1].of.i32;
  mem_offset = args[2].of.i32;
  pgarg_idx= args[3].of.i32;
  bytea_offset = args[4].of.i32;
  n = args[5].of.i32;

  plwasm_utils_pg_proc_check_arg_index(cctx, fcinfo, pgarg_idx);

  tmp_bytea = PG_GETARG_BYTEA_PP(pgarg_idx);
  tmp_bytea_sz = VARSIZE(tmp_bytea);
  if (n == -1) {
    n = tmp_bytea_sz;
  }
  mem_ptr = plwasm_wasm_mem_offset(cctx, mem_idx + mem_offset, n, true, NULL);
  tmp_bytea = PG_GETARG_BYTEA_P_SLICE(pgarg_idx, bytea_offset, n);

  memcpy(mem_ptr, VARDATA(tmp_bytea), n);

  results[0].of.i32 = n;

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
  int32_t arg_retval;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg_retval = args[0].of.i32;

  cctx->ret.type = INT4OID;
  cctx->ret.of.i32 = arg_retval;
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
  size_t  mem_offset;
  char   *arg_str;
  size_t arg_str_sz;
  size_t converted_sz;
  char   *ret_str;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  mem_offset = args[0].of.i32;
  arg_str_sz = args[1].of.i32;

  arg_str = plwasm_wasm_mem_offset(cctx, mem_offset, arg_str_sz, true, NULL);

  ret_str = plwasm_utils_str_enc(
    cctx,
    arg_str,
    arg_str_sz,
    cctx->func_config.string_enc,
    GetDatabaseEncoding(),
    true,
    &converted_sz);

  cctx->ret.type = TEXTOID;
  cctx->ret.of.text = cstring_to_text(ret_str);
  CALL_DEBUG5(cctx,
    "%s Set the return value to \"%s\"",
    FUNC_NAME, VARDATA(cctx->ret.of.text));
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
  int    mem_idx;
  int    mem_offset;
  int    n;
  char   *mem_ptr;
  bytea  *ret_bytea;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  mem_idx = args[0].of.i32;
  mem_offset = args[1].of.i32;
  n = args[2].of.i32;

  mem_ptr = plwasm_wasm_mem_offset(cctx, mem_idx + mem_offset, n, true, NULL);
  ret_bytea = palloc((Size)n + VARHDRSZ);
  memcpy(VARDATA(ret_bytea), mem_ptr, n);

  cctx->ret.type = BYTEAOID;
  cctx->ret.of.bytea = ret_bytea;

  CALL_DEBUG5(cctx, 
    "%s return void. Set the return value of proc to BYTEA(%d)",
    FUNC_NAME,
    VARSIZE(cctx->ret.of.bytea));
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

