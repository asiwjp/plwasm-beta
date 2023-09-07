#ifndef H_PLWASM_CONTEXT
#define H_PLWASM_CONTEXT
#include "plwasm_types.h"
//#include "postgres.h"
//#include "catalog/pg_proc.h"
//#include "catalog/pg_type.h"
//#include "commands/event_trigger.h"
//#include "commands/trigger.h"
//#include "executor/spi.h"
//#include "funcapi.h"
//#include "utils/builtins.h"

/*
 * Extension Context
 */
void
plwasm_extension_context_init(
  plwasm_extension_context_t *ectx
);

/*
 * Call Context
 */
void
plwasm_call_context_init(
  plwasm_call_context_t *cctx,
  plwasm_extension_context_t *ectx,
  FunctionCallInfo fcinfo
);

void
plwasm_call_context_destroy(
  plwasm_call_context_t *cctx
);

#endif
