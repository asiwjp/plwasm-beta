#include "plwasm_wasm_pglib_resultset.h"
#include "plwasm_spi.h"
#include "plwasm_wasm_memory.h"
#include "plwasm_wasm_utils.h"
#include "plwasm_utils_pg.h"
#include "plwasm_utils_str.h"
#include "plwasm_log.h"
#include <mb/pg_wchar.h>

#define WASM_MODULE_NAME "pg"
#define WASM_MODULE_NAME_LEN 2

static wasm_trap_t*
plwasm_wasm_pglib_resultset_fetch(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.resultset_fetch";

  plwasm_call_context_t	*cctx;
  plwasm_pg_statement_context_t	*stmctx;
  int		arg1_stmt_id;
  bool		result;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_stmt_id = args[0].of.i32;

  stmctx = plwasm_spi_statement_get_context(cctx, arg1_stmt_id);
  result = plwasm_spi_resultset_fetch(cctx, stmctx);
  
  results[0].of.i32 = result ? 1 : 0;

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

static wasm_trap_t*
plwasm_wasm_pglib_resultset_close(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.resultset_close";

  plwasm_call_context_t	*cctx;
  plwasm_pg_statement_context_t	*stmctx;
  int		arg1_stmt_id;
  bool		result;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_stmt_id = args[0].of.i32;

  stmctx = plwasm_spi_statement_get_context(cctx, arg1_stmt_id);
  result = plwasm_spi_resultset_close(cctx, stmctx);

  results[0].of.i32 = result ? 1 : 0;

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

static wasm_trap_t*
plwasm_wasm_pglib_resultset_get_int32(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.resultset_get_int32";

  plwasm_call_context_t	*cctx;
  plwasm_pg_statement_context_t	*stmctx;
  int		arg1_stmt_id;
  int		arg2_fld_idx;
  Datum		val;
  bool		is_null;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_stmt_id  = args[0].of.i32;
  arg2_fld_idx = args[1].of.i32;

  stmctx = plwasm_spi_statement_get_context(cctx, arg1_stmt_id);
  val = plwasm_spi_resultset_get_val_as(cctx, stmctx, arg2_fld_idx, INT4OID, &is_null);
  if (is_null) {
    CALL_ERROR(
      cctx,
      "%s column value is null. statement_index=%d, field_index=%d",
      FUNC_NAME,
      arg1_stmt_id,
      arg2_fld_idx);
  }

  results[0].of.i32 = DatumGetInt32(val);

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

static wasm_trap_t*
plwasm_wasm_pglib_resultset_get_int64(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.resultset_get_int64";

  plwasm_call_context_t	*cctx;
  plwasm_pg_statement_context_t	*stmctx;
  int		arg1_stmt_id;
  int		arg2_fld_idx;
  Datum		val;
  bool		is_null;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_stmt_id  = args[0].of.i32;
  arg2_fld_idx = args[1].of.i32;

  stmctx = plwasm_spi_statement_get_context(cctx, arg1_stmt_id);
  val = plwasm_spi_resultset_get_val_as(cctx, stmctx, arg2_fld_idx, INT8OID, &is_null);
  if (is_null) {
    CALL_ERROR(
      cctx,
      "%s column value is null. statement_index=%d, field_index=%d",
      FUNC_NAME,
      arg1_stmt_id,
      arg2_fld_idx);
  }

  results[0].kind = WASMTIME_I64;
  results[0].of.i64 = DatumGetInt64(val);

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

static wasm_trap_t*
plwasm_wasm_pglib_resultset_get_float32(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.resultset_get_float32";

  plwasm_call_context_t	*cctx;
  plwasm_pg_statement_context_t	*stmctx;
  int		arg1_stmt_id;
  int		arg2_fld_idx;
  Datum		val;
  bool		is_null;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_stmt_id  = args[0].of.i32;
  arg2_fld_idx = args[1].of.i32;

  stmctx = plwasm_spi_statement_get_context(cctx, arg1_stmt_id);
  val = plwasm_spi_resultset_get_val_as(cctx, stmctx, arg2_fld_idx, FLOAT4OID, &is_null);
  if (is_null) {
    CALL_ERROR(
      cctx,
      "%s column value is null. statement_index=%d, field_index=%d",
      FUNC_NAME,
      arg1_stmt_id,
      arg2_fld_idx);
  }

  results[0].kind = WASMTIME_F32;
  results[0].of.f32 = DatumGetFloat4(val);

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

static wasm_trap_t*
plwasm_wasm_pglib_resultset_get_float64(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.resultset_get_float64";

  plwasm_call_context_t	*cctx;
  plwasm_pg_statement_context_t	*stmctx;
  int		arg1_stmt_id;
  int		arg2_fld_idx;
  Datum		val;
  bool		is_null;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_stmt_id  = args[0].of.i32;
  arg2_fld_idx = args[1].of.i32;

  stmctx = plwasm_spi_statement_get_context(cctx, arg1_stmt_id);
  val = plwasm_spi_resultset_get_val_as(cctx, stmctx, arg2_fld_idx, FLOAT8OID, &is_null);
  if (is_null) {
    CALL_ERROR(
      cctx,
      "%s column value is null. statement_index=%d, field_index=%d",
      FUNC_NAME,
      arg1_stmt_id,
      arg2_fld_idx);
  }

  results[0].kind = WASMTIME_F64;
  results[0].of.f64 = DatumGetFloat8(val);

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

static wasm_trap_t*
plwasm_wasm_pglib_resultset_get_text_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.resultset_get_text_unsafe";

  plwasm_call_context_t	*cctx;
  plwasm_pg_statement_context_t	*stmctx;
  int		arg1_mem_idx;
  int		arg2_mem_sz;
  int		arg3_stmt_id;
  int		arg4_fld_idx;
  Datum		val;
  bool		is_null;
  char		*mem_ptr;
  char		*val_cstr;
  int		val_cstr_len;
  char		*val_cstr_encoded;
  size_t	val_cstr_encoded_sz;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_mem_idx = args[0].of.i32;
  arg2_mem_sz = args[1].of.i32;
  arg3_stmt_id  = args[2].of.i32;
  arg4_fld_idx = args[3].of.i32;

  stmctx = plwasm_spi_statement_get_context(cctx, arg3_stmt_id);
  val = plwasm_spi_resultset_get_val_as(cctx, stmctx, arg4_fld_idx, TEXTOID, &is_null);
  if (is_null) {
    CALL_ERROR(
      cctx,
      "%s column value is null. statement_index=%d, field_index=%d",
      FUNC_NAME,
      arg3_stmt_id,
      arg4_fld_idx);
  }

  val_cstr = text_to_cstring(DatumGetTextP(val));
  val_cstr_len = strlen(val_cstr);
  val_cstr_encoded = plwasm_utils_str_enc(
    cctx,
    val_cstr,
    val_cstr_len, 
    GetDatabaseEncoding(),
    cctx->func_config.string_enc,
    false,
    &val_cstr_encoded_sz);
  if (arg2_mem_sz < val_cstr_len) {
    CALL_ERROR(cctx,
      "%s failed. buffer too small. buffer size=%d, required=%d",
      FUNC_NAME, arg2_mem_sz, val_cstr_len);
  }

  mem_ptr = plwasm_wasm_mem_offset(cctx, arg1_mem_idx, arg2_mem_sz, true, NULL);
  memcpy(mem_ptr, val_cstr_encoded, val_cstr_encoded_sz);
  pfree(val_cstr);

  results[0].of.i32 = val_cstr_encoded_sz;

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

void
plwasm_wasm_pglib_resultset_load(
    plwasm_extension_context_t *ectx
) {
  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "resultset_fetch",
    plwasm_wasm_pglib_resultset_fetch,
    wasm_valtype_new_i32(),
    1,
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "resultset_close",
    plwasm_wasm_pglib_resultset_close,
    wasm_valtype_new_i32(),
    1,
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "resultset_get_int32",
    plwasm_wasm_pglib_resultset_get_int32,
    wasm_valtype_new_i32(),
    2,
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "resultset_get_int64",
    plwasm_wasm_pglib_resultset_get_int64,
    wasm_valtype_new_i64(),
    2,
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "resultset_get_float32",
    plwasm_wasm_pglib_resultset_get_float32,
    wasm_valtype_new_f32(),
    2,
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "resultset_get_float64",
    plwasm_wasm_pglib_resultset_get_float64,
    wasm_valtype_new_f64(),
    2,
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "resultset_get_text_unsafe",
    plwasm_wasm_pglib_resultset_get_text_unsafe,
    wasm_valtype_new_i32(),
    4,
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32());
}
