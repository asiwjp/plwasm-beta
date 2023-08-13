#include "plwasm_wasm_pglib_resultset.h"
#include "plwasm_spi.h"
#include "plwasm_wasm_memory.h"
#include "plwasm_wasm_utils.h"
#include "plwasm_utils_pg.h"
#include "plwasm_utils_str.h"
#include "plwasm_log.h"

#define WASM_MODULE_NAME "pg"

wasm_trap_t*
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
  plwasm_pg_command_context_t	*cmdctx;
  int		arg1_cmd_id;
  bool		result;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_cmd_id = args[0].of.i32;

  cmdctx = plwasm_spi_command_get_context(cctx, arg1_cmd_id);
  result = plwasm_spi_resultset_fetch(cctx, cmdctx);
  
  results[0].of.i32 = result ? 1 : 0;

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t*
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
  plwasm_pg_command_context_t	*cmdctx;
  int		arg1_cmd_id;
  bool		result;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_cmd_id = args[0].of.i32;

  cmdctx = plwasm_spi_command_get_context(cctx, arg1_cmd_id);
  result = plwasm_spi_resultset_close(cctx, cmdctx);

  results[0].of.i32 = result ? 1 : 0;

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}

wasm_trap_t*
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
  plwasm_pg_command_context_t	*cmdctx;
  int		arg1_cmd_id;
  int		arg2_fld_idx;
  Datum		val;
  bool		is_null;

  cctx = plwasm_wasm_func_begin(caller, FUNC_NAME, args, nargs);
  arg1_cmd_id  = args[0].of.i32;
  arg2_fld_idx = args[1].of.i32;

  cmdctx = plwasm_spi_command_get_context(cctx, arg1_cmd_id);
  val = plwasm_spi_resultset_get_val_as(cctx, cmdctx, arg2_fld_idx, INT4OID, &is_null);

  results[0].of.i32 = DatumGetInt32(val);

  plwasm_wasm_func_end(cctx, FUNC_NAME, results, nresults);
  return NULL;
}
