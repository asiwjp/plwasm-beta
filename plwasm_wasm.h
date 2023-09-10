#ifndef H_PLWASM_WASM
#define H_PLWASM_WASM

#include "plwasm_types.h"

Datum
plwasm_wasm_invoke(
	plwasm_call_context_t *cctx,
	wasmtime_instance_t *instance,
	const char *func_name,
	Oid expect_ret_type
);

#endif
