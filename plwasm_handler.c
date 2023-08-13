#include "plwasm.h"
#include "plwasm_spi.h"
#include "plwasm_wasm_engine.h"
#include "plwasm_wasm.h"
#include "plwasm_wasm_module.h"
#include "plwasm_log.h"
#include "postgres.h"
//#include "catalog/pg_proc.h"
//#include "catalog/pg_type.h"
#include "commands/event_trigger.h"
#include "commands/trigger.h"
//#include "executor/spi.h"
//#include "funcapi.h"
//#include "utils/builtins.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(plwasm_call_handler);

static plwasm_extension_context_t ectx;

Datum plwasm_call_handler(
  PG_FUNCTION_ARGS
);

static Datum plwasm_func_handler(
  plwasm_call_context_t *cctx,
  PG_FUNCTION_ARGS
);

static void plwasm_call_context_init(
  plwasm_call_context_t *cctx,
  plwasm_extension_context_t *ectx,
  FunctionCallInfo fcinfo
);

static void plwasm_call_context_destroy(
  plwasm_call_context_t *cctx
);

void
_PG_init() {
   plwasm_wasm_engine_new(&ectx);
}


static void plwasm_call_context_init(
  plwasm_call_context_t *cctx,
  plwasm_extension_context_t *ectx,
  FunctionCallInfo fcinfo
) {
  cctx->type = plwasm_CTX_TYPE_CALL;
  cctx->ectx = ectx;
  cctx->memctx_default = NULL;
  cctx->rt.store = NULL;
  cctx->func_config.file = NULL;
  cctx->func_config.wat = NULL;
  cctx->func_config.func_name = NULL;
  cctx->func_config.string_enc_name = NULL;
  cctx->func_config.trace = false;
  cctx->func_config.stats = false;
  cctx->ret.type = VOIDOID;
  cctx->entry_point_name = "";
  cctx->fcinfo = fcinfo;

  cctx->rt.store = wasmtime_store_new(cctx->ectx->engine, cctx, NULL);
  if (cctx->rt.store == NULL)
    CALL_WASM_ERROR(cctx, "failed to create store", NULL, NULL);

  cctx->rt.context = wasmtime_store_context(cctx->rt.store);
}

static void plwasm_call_context_destroy(
  plwasm_call_context_t *cctx
) {
  if (cctx->rt.store != NULL) {
    wasmtime_store_delete(cctx->rt.store);
    cctx->rt.store = NULL;
  }
}

Datum
plwasm_call_handler(PG_FUNCTION_ARGS)
{
    Datum retval = (Datum) 0;
    plwasm_call_context_t cctx;
    struct timespec begin_time;
 
    cctx.func_config.stats = false;

    PG_TRY();
    {
      
      plwasm_log_stopwatch_begin(begin_time);
      plwasm_call_context_init(&cctx, &ectx, fcinfo);
      cctx.times.begin = begin_time;
      plwasm_log_stopwatch_save(&cctx, cctx.times.initted);

      if (CALLED_AS_TRIGGER(fcinfo) || CALLED_AS_EVENT_TRIGGER(fcinfo))
      {
      }
      else
      {
        retval = plwasm_func_handler(&cctx, fcinfo);
      }
    }
    PG_FINALLY();
    {
      plwasm_spi_finish(&cctx);
      plwasm_call_context_destroy(&cctx);
      plwasm_log_stopwatch_save(&cctx, cctx.times.ended);
    }
    PG_END_TRY();

    if (cctx.func_config.stats) {
      EXT_DEBUG5(&ectx,
         "total=%1.3f[ms] (init=%f, load=%f, instantiate=%f, ep_find=%f, ep_invoke=%f, fn_find=%f, fn_invoke=%f, release=%f)",
         compute_elapsed_msec(&cctx.times.begin,   &cctx.times.ended),
         compute_elapsed_msec(&cctx.times.begin,   &cctx.times.initted),
         compute_elapsed_msec(&cctx.times.initted, &cctx.times.loaded),
         compute_elapsed_msec(&cctx.times.loaded,  &cctx.times.instantiated),
         compute_elapsed_msec(&cctx.times.instantiated,           &cctx.times.entry_point_find_ended),
         compute_elapsed_msec(&cctx.times.entry_point_find_ended, &cctx.times.entry_point_invoked),
         compute_elapsed_msec(&cctx.times.entry_point_invoked,    &cctx.times.func_find_ended),
         compute_elapsed_msec(&cctx.times.func_find_ended,        &cctx.times.func_invoked),
         compute_elapsed_msec(&cctx.times.func_invoked,           &cctx.times.ended)
      );
    }
    return retval;
}

static Datum
plwasm_func_handler(plwasm_call_context_t *cctx, PG_FUNCTION_ARGS)
{
	Datum		wasm_retval;
        wasmtime_module_t *module;

	module = plwasm_wasm_module_load_with_cache(cctx, fcinfo->flinfo->fn_oid);
        plwasm_log_stopwatch_save(cctx, cctx->times.loaded);

	plwasm_wasm_module_instantiate(cctx, module);
        plwasm_log_stopwatch_save(cctx, cctx->times.instantiated);

	wasm_retval = plwasm_wasm_invoke(cctx, cctx->pg_proc.name, cctx->pg_proc.ret_type);
        plwasm_log_stopwatch_save(cctx, cctx->times.invoked);

	if (cctx->pg_proc.ret_type == VOIDOID) {
		PG_RETURN_NULL();
	}

	PG_RETURN_DATUM(wasm_retval);
}

