#include "plwasm.h"
#include "plwasm_log.h"
#include "plwasm_wasm.h"
#include "plwasm_wasm_pglib.h"
#include "plwasm_wasm_module.h"
#include "plwasm_utils_pg.h"

#include <stdio.h>

Datum
plwasm_wasm_invoke(
  	plwasm_call_context_t *cctx,
	wasmtime_instance_t *instance,
	const char* func_name,
	Oid expect_ret_type
) {
  FunctionCallInfo fcinfo;
  wasmtime_error_t *error;
  wasmtime_extern_t run;
  wasmtime_val_t results[1];
  int nresults;

  bool found;
  wasm_trap_t *trap;

  fcinfo = cctx->fcinfo;
  trap = NULL;

  if (expect_ret_type == TEXTOID
     || expect_ret_type == CSTRINGOID
     || expect_ret_type == BYTEAOID
     || expect_ret_type == INT4OID
     || expect_ret_type == INT8OID
     || expect_ret_type == VOIDOID
  ) {
    nresults = 0;

  } else {
    nresults = 1;
  }

  wasmtime_context_set_data(cctx->ectx->rt.context, cctx);

  CALL_DEBUG5(cctx, "Calling target function %s", func_name);
  found = wasmtime_instance_export_get(
    cctx->ectx->rt.context,
    instance,
    func_name,
    strlen(func_name),
    &run);
  plwasm_log_stopwatch_save(cctx, cctx->times.func_find_ended);
  if (!found)
    CALL_ERROR(cctx, "expected function was not found. name=%s", func_name);

  trap = NULL;
  error = wasmtime_func_call(cctx->ectx->rt.context, &run.of.func, NULL, 0, results, nresults, &trap);
  plwasm_log_stopwatch_save(cctx, cctx->times.func_invoked);
  if (error != NULL || trap != NULL)
    CALL_WASM_ERROR(cctx, "failed to call function", error, trap);

  if (cctx->ret.type == VOIDOID) {
    CALL_DEBUG5(cctx, "%s return null", func_name);
    PG_RETURN_NULL();
  }

  if (expect_ret_type != cctx->ret.type) {
    CALL_ERROR(cctx, "%s return type is missmatch.", func_name);
  }

  if (expect_ret_type == BYTEAOID) {
    CALL_DEBUG5(cctx, "%s return BYTEA(%d)", func_name, VARSIZE(cctx->ret.of.bytea));
    PG_RETURN_BYTEA_P(cctx->ret.of.bytea);
  }

  if (expect_ret_type == CSTRINGOID) {
    CALL_DEBUG5(cctx, "%s return %s", func_name, cctx->ret.of.cstring.ptr);
    PG_RETURN_CSTRING(cctx->ret.of.cstring.ptr);
  }

  if (expect_ret_type == TEXTOID) {
    CALL_DEBUG5(cctx, "%s return %s", func_name, VARDATA(cctx->ret.of.text));
    PG_RETURN_TEXT_P(cctx->ret.of.text);
  }

  if (expect_ret_type == INT4OID) {
    CALL_DEBUG5(cctx, "%s return %d", func_name, cctx->ret.of.i32);
    PG_RETURN_INT32(cctx->ret.of.i32);
  }

  if (expect_ret_type == INT8OID) {
    CALL_DEBUG5(cctx, "%s return %ld", func_name, cctx->ret.of.i64);
    PG_RETURN_INT64(cctx->ret.of.i64);
  }

  if (nresults == 0) {
    CALL_DEBUG5(cctx, "%s return null", func_name);
    PG_RETURN_NULL();
  }

  if (results[0].kind == WASMTIME_I32)
    PG_RETURN_INT32(results[0].of.i32);

  CALL_DEBUG5(cctx, "%s return null", func_name);
  PG_RETURN_NULL();
}
