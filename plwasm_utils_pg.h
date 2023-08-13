#ifndef H_PLWASM_UTILS_PG
#define H_PLWASM_UTILS_PG

#include <postgres.h>
#include "plwasm_types.h"

#define plwasm_utils_pg_pfree(ptr) \
  if (ptr != NULL) pfree(ptr); \
  ptr = NULL;

MemoryContext
plwasm_utils_pg_memctx_switch_to_proc_context(
  plwasm_call_context_t *cctx
);

void
plwasm_utils_pg_proc_check_arg_index(
  plwasm_call_context_t *cctx,
  FunctionCallInfo fcinfo,
  int index
);

Datum
plwasm_utils_pg_type_cast(
  plwasm_call_context_t *cctx,
  Datum val,
  Oid src_type,
  Oid dest_type
);

Datum
plwasm_utils_pg_text_to(
  plwasm_call_context_t *cctx,
  text *val,
  Oid to_type
);

Datum
plwasm_utils_pg_cstring_to(
  plwasm_call_context_t *cctx,
  char *val,
  Oid to_type
);
#endif
