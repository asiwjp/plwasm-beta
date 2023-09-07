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
	char* proname,
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
     || expect_ret_type == VOIDOID
  ) {
    nresults = 0;

  } else {
    nresults = 1;
  }

  wasmtime_context_set_data(cctx->ectx->rt.context, cctx);

  CALL_DEBUG5(cctx, "Calling target function %s", cctx->func_config.func_name);
  found = wasmtime_instance_export_get(
    cctx->ectx->rt.context,
    cctx->instance,
    cctx->func_config.func_name,
    strlen(cctx->func_config.func_name),
    &run);
  plwasm_log_stopwatch_save(cctx, cctx->times.func_find_ended);
  if (!found)
    CALL_ERROR(cctx, "expected function was not found. name=%s", cctx->func_config.func_name);

  trap = NULL;
  error = wasmtime_func_call(cctx->ectx->rt.context, &run.of.func, NULL, 0, results, nresults, &trap);
  plwasm_log_stopwatch_save(cctx, cctx->times.func_invoked);
  if (error != NULL || trap != NULL)
    CALL_WASM_ERROR(cctx, "failed to call function", error, trap);

  if (cctx->ret.type == VOIDOID) {
    CALL_DEBUG5(cctx, "%s return null", proname);
    PG_RETURN_NULL();
  }

  if (expect_ret_type != cctx->ret.type) {
    CALL_ERROR(cctx, "%s return type is missmatch.", proname);
  }

  if (expect_ret_type == BYTEAOID) {
    CALL_DEBUG5(cctx, "%s return BYTEA(%d)", proname, VARSIZE(cctx->ret.of.bytea));
    PG_RETURN_BYTEA_P(cctx->ret.of.bytea);
  }

  if (expect_ret_type == CSTRINGOID) {
    CALL_DEBUG5(cctx, "%s return %s", proname, cctx->ret.of.cstring.ptr);
    PG_RETURN_CSTRING(cctx->ret.of.cstring.ptr);
  }

  if (expect_ret_type == TEXTOID) {
    CALL_DEBUG5(cctx, "%s return %s", proname, VARDATA(cctx->ret.of.text));
    PG_RETURN_TEXT_P(cctx->ret.of.text);
  }

  if (expect_ret_type == INT4OID) {
    CALL_DEBUG5(cctx, "%s return %d", proname, cctx->ret.of.i32);
    PG_RETURN_INT32(cctx->ret.of.i32);
  }

  if (nresults == 0) {
    CALL_DEBUG5(cctx, "%s return null", proname);
    PG_RETURN_NULL();
  }

  if (results[0].kind == WASMTIME_I32)
    PG_RETURN_INT32(results[0].of.i32);

  CALL_DEBUG5(cctx, "%s return null", proname);
  PG_RETURN_NULL();
}
