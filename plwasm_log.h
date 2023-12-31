#ifndef H_PLWASM_LOG
#define H_PLWASM_LOG
#include "plwasm_types.h"

#define CALL_FATAL(cctx, ...) \
  ereport(FATAL, (errmsg(__VA_ARGS__)));

#define CALL_ERROR(cctx, ...) \
  ereport(ERROR, (errmsg(__VA_ARGS__)));

#define CALL_INFO(cctx, ...) \
  ereport(INFO, (errmsg(__VA_ARGS__)));

#define CALL_DEBUG1(cctx, ...) \
  if ((cctx)->func_config.trace && (cctx)->func_config.trace_threshold >= 1) ereport(DEBUG1, (errmsg(__VA_ARGS__)));

#define CALL_DEBUG5(cctx, ...) \
  if ((cctx)->func_config.trace && (cctx)->func_config.trace_threshold == 5) ereport(DEBUG1, (errmsg(__VA_ARGS__)));

#define EXT_FATAL(ectx, ...) \
  ereport(ERROR, (errmsg(__VA_ARGS__)));

#define EXT_ERROR(ectx, ...) \
  ereport(ERROR, (errmsg(__VA_ARGS__)));

#define EXT_INFO(ectx, ...) \
  ereport(INFO, (errmsg(__VA_ARGS__)));

#define EXT_DEBUG5(ectx, ...) \
  if ((ectx)->config.trace && (ectx)->config.trace_threshold == 5) ereport(DEBUG1, (errmsg(__VA_ARGS__)));

#ifdef _MSC_VER
#define plwasm_log_stopwatch_begin(tm) timespec_get(&(tm), TIME_UTC)
#define plwasm_log_stopwatch_save(cctx, tm) \
  if ((cctx)->func_config.timing) { \
    timespec_get(&(tm), TIME_UTC);  \
  }
#else
#define plwasm_log_stopwatch_begin(tm) clock_gettime(CLOCK_REALTIME, &(tm))
#define plwasm_log_stopwatch_save(cctx, tm) if ((cctx)->func_config.timing) { clock_gettime(CLOCK_REALTIME, &(tm)); }
#endif

double
compute_elapsed_msec(
  struct timespec *begin,
  struct timespec *end
);

void
EXT_WASM_ERROR(
  plwasm_extension_context_t *ectx,
  const char *message,
  wasmtime_error_t *error,
  wasm_trap_t *trap
);

void
CALL_TRACE_WASM_FUNC_BEGIN(
  plwasm_call_context_t *cctx,
  const char* func_name,
  const wasmtime_val_t *args,
  int nargs
);

void
CALL_TRACE_WASM_FUNC_END(
  plwasm_call_context_t *cctx,
  const char* func_name,
  const wasmtime_val_t *results,
  int nresults
);

void
CALL_WASM_ERROR(
  plwasm_call_context_t *cctx,
  const char *message,
  wasmtime_error_t *error,
  wasm_trap_t *trap
);

#endif
