#include "plwasm_context.h"
#include "plwasm_spi.h"
#include "plwasm_wasm_engine.h"
#include "plwasm_wasm.h"
#include "plwasm_wasm_module.h"
#include "plwasm_log.h"
#include "postgres.h"
#include "utils/guc.h"
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

void
_PG_init() {
   plwasm_extension_context_init(&ectx);

   DefineCustomBoolVariable("plwasm.trace",
				"Report trace information for PL/wasm extension.",
				 NULL,
				 &ectx.config.trace,
				 false,
				 PGC_SUSET,
				 0,
				 NULL,
				 NULL,
				 NULL);

   DefineCustomIntVariable("plwasm.trace_threshold",
				"This is the detailed degree of trace logging for PL/wasm extension.",
				 NULL,
				 &ectx.config.trace_threshold,
				 0,
				 0,
				 5,
				 PGC_SUSET,
				 0,
				 NULL,
				 NULL,
				 NULL);

   DefineCustomBoolVariable("plwasm.timing",
				"Reports processing time information for PL/wasm extension.",
				 NULL,
				 &ectx.config.timing,
				 false,
				 PGC_SUSET,
				 0,
				 NULL,
				 NULL,
				 NULL);

   DefineCustomBoolVariable("plwasm.cache.instance",
				"Enable caching of WASM module instances.",
				 NULL,
				 &ectx.config.cache.instance.enabled,
				 false,
				 PGC_SUSET,
				 0,
				 NULL,
				 NULL,
				 NULL);
   plwasm_wasm_engine_new(&ectx);
}

Datum
plwasm_call_handler(PG_FUNCTION_ARGS)
{
    Datum retval = (Datum) 0;
    plwasm_call_context_t cctx;
    struct timespec begin_time;
 
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
      plwasm_wasm_module_extra_release(&cctx);
      plwasm_call_context_destroy(&cctx);
      plwasm_log_stopwatch_save(&cctx, cctx.times.ended);
    }
    PG_END_TRY();

    if (cctx.func_config.timing) {
      EXT_INFO(&ectx,
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
	Oid		fn_oid = fcinfo->flinfo->fn_oid;

	cctx->module = plwasm_wasm_module_load_with_cache(cctx, fn_oid);

	cctx->instance = plwasm_wasm_module_instantiate_with_cache(cctx, fn_oid, cctx->module);

	plwasm_wasm_module_extra_init(cctx);

	wasm_retval = plwasm_wasm_invoke(cctx, cctx->pg_proc.name, cctx->pg_proc.ret_type);

	if (cctx->pg_proc.ret_type == VOIDOID) {
		PG_RETURN_NULL();
	}

	PG_RETURN_DATUM(wasm_retval);
}

