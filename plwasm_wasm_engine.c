#include "plwasm_wasm_engine.h"
#include "plwasm_wasm_pglib.h"
#include "plwasm_log.h"

void plwasm_wasm_engine_new(plwasm_extension_context_t *ectx) {
  wasmtime_error_t *error;
  HASHCTL hash_ctl;

  ectx->type = plwasm_CTX_TYPE_EXTENSION;
  ectx->engine = wasm_engine_new();
  if (ectx->engine == NULL)
    elog(ERROR, "wasm_engine_new failed.");

  ectx->linker = wasmtime_linker_new(ectx->engine);
  if (ectx->linker == NULL)
    elog(ERROR, "wasm_linker_new failed.");

  error = wasmtime_linker_define_wasi(ectx->linker);
  if (error != NULL)
    elog(ERROR, "wasm_linker_define_wasi failed.");
    //elog_wasm(cctx, "wasm_linker_define_wasi failed.", error, NULL);
  
  ectx->memctx = AllocSetContextCreate(PostmasterContext,
		 "PL/WebAssembly cache context",
		 ALLOCSET_SMALL_SIZES);
  hash_ctl.keysize = sizeof(Oid);
  hash_ctl.entrysize = sizeof(plwasm_wasm_module_cache_entry_t);
  hash_ctl.hcxt = ectx->memctx;
  ectx->wasm_module_cache = hash_create("plwasm-modules", 1024, &hash_ctl, HASH_ELEM | HASH_BLOBS | HASH_CONTEXT);

  ectx->modules.pg = NULL;
  plwasm_wasm_pglib_init(ectx);

  ereport(DEBUG1, (errmsg("wasm engine created.")));
}

void plwasm_wasm_engine_delete(plwasm_extension_context_t *ectx) {
  hash_destroy(ectx->wasm_module_cache);

  if (ectx->engine != NULL) {
    wasm_engine_delete(ectx->engine);
    ectx->engine = NULL;
  }
}

