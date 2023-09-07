#include "plwasm_wasm_memory.h"
#include "plwasm_log.h"
#include "plwasm_utils_str.h"
#include "mb/pg_wchar.h"

char*
plwasm_wasm_mem_get_string(
    plwasm_call_context_t *cctx,
    int offset,
    size_t sz
) {
  size_t converted_sz = 0;
  char *txt = plwasm_wasm_mem_offset(
	cctx, offset, sz, true, NULL);

  return plwasm_utils_str_enc(
    cctx,
    txt,
    sz,
    cctx->func_config.string_enc,
    GetDatabaseEncoding(),
    true,
    &converted_sz);
}

char*
plwasm_wasm_mem_put_cstring(
    plwasm_call_context_t *cctx,
    int dest_offset,
    size_t dest_sz,
    char *cstr,
    size_t cstr_sz
) {
  char	*mem_ptr;
  char	*encoded;
  size_t encoded_sz;

  encoded = plwasm_utils_str_enc(
    cctx,
    cstr,
    cstr_sz, 
    GetDatabaseEncoding(),
    cctx->func_config.string_enc,
    false,
    &encoded_sz);
  if (dest_sz < encoded_sz) {
    CALL_ERROR(cctx,
      "buffer too small. buffer_size=%ld, required=%ld",
      dest_sz, encoded_sz);
  }

  mem_ptr = plwasm_wasm_mem_offset(cctx, dest_offset, cstr_sz, false, NULL);
  memcpy(mem_ptr, encoded, encoded_sz);
  return encoded;
}

char*
plwasm_wasm_mem_offset(
    plwasm_call_context_t *cctx,
    int offset,
    int desired_sz,
    bool strict,
    int *accessible_sz
) {
  wasmtime_extern_t ext;
  bool    ok;
  char   *memory;
  //size_t memory_sz;
  //int    accessible_sz_tmp;

  if (desired_sz < 0) {
    CALL_ERROR(cctx, "Invalid argument. desired_size=%d", desired_sz);
  }

  ok = wasmtime_instance_export_get(
    cctx->ectx->rt.context,
    cctx->instance,
    "memory",
    6,
    (wasmtime_extern_t*)&ext);
  if (!ok) {
    CALL_ERROR(cctx, "export memory was not found.");
  }

  memory = (char*) wasmtime_memory_data(
    cctx->ectx->rt.context, 
    &(ext.of.memory));
  //memory_sz = wasmtime_memory_size(
  //  cctx->rt.context,
  //  &(ext.of.memory));

  //accessible_sz_tmp = memory_sz - offset;
  //if (accessible_sz_tmp < 0) {
  //  CALL_ERROR(cctx,
  //    "memory offset is out of range. memory_offset=%d, memory_size=%ld, accessible_sz=%d, desired_size=%d",
  //    offset, memory_sz, accessible_sz_tmp, desired_sz);
  //}

  //if (accessible_sz_tmp < desired_sz) {
  //  if (strict) {
  //    CALL_ERROR(cctx,
  //      "memory is too small. memory_offset=%d, memory_size=%ld, accessible_sz=%d, desired_size=%d",
  //      offset, memory_sz, accessible_sz_tmp, desired_sz);
  //  }
  //} else {
  //  accessible_sz_tmp = desired_sz;
  //}
  //
  //if (accessible_sz != NULL) {
  //  *accessible_sz = accessible_sz_tmp;
  //}
  return memory + offset;
}
