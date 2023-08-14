#include "plwasm_wasm_pglib_command.h"
#include "plwasm_spi.h"
#include "plwasm_wasm_memory.h"
#include "plwasm_wasm_utils.h"
#include "plwasm_utils_pg.h"
#include "plwasm_utils_str.h"
#include "plwasm_log.h"

wasm_trap_t*
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
  int		arg1_cmd_mem_offset;
  size_t	arg2_cmd_mem_sz;
  char		*cmd_txt;
  Datum		val;
  bool		is_null;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_cmd_mem_offset = args[0].of.i32;
  arg2_cmd_mem_sz = args[1].of.i32;

  cmd_txt = plwasm_wasm_mem_get_string(
	cctx, arg1_cmd_mem_offset, arg2_cmd_mem_sz);

  val = plwasm_spi_query_scalar_as(cctx, cmd_txt, INT4OID, &is_null);

  results[0].of.i32 = DatumGetInt32(val);

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t*
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
  int		arg3_cmd_mem_offset;
  size_t	arg4_cmd_mem_sz;
  char		*cmd_txt;
  Datum		val;
  char		*cstr;
  bool		is_null;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);

  arg1_out_mem_offset = args[0].of.i32;
  arg2_out_mem_sz = args[1].of.i32;
  arg3_cmd_mem_offset = args[2].of.i32;
  arg4_cmd_mem_sz = args[3].of.i32;

  cmd_txt = plwasm_wasm_mem_get_string(cctx, arg3_cmd_mem_offset, arg4_cmd_mem_sz);

  val = plwasm_spi_query_scalar_as(cctx, cmd_txt, CSTRINGOID, &is_null);

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


wasm_trap_t*
plwasm_wasm_pglib_command_create_unsafe(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.command_create_unsafe";

  plwasm_call_context_t *cctx;
  int		arg1_cmd_mem_offset;
  size_t	arg2_cmd_mem_sz;
  char		*cmd_txt;
  plwasm_pg_command_context_t	*cmdctx;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_cmd_mem_offset = args[0].of.i32;
  arg2_cmd_mem_sz = args[1].of.i32;

  cmd_txt = plwasm_wasm_mem_get_string(cctx, arg1_cmd_mem_offset, arg2_cmd_mem_sz);

  plwasm_spi_ready(cctx);
  cmdctx = plwasm_spi_command_create(cctx, cmd_txt);
	
  results[0].of.i32 = cmdctx->id;
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t*
plwasm_wasm_pglib_command_prepare(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.command_prepare";

  plwasm_call_context_t *cctx;
  plwasm_pg_command_context_t *cmdctx;
  int		arg1_cmd_id;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_cmd_id = args[0].of.i32;

  cmdctx = plwasm_spi_command_get_context(cctx, arg1_cmd_id);
  plwasm_spi_command_prepare(cctx, cmdctx);
	
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t*
plwasm_wasm_pglib_command_execute(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.command_excute";

  plwasm_call_context_t *cctx;
  plwasm_pg_command_context_t *cmdctx;
  int		arg1_cmd_id;
  int64_t	affected;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_cmd_id = args[0].of.i32;

  cmdctx = plwasm_spi_command_get_context(cctx, arg1_cmd_id);
  affected = plwasm_spi_command_execute(cctx, cmdctx, 0);

  results[0].kind = WASMTIME_I64;
  results[0].of.i64 = affected;
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t*
plwasm_wasm_pglib_command_close(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
  char *FUNC_NAME = "pg.command_close";

  plwasm_call_context_t		*cctx;
  plwasm_pg_command_context_t	*cmdctx;
  int				arg1_cmd_id;
  int				result;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_cmd_id = args[0].of.i32;

  cmdctx = plwasm_spi_command_get_context(cctx, arg1_cmd_id);
  result = plwasm_spi_command_close(cctx, cmdctx);

  results[0].of.i32 = result;
  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}
