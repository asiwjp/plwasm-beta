#include "plwasm_log.h"
#include "plwasm_wasm_utils.h"

double compute_elapsed_msec(struct timespec *begin, struct timespec *end) {
  double begin_nsec, end_nsec;
  begin_nsec = (double)(begin->tv_nsec);
  if (begin->tv_nsec <= end->tv_nsec) {
     end_nsec = (double)(end->tv_nsec);
  } else {
     end_nsec = (double)(end->tv_nsec) + (1000 * 1000 * 1000);
  }
  return (end_nsec - begin_nsec) / 1000 / 1000;
}

void EXT_WASM_ERROR(
  plwasm_extension_context_t *ectx, 
  const char *message,
  wasmtime_error_t *error,
  wasm_trap_t *trap
) {
  CALL_WASM_ERROR(NULL, message, error, trap);
}

void
CALL_TRACE_WASM_FUNC_BEGIN(
  plwasm_call_context_t *cctx, 
  const char *func_name,
  const wasmtime_val_t *args,
  int nargs
) {
  char buf[1024];
  char *bufp = buf;
  int  remains = sizeof(buf) - 4;
  int written;
  
  written = snprintf(bufp, remains, "%s called by ", func_name);
  bufp += written;
  remains -= written;
  plwasm_wasm_vals_to_str(bufp, remains, args, nargs, ", ", "no args");
  CALL_DEBUG1(cctx, "%s", buf);
}

void
CALL_TRACE_WASM_FUNC_END(
  plwasm_call_context_t *cctx, 
  const char *func_name,
  const wasmtime_val_t *results,
  int nresults
) {
  char buf[1024];
  char *bufp = buf;
  int  remains = sizeof(buf) - 4;
  int written;
  
  written = snprintf(bufp, remains, "%s return ", func_name);
  bufp += written;
  remains -= written;
  plwasm_wasm_vals_to_str(bufp, remains, results, nresults, ", ", "void");
  CALL_DEBUG1(cctx, "%s", buf);
}

void CALL_WASM_ERROR(
  plwasm_call_context_t *cctx, 
  const char *message,
  wasmtime_error_t *error,
  wasm_trap_t *trap
) {
  wasm_byte_vec_t error_message;

  if (error != NULL) {
    wasmtime_error_message(error, &error_message);
    wasmtime_error_delete(error);
  } else {
    wasm_trap_message(trap, &error_message);
    wasm_trap_delete(trap);
  }

  ereport(ERROR,
    (errmsg("%s wasm_error=%s", message, pnstrdup((char*) error_message.data, error_message.size))));
}
