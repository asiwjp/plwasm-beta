#include "plwasm_wasm_pglib_statement.h"
#include "plwasm_spi.h"
#include "plwasm_wasm_memory.h"
#include "plwasm_wasm_utils.h"
#include "plwasm_utils_pg.h"
#include "plwasm_utils_str.h"
#include "plwasm_log.h"

#define WASM_MODULE_NAME "pg"
#define WASM_MODULE_NAME_LEN 2

static wasm_trap_t*
plwasm_wasm_pglib_query_int32_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.query_int32_unsafe";

  plwasm_call_context_t *cctx;
  int		arg1_stmt_mem_offset;
  size_t	arg2_stmt_mem_sz;
  char		*stmt_txt;
  Datum		val;
  bool		is_null;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_stmt_mem_offset = args[0].of.i32;
  arg2_stmt_mem_sz = args[1].of.i32;

  stmt_txt = plwasm_wasm_mem_get_string(
	cctx, arg1_stmt_mem_offset, arg2_stmt_mem_sz);

  val = plwasm_spi_query_scalar_as(cctx, stmt_txt, INT4OID, &is_null);

  results[0].of.i32 = DatumGetInt32(val);

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

static wasm_trap_t*
plwasm_wasm_pglib_query_text_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.query_text_unsafe";

  plwasm_call_context_t *cctx;
  int		arg1_out_mem_offset;
  size_t	arg2_out_mem_sz;
  int		arg3_stmt_mem_offset;
  size_t	arg4_stmt_mem_sz;
  char		*stmt_txt;
  Datum		val;
  char		*cstr;
  bool		is_null;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);

  arg1_out_mem_offset = args[0].of.i32;
  arg2_out_mem_sz = args[1].of.i32;
  arg3_stmt_mem_offset = args[2].of.i32;
  arg4_stmt_mem_sz = args[3].of.i32;

  stmt_txt = plwasm_wasm_mem_get_string(cctx, arg3_stmt_mem_offset, arg4_stmt_mem_sz);

  val = plwasm_spi_query_scalar_as(cctx, stmt_txt, CSTRINGOID, &is_null);

  cstr = DatumGetCString(val);
  plwasm_wasm_mem_put_cstring(
    cctx,
    arg1_out_mem_offset,
    arg2_out_mem_sz,
    cstr,
    strlen(cstr));

  results[0].of.i32 = strlen(cstr);

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}


static wasm_trap_t*
plwasm_wasm_pglib_statement_new_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.statement_new_unsafe";

  plwasm_call_context_t *cctx;
  int		arg1_stmt_mem_offset;
  size_t	arg2_stmt_mem_sz;
  char		*stmt_txt;
  plwasm_pg_statement_context_t	*stmctx;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_stmt_mem_offset = args[0].of.i32;
  arg2_stmt_mem_sz = args[1].of.i32;

  stmt_txt = plwasm_wasm_mem_get_string(cctx, arg1_stmt_mem_offset, arg2_stmt_mem_sz);

  plwasm_spi_ready(cctx);
  stmctx = plwasm_spi_statement_new(cctx, stmt_txt);
	
  results[0].of.i32 = stmctx->id;
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

static wasm_trap_t*
plwasm_wasm_pglib_statement_prepare(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.statement_prepare";

  plwasm_call_context_t *cctx;
  plwasm_pg_statement_context_t *stmctx;
  int		arg1_stmt_id;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_stmt_id = args[0].of.i32;

  stmctx = plwasm_spi_statement_get_context(cctx, arg1_stmt_id);
  plwasm_spi_statement_prepare(cctx, stmctx);
	
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

static wasm_trap_t*
plwasm_wasm_pglib_statement_execute(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.statement_excute";

  plwasm_call_context_t *cctx;
  plwasm_pg_statement_context_t *stmctx;
  int		arg1_stmt_id;
  int64_t	affected;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_stmt_id = args[0].of.i32;

  stmctx = plwasm_spi_statement_get_context(cctx, arg1_stmt_id);
  affected = plwasm_spi_statement_execute(cctx, stmctx, 0);

  results[0].kind = WASMTIME_I64;
  results[0].of.i64 = affected;
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

static wasm_trap_t*
plwasm_wasm_pglib_statement_close(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.statement_close";

  plwasm_call_context_t		*cctx;
  plwasm_pg_statement_context_t	*stmctx;
  int				arg1_stmt_id;
  int				result;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_stmt_id = args[0].of.i32;

  stmctx = plwasm_spi_statement_get_context(cctx, arg1_stmt_id);
  result = plwasm_spi_statement_close(cctx, stmctx);

  results[0].of.i32 = result;
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

void
plwasm_wasm_pglib_statement_load(
    plwasm_extension_context_t *ectx
) {
  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "query_int32_unsafe",
    plwasm_wasm_pglib_query_int32_unsafe,
    wasm_valtype_new_i32(),
    2,
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "query_text_unsafe",
    plwasm_wasm_pglib_query_text_unsafe,
    wasm_valtype_new_i32(),
    4,
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "statement_new_unsafe",
    plwasm_wasm_pglib_statement_new_unsafe,
    wasm_valtype_new_i32(),
    2,
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_0(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "statement_prepare",
    plwasm_wasm_pglib_statement_prepare,
    1,
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "statement_execute",
    plwasm_wasm_pglib_statement_execute,
    wasm_valtype_new_i64(),
    1,
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "statement_close",
    plwasm_wasm_pglib_statement_close,
    wasm_valtype_new_i32(),
    1,
    wasm_valtype_new_i32());
}
