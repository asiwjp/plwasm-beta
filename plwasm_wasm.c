#include "plwasm.h"
#include "plwasm_log.h"
#include "plwasm_wasm.h"
#include "plwasm_wasm_pglib.h"
#include "plwasm_wasm_module.h"
#include "plwasm_utils_pg.h"

#include <stdio.h>
#include <sys/time.h>

#define ENTRY_POINT_FUNC_NAME_REACTOR "_initialize"
#define ENTRY_POINT_FUNC_NAME_START "_start"

bool plwasm_wasm_find_entry_point(plwasm_call_context_t *cctx, wasmtime_extern_t *run);

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

  bool ok;
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

  CALL_DEBUG5(cctx, "Calling entry point function.");
  ok = plwasm_wasm_find_entry_point(cctx, &run);
  plwasm_log_stopwatch_save(cctx, cctx->times.entry_point_find_ended);
  if (ok) {
    trap = NULL;
    error = wasmtime_func_call(cctx->rt.context, &run.of.func, NULL, 0, NULL, 0, &trap);
    plwasm_log_stopwatch_save(cctx, cctx->times.entry_point_invoked);
    if (error != NULL || trap != NULL) {
      CALL_WASM_ERROR(cctx, "failed to call entry point function", error, trap);
    }

    CALL_DEBUG5(cctx, "%s called.", cctx->entry_point_name);
  } else {
    plwasm_log_stopwatch_save(cctx, cctx->times.entry_point_find_ended);
    cctx->times.entry_point_invoked = cctx->times.entry_point_find_ended;
    CALL_DEBUG5(cctx, "Entry point was not found.");
  }

  CALL_DEBUG5(cctx, "Calling target function %s", cctx->func_config.func_name);
  ok = wasmtime_instance_export_get(
    cctx->rt.context,
    &(cctx->rt.instance),
    cctx->func_config.func_name,
    strlen(cctx->func_config.func_name),
    &run);
  plwasm_log_stopwatch_save(cctx, cctx->times.func_find_ended);
  if (!ok)
    CALL_ERROR(cctx, "expected function was not found. name=%s", cctx->func_config.func_name);

  trap = NULL;
  error = wasmtime_func_call(cctx->rt.context, &run.of.func, NULL, 0, results, nresults, &trap);
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

bool plwasm_wasm_find_entry_point(plwasm_call_context_t *cctx, wasmtime_extern_t *run) {
  bool found;
  char *entry_point_name;

  entry_point_name = ENTRY_POINT_FUNC_NAME_REACTOR;
  found = wasmtime_instance_export_get(
    cctx->rt.context,
    &(cctx->rt.instance),
    entry_point_name,
    strlen(entry_point_name),
    run);

  if (found) {
    cctx->entry_point_name = entry_point_name;
    return true;
  }

  entry_point_name = ENTRY_POINT_FUNC_NAME_START;
  found = wasmtime_instance_export_get(
    cctx->rt.context,
    &(cctx->rt.instance),
    entry_point_name,
    strlen(entry_point_name),
    run);
  
  if (found) {
    cctx->entry_point_name = entry_point_name;
    return true;
  }

  return false;
}

