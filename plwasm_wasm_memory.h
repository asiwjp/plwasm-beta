#ifndef H_PLWASM_WASM_MEMORY
#define H_PLWASM_WASM_MEMORY

#include "plwasm_types.h"

char*
plwasm_wasm_mem_get_string(
	plwasm_call_context_t *cctx,
	int offset,
	size_t sz
);

char*
plwasm_wasm_mem_put_cstring(
    plwasm_call_context_t *cctx,
    int dest_offset,
    size_t dest_sz,
    char *cstr,
    size_t cstrlen
);

char*
plwasm_wasm_mem_offset(
    plwasm_call_context_t *cctx,
    int offset,
    int desired_sz,
    bool strict,
    int *accessible_sz
);

#endif
