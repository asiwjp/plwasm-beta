#ifndef H_PLWASM_FUNC_BODY
#define H_PLWASM_FUNC_BODY
#include "plwasm_types.h"

void
plwasm_func_body_describe(
  plwasm_call_context_t *cctx,
  plwasm_pg_proc_t *pg_proc,
  Oid pg_proc_oid
);

void 
plwasm_func_body_parse(
  plwasm_call_context_t *cctx,
  plwasm_pg_proc_t *pg_proc
);

#endif
