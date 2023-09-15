#include "plwasm_wasm_engine.h"
#include "plwasm_wasm_pglib.h"
#include "plwasm_log.h"

void plwasm_wasm_engine_new(plwasm_extension_context_t *ectx) {
  wasmtime_error_t *error;
  HASHCTL hsctl_modules;
  HASHCTL hsctl_instances;

  ectx->engine = wasm_engine_new();
  if (ectx->engine == NULL)
    elog(ERROR, "wasm_engine_new failed.");

  ectx->rt.linker = wasmtime_linker_new(ectx->engine);
  if (ectx->rt.linker == NULL)
    elog(ERROR, "wasm_linker_new failed.");

  error = wasmtime_linker_define_wasi(ectx->rt.linker);
  if (error != NULL)
    elog(ERROR, "wasm_linker_define_wasi failed.");

  ectx->rt.store = wasmtime_store_new(ectx->engine, NULL, NULL);
  if (ectx->rt.store == NULL)
    EXT_WASM_ERROR(ectx, "failed to create store", NULL, NULL);

  ectx->rt.context = wasmtime_store_context(ectx->rt.store);
  if (ectx->rt.context == NULL)
    EXT_WASM_ERROR(ectx, "failed to create context", NULL, NULL);

  ectx->memctx = AllocSetContextCreate(CacheMemoryContext,
		 "PL/WebAssembly cache context",
		 ALLOCSET_SMALL_SIZES);

  // create module cache
  hsctl_modules.hcxt = ectx->memctx;
  hsctl_modules.keysize = sizeof(Oid);
  hsctl_modules.entrysize = sizeof(plwasm_hs_entry_cache_wasm_module_t);
  ectx->wasm_module_cache = hash_create(
	"plwasm-modules",
	1024,
	&hsctl_modules,
	HASH_ELEM | HASH_BLOBS | HASH_CONTEXT);

  // create instance cache
  hsctl_instances.hcxt = ectx->memctx;
  hsctl_instances.keysize = sizeof(Oid);
  hsctl_instances.entrysize = sizeof(plwasm_hs_entry_cache_wasm_instance_t);
  ectx->wasm_instance_cache = hash_create(
	"plwasm-instances",
	1024,
	&hsctl_instances,
	HASH_ELEM | HASH_BLOBS | HASH_CONTEXT);

  // load shared modules
  plwasm_wasm_pglib_load(ectx);

  ereport(DEBUG1, (errmsg("wasm engine created.")));
}

void plwasm_wasm_engine_delete(plwasm_extension_context_t *ectx) {
  hash_destroy(ectx->wasm_module_cache);

  if (ectx->engine != NULL) {
    wasm_engine_delete(ectx->engine);
    ectx->engine = NULL;
  }
}

