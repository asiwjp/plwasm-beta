
#include "plwasm_utils_pg.h"
#include "plwasm_log.h"
#include "catalog/pg_transform.h"
#include "utils/syscache.h"

MemoryContext
plwasm_utils_pg_memctx_switch_to_proc_context(
  plwasm_call_context_t *cctx
) {
  if (cctx->memctx_default == NULL) {
    cctx->memctx_default = AllocSetContextCreate(
      TopTransactionContext,
      "PL/WebAssmbly procedure context",
      ALLOCSET_DEFAULT_SIZES
    );
  }
  return MemoryContextSwitchTo(cctx->memctx_default);
}

void
plwasm_utils_pg_proc_check_arg_index(
  plwasm_call_context_t *cctx,
  FunctionCallInfo fcinfo,
  int index
) {
   if (0 >= index && index <= PG_NARGS()) {
     return;
   }
   CALL_ERROR(cctx, "Argument index is out of range. index=%d", index);
}

Datum
plwasm_utils_pg_type_cast(
  plwasm_call_context_t *cctx,
  Datum val,
  Oid from_type,
  Oid to_type
) {
	HeapTuple		tuple;
	Oid			funcid;

	if (from_type == TEXTOID) {
		return plwasm_utils_pg_text_to(cctx, DatumGetTextP(val), to_type);
	}

	if (from_type == CSTRINGOID) {
		return plwasm_utils_pg_cstring_to(cctx, DatumGetCString(val), to_type);
	}

	tuple = SearchSysCache2(TRFTYPELANG, from_type, to_type);
	if (!HeapTupleIsValid(tuple)) {
		CALL_ERROR(cctx, "transform type function was not found. %u to %u", from_type, to_type);
	}

	funcid = ((Form_pg_transform) GETSTRUCT(tuple))->trffromsql;
	ReleaseSysCache(tuple);

	return OidFunctionCall1(funcid, val);
}

Datum
plwasm_utils_pg_text_to(
  plwasm_call_context_t *cctx,
  text *val,
  Oid to_type
) {
	if (to_type == CSTRINGOID) {
		return (Datum) text_to_cstring(val);
	}
	CALL_ERROR(cctx, "Specified cast is not supported.");
}

Datum
plwasm_utils_pg_cstring_to(
  plwasm_call_context_t *cctx,
  char *val,
  Oid to_type
) {
	if (to_type == TEXTOID) {
		return (Datum) cstring_to_text(val);
	}
	CALL_ERROR(cctx, "Specified cast is not supported.");
}
