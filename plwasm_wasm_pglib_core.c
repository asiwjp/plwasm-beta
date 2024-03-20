#include "plwasm_wasm_pglib_core.h"
#include "plwasm_wasm_memory.h"
#include "plwasm_wasm_utils.h"
#include "plwasm_utils_pg.h"
#include "plwasm_utils_str.h"
#include "plwasm_log.h"
#include <postgres.h>
#include <mb/pg_wchar.h>

#define WASM_MODULE_NAME "pg"
#define WASM_MODULE_NAME_LEN 2

static wasm_trap_t*
plwasm_wasm_pglib_log_unsafe(
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

void
plwasm_wasm_pglib_core_load(
    plwasm_extension_context_t *ectx
) {
  plwasm_wasm_define_func_0(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "log_unsafe",
    plwasm_wasm_pglib_log_unsafe,
    3,
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32());
}

