#include "plwasm_wasm_utils.h"
#include "plwasm_log.h"
#include "postgres.h"
#include <stdarg.h>

void plwasm_wasm_define_func(
  plwasm_extension_context_t *ectx,
  char *module_name,
  int  module_name_len,
  char *func_name,
  void *func,
  wasm_functype_t *func_type
);

int
plwasm_wasm_vals_to_str(
  char *buf,
  int buf_sz,
  const wasmtime_val_t *vals,
  int n,
  const char *sep,
  const char *default_string
) {
  char *bufp = buf;
  int sep_len = strlen(sep);
  int remains = buf_sz - 4;
  int written;

  if (buf == NULL || buf_sz < 4) {
    ereport(ERROR, (errmsg("plwasm_wasm_vals_to_str invalid arg.")));
  }

  if (n == 0) {
    return snprintf(bufp, buf_sz, "%s", default_string);
  }


  for (int i = 0; i < n; ++i) {
    char vstr[128];
    int vstr_len = plwasm_wasm_val_to_str(vstr, sizeof(vstr), &(vals[i]));
   
    if (i > 0) {
      if (remains < sep_len) {
         snprintf(bufp, remains, "...");
         break;
      }
      written = snprintf(bufp, remains, "%s", sep);
      bufp += written;
      remains -= written;
    }

    if (remains < vstr_len) {  
      snprintf(bufp, remains, "...");
      break;
    }
    written = snprintf(bufp, remains, "%s", vstr);
    bufp += written;
    remains -= written;
  }

  return bufp - buf;
}

int
plwasm_wasm_val_to_str(
  char *buf,
  int buf_sz,
  const wasmtime_val_t *val
) {
  switch (val->kind) {
    case WASMTIME_I32:
      return snprintf(buf, buf_sz, "%d", val->of.i32);
    case WASMTIME_I64:
      return snprintf(buf, buf_sz, "%ld", val->of.i64);
    case WASMTIME_F32:
      return snprintf(buf, buf_sz, "%f", val->of.f32);
    case WASMTIME_F64:
      return snprintf(buf, buf_sz, "%lf", val->of.f64);
  }

  return 0;
}

plwasm_call_context_t*
plwasm_wasm_func_begin(
  wasmtime_caller_t *caller,
  const char* func_name,
  const wasmtime_val_t *args,
  int nargs
) {
  plwasm_call_context_t *cctx;

  cctx = (plwasm_call_context_t*) wasmtime_context_get_data(
    wasmtime_caller_context(caller)
  );

  if (cctx->func_config.trace)
    CALL_TRACE_WASM_FUNC_BEGIN(cctx, func_name, args, nargs);

  return cctx;
}

void
plwasm_wasm_func_end(
  plwasm_call_context_t *cctx,
  const char* func_name,
  const wasmtime_val_t *results,
  int nresults
) {

  if (cctx->func_config.trace)
    CALL_TRACE_WASM_FUNC_END(cctx, func_name, results, nresults);
}

void plwasm_wasm_define_func_0(
  plwasm_extension_context_t *ectx,
  char *module_name,
  int  module_name_len,
  char *func_name,
  void *func,
  int  argsn,
  ...
) {
  wasm_functype_t	*func_type;
  wasm_valtype_vec_t	params, results;
  wasm_valtype_t	**ps = NULL;
  va_list		va;

  if (argsn > 0) {
    ps = (wasm_valtype_t**) palloc(sizeof(wasm_valtype_t*) * argsn);
    va_start(va, argsn);
    for (int i = 0; i < argsn; ++i) {
      ps[i] = va_arg(va, wasm_valtype_t*);
    }
    va_end(va);
    wasm_valtype_vec_new(&params, argsn, ps);
  }

  wasm_valtype_vec_new_empty(&results);

  func_type = wasm_functype_new(&params, &results);
  plwasm_wasm_define_func(ectx, module_name, module_name_len, func_name, func, func_type);
  if (ps != NULL) {
    pfree(ps);
  }
}

void plwasm_wasm_define_func_1(
  plwasm_extension_context_t *ectx,
  char *module_name,
  int  module_name_len,
  char *func_name,
  void *func,
  wasm_valtype_t *result_type,
  int  argsn,
  ...
) {
  wasm_functype_t	*func_type;
  wasm_valtype_vec_t	params, results;
  wasm_valtype_t	**ps = NULL;
  wasm_valtype_t*	rs[1] = {result_type};
  va_list		va;

  if (argsn > 0) {
    ps = (wasm_valtype_t**) palloc(sizeof(wasm_valtype_t*) * argsn);
    va_start(va, argsn);
    for (int i = 0; i < argsn; ++i) {
      ps[i] = va_arg(va, wasm_valtype_t*);
    }
    va_end(va);
    wasm_valtype_vec_new(&params, argsn, ps);
  }

  wasm_valtype_vec_new(&results, 1, rs);

  func_type = wasm_functype_new(&params, &results);
  plwasm_wasm_define_func(ectx, module_name, module_name_len, func_name, func, func_type);
  if (ps != NULL) {
    pfree(ps);
  }
}

void plwasm_wasm_define_func(
  plwasm_extension_context_t *ectx,
  char *module_name,
  int  module_name_len,
  char *func_name,
  void *func,
  wasm_functype_t *func_type
) {
  wasmtime_error_t *error;

  EXT_DEBUG5(ectx, "define function. name=%s.%s", module_name, func_name);

  error = wasmtime_linker_define_func(
    ectx->linker, 
    module_name,
    module_name_len, 
    func_name,
    strlen(func_name),
    func_type,
    func,
    NULL,
    NULL);
  if (error != NULL)
    EXT_WASM_ERROR(ectx, "define function failed.", error, NULL);
}

