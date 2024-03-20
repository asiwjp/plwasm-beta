#include "plwasm_wasm_pglib.h"
#include "plwasm_wasm_pglib_core.h"
#include "plwasm_wasm_pglib_args.h"
#include "plwasm_wasm_pglib_returns.h"
#include "plwasm_wasm_pglib_statement.h"
#include "plwasm_wasm_pglib_resultset.h"
#include "plwasm_wasm_memory.h"
#include "plwasm_wasm_utils.h"
#include "plwasm_spi.h"
#include "plwasm_utils_pg.h"
#include "plwasm_log.h"
#include <postgres.h>

#define WASM_MODULE_NAME "pg"
#define WASM_MODULE_NAME_LEN 2

void
plwasm_wasm_pglib_load(
    plwasm_extension_context_t *ectx
) {
  char* FUNC_NAME = "plwasm_wasm_pglib_load";
  wasmtime_error_t *error;
  wasm_byte_vec_t wat_pg;
  wasm_byte_vec_t wasm_pg;
  wasmtime_module_t *module_pg;
  char *empty_src;
  int empty_src_length;

  EXT_DEBUG5(ectx, "%s begin", FUNC_NAME);

  empty_src = "(module)";
  empty_src_length = strlen(empty_src);
  wasm_byte_vec_new_uninitialized(&wat_pg, empty_src_length);
  error = wasmtime_wat2wasm(empty_src, empty_src_length, &wasm_pg);
  if (error != NULL)
    EXT_ERROR(ectx, "%s failed to parse wat", FUNC_NAME);

  module_pg = NULL;
  error = wasmtime_module_new(ectx->engine, (uint8_t*)wasm_pg.data, wasm_pg.size, &module_pg);
  if (error != NULL)
    EXT_ERROR(ectx, "%s failed to module_new", FUNC_NAME);

  plwasm_wasm_pglib_core_load(ectx);
  plwasm_wasm_pglib_args_load(ectx);
  plwasm_wasm_pglib_returns_load(ectx);
  plwasm_wasm_pglib_statement_load(ectx);
  plwasm_wasm_pglib_resultset_load(ectx);

  ectx->modules.pg = module_pg;
}

void
plwasm_wasm_pglib_bind(
  plwasm_call_context_t *cctx
) {
  wasmtime_error_t *error;

  error = wasmtime_linker_module(
	cctx->ectx->rt.linker, 
	cctx->ectx->rt.context, 
	WASM_MODULE_NAME, 
	WASM_MODULE_NAME_LEN,
	cctx->ectx->modules.pg);
  if (error != NULL)
    CALL_WASM_ERROR(cctx, "define module failed.", error, NULL);
}

void
plwasm_wasm_pglib_init(
  plwasm_call_context_t *cctx
) {
  cctx->spi.connected = false;
}

void
plwasm_wasm_pglib_release(
  plwasm_call_context_t *cctx
) {
  plwasm_spi_finish(cctx);
}
