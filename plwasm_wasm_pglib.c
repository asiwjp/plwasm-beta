#include "plwasm_wasm_pglib.h"
#include "plwasm_wasm_pglib_core.h"
#include "plwasm_wasm_pglib_command.h"
#include "plwasm_wasm_pglib_resultset.h"
#include "plwasm_wasm_memory.h"
#include "plwasm_wasm_utils.h"
#include "plwasm_utils_pg.h"
#include "plwasm_utils_str.h"
#include "plwasm_log.h"
#include <postgres.h>
#include <mb/pg_wchar.h>

#include <iconv.h>

#define WASM_MODULE_NAME "pg"
#define WASM_MODULE_NAME_LEN 2

void
plwasm_wasm_pglib_init(
    plwasm_extension_context_t *ectx
) {
  char* FUNC_NAME = "plwasm_wasm_pglib_init";
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

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "args_is_null",
    plwasm_wasm_pglib_args_is_null,
    wasm_valtype_new_i32(),
    1,
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "args_get_int32",
    plwasm_wasm_pglib_args_get_int32,
    wasm_valtype_new_i32(),
    1,
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "args_get_text_unsafe",
    plwasm_wasm_pglib_args_get_text_unsafe,
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
    "args_get_bytea_unsafe",
    plwasm_wasm_pglib_args_get_bytea_unsafe,
    wasm_valtype_new_i32(),
    6,
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_0(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "returns_set_null",
    plwasm_wasm_pglib_returns_set_null,
    0);

  plwasm_wasm_define_func_0(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "returns_set_int32",
    plwasm_wasm_pglib_returns_set_int32,
    1,
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_0(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "returns_set_text_unsafe",
    plwasm_wasm_pglib_returns_set_text_unsafe,
    2,
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_0(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "returns_set_bytea_unsafe",
    plwasm_wasm_pglib_returns_set_bytea_unsafe,
    4,
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32());

  /*
   * command api
   */
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
    "command_create_unsafe",
    plwasm_wasm_pglib_command_create_unsafe,
    wasm_valtype_new_i32(),
    2,
    wasm_valtype_new_i32(),
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_0(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "command_prepare",
    plwasm_wasm_pglib_command_prepare,
    1,
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "command_execute",
    plwasm_wasm_pglib_command_execute,
    wasm_valtype_new_i64(),
    1,
    wasm_valtype_new_i32());

  plwasm_wasm_define_func_1(
    ectx,
    WASM_MODULE_NAME,
    WASM_MODULE_NAME_LEN,
    "command_close",
    plwasm_wasm_pglib_command_close,
    wasm_valtype_new_i32(),
    1,
    wasm_valtype_new_i32());

  /*
   * resultset api
   */
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

  ectx->modules.pg = module_pg;
}

void
plwasm_wasm_pglib_bind(
  plwasm_call_context_t *cctx
) {
  wasmtime_error_t *error;

  error = wasmtime_linker_module(
	cctx->ectx->linker, 
	cctx->rt.context, 
	WASM_MODULE_NAME, 
	WASM_MODULE_NAME_LEN,
	cctx->ectx->modules.pg);
  if (error != NULL)
    CALL_WASM_ERROR(cctx, "define module failed.", error, NULL);

  cctx->spi.connected = false;
}

