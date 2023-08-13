#include "plwasm_wasm_module.h"
#include "plwasm_func_body.h"
#include "plwasm_log.h"
#include "plwasm_utils_str.h"
#include "plwasm_utils_json.h"
#include "plwasm_wasm_pglib.h"

#include <stdio.h>
#include <stdbool.h>
#include <catalog/pg_collation_d.h>
#include <mb/pg_wchar.h>
#include <utils/jsonb.h>


void plwasm_wasm_load_file(
  plwasm_call_context_t *cctx,
  wasm_byte_vec_t *out,
  char *proname,
  char *source
);

wasmtime_module_t*
plwasm_wasm_module_load_with_cache(
  plwasm_call_context_t *cctx,
  Oid fn_oid
) {
  char *FUNC_NAME = "plwasm_wasm_module_load_with_cache";
  bool found;
  plwasm_wasm_module_cache_entry_t *cache_entry;
  wasm_byte_vec_t wasm;
  //volatile MemoryContext memctx;
  volatile MemoryContext old_memctx;

  cache_entry = (plwasm_wasm_module_cache_entry_t*) hash_search(
    cctx->ectx->wasm_module_cache,
    &fn_oid,
    HASH_ENTER,
    &found);
  if (found) {
    CALL_DEBUG5(cctx, "%s module cache was found. func_name=%s", FUNC_NAME, cache_entry->config.func_name);
    cctx->pg_proc = cache_entry->pg_proc;
    cctx->func_config.file = cache_entry->config.file;
    cctx->func_config.func_name = cache_entry->config.func_name;
    cctx->func_config.string_enc_name  = cache_entry->config.string_enc_name;
    cctx->func_config.string_enc = cache_entry->config.string_enc;
    cctx->func_config.trace = cache_entry->config.trace;
    return cache_entry->module;
  }

  CALL_DEBUG5(cctx, "%s module cache was not found. fn_oid=%d", FUNC_NAME, fn_oid);
  plwasm_func_body_describe(cctx, &(cctx->pg_proc), fn_oid);
  plwasm_func_body_parse(cctx, &(cctx->pg_proc));
  plwasm_wasm_load_wasm(cctx, &wasm, cctx->pg_proc.name);
  cache_entry->module = plwasm_wasm_module_create(cctx, &wasm);
  wasm_byte_vec_delete(&wasm);

  old_memctx = MemoryContextSwitchTo(cctx->ectx->memctx);
  cache_entry->pg_proc.name = pstrdup(cctx->pg_proc.name);
  cache_entry->pg_proc.source = pstrdup(cctx->pg_proc.source);
  cache_entry->pg_proc.source_len = cctx->pg_proc.source_len;
  cache_entry->pg_proc.ret_type = cctx->pg_proc.ret_type;
  cache_entry->config.file = (cctx->func_config.file != NULL) ? pstrdup(cctx->func_config.file) : NULL;
  cache_entry->config.func_name = pstrdup(cctx->func_config.func_name);
  cache_entry->config.string_enc_name = pstrdup(cctx->func_config.string_enc_name);
  cache_entry->config.string_enc = cctx->func_config.string_enc;
  cache_entry->config.trace = cctx->func_config.trace;
  MemoryContextSwitchTo(old_memctx);
  CALL_DEBUG5(cctx, "%s module cache was not found. func_name=%s", FUNC_NAME, cache_entry->config.func_name);
  return cache_entry->module;
}

wasmtime_module_t*
plwasm_wasm_module_create(
  	plwasm_call_context_t *cctx,
        wasm_byte_vec_t *wasm
) {

  wasmtime_error_t *error;
  wasmtime_module_t *module;
  wasi_config_t *wasi_config;

  module = NULL;
  error = wasmtime_module_new(
    cctx->ectx->engine,
    (uint8_t*) wasm->data,
    wasm->size,
    &module);
  if (error != NULL)
    CALL_WASM_ERROR(cctx, "failed to compile module", error, NULL);

  // Instantiate wasi
  wasi_config = wasi_config_new();
  if (wasi_config == NULL)
    CALL_ERROR(cctx, "failed to wasi_config_new");

  wasi_config_inherit_argv(wasi_config);
  wasi_config_inherit_env(wasi_config);
  wasi_config_inherit_stdin(wasi_config);
  wasi_config_inherit_stdout(wasi_config);
  wasi_config_inherit_stderr(wasi_config);

  error = wasmtime_context_set_wasi(cctx->rt.context, wasi_config);
  if (error != NULL)
    CALL_WASM_ERROR(cctx, "failed to instantiate WASI", error, NULL);
  
  return module;
}

void
plwasm_wasm_module_instantiate(
  	plwasm_call_context_t *cctx,
        wasmtime_module_t *module
) {
  wasmtime_error_t *error;
  wasm_trap_t *trap;

  CALL_DEBUG5(cctx, "Bind pg module.");
  plwasm_wasm_pglib_bind(cctx);

  CALL_DEBUG5(cctx, "Instantiating module.");
  trap = NULL;
  error = wasmtime_linker_instantiate(cctx->ectx->linker, cctx->rt.context, module, &(cctx->rt.instance), &trap); 
  if (error != NULL || trap != NULL)
    CALL_WASM_ERROR(cctx, "failed to instantiate", error, trap);
}

void plwasm_wasm_load_wasm(
  plwasm_call_context_t *cctx,
  wasm_byte_vec_t *wasm,
  char *proname
) {

  //char *FUNC_NAME = "plwasm_wasm_load_wasm";
  int source_len;
  wasmtime_error_t *error;
  wasm_byte_vec_t wat;

  if (cctx->func_config.wat != NULL) {
    source_len = strlen(cctx->func_config.wat);
    error = wasmtime_wat2wasm(cctx->func_config.wat, source_len, wasm);
    if (error != NULL)
      CALL_WASM_ERROR(cctx, "failed to parse wat", error, NULL);
    wasm_byte_vec_delete(&wat);
    return;
  }

  plwasm_wasm_load_file(cctx, wasm, proname, cctx->func_config.file);
}

void plwasm_wasm_load_file(
  plwasm_call_context_t *cctx,
  wasm_byte_vec_t *out,
  char *proname,
  char *source) {

  int source_len;
  FILE *fp;

  CALL_DEBUG5(cctx, "load file. name=%s", source);
  fp = fopen(source, "r");
  if (fp == NULL) {
    CALL_ERROR(cctx, "open failed. name=%s", source);
  }

  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    CALL_ERROR(cctx, "get file size failed. name=%s", source);
  }

  source_len = ftell(fp);
  CALL_DEBUG5(cctx, "file size = %d", source_len);

  if (fseek(fp, 0, SEEK_SET) != 0) {
    fclose(fp);
    CALL_ERROR(cctx, "rewind failed. name=%s", source);
  }

  wasm_byte_vec_new_uninitialized(out, source_len);
  fread(out->data, 1, source_len, fp);
  if (ferror(fp) != 0) {
    fclose(fp);
    CALL_ERROR(cctx, "fread failed. name=%s", source);
  }
  fclose(fp);
}
