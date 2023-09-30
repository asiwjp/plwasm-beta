#include "plwasm_wasm_module.h"
#include "plwasm_func_body.h"
#include "plwasm_log.h"
#include "plwasm_wasm_pglib.h"

#include <fcntl.h>
#include <sys/stat.h>

#define ENTRY_POINT_FUNC_NAME_REACTOR "_initialize"
#define ENTRY_POINT_FUNC_NAME_START "_start"

char*
plwasm_wasm_find_entry_point(
	plwasm_call_context_t *cctx,
	wasmtime_extern_t *run,
	wasmtime_instance_t *instance
);

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
  plwasm_hs_entry_cache_wasm_module_t *cache_entry;
  wasm_byte_vec_t wasm;
  //volatile MemoryContext memctx;
  volatile MemoryContext old_memctx;

  cache_entry = (plwasm_hs_entry_cache_wasm_module_t*) hash_search(
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
    cctx->func_config.cache.instance = cache_entry->config.cache.instance;
    cctx->func_config.trace = cache_entry->config.trace;
    cctx->func_config.timing = cache_entry->config.timing;
    plwasm_log_stopwatch_save(cctx, cctx->times.loaded);
    return cache_entry->module;
  }

  CALL_DEBUG5(cctx, "%s module cache was not found. fn_oid=%d", FUNC_NAME, fn_oid);
  plwasm_func_body_describe(cctx, &(cctx->pg_proc), fn_oid);
  plwasm_func_body_parse(cctx, &(cctx->pg_proc));
  plwasm_wasm_load_wasm(cctx, &wasm, cctx->pg_proc.name);
  cache_entry->module = plwasm_wasm_module_new(cctx, &wasm);
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
  cache_entry->config.cache.instance = cctx->func_config.cache.instance;
  cache_entry->config.trace = cctx->func_config.trace;
  cache_entry->config.timing = cctx->func_config.timing;
  MemoryContextSwitchTo(old_memctx);
  plwasm_log_stopwatch_save(cctx, cctx->times.loaded);
  CALL_DEBUG5(cctx, "%s module cache was not found. func_name=%s", FUNC_NAME, cache_entry->config.func_name);
  return cache_entry->module;
}

wasmtime_module_t*
plwasm_wasm_module_new(
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

  error = wasmtime_context_set_wasi(cctx->ectx->rt.context, wasi_config);
  if (error != NULL)
    CALL_WASM_ERROR(cctx, "failed to instantiate WASI", error, NULL);

  return module;
}

wasmtime_instance_t*
plwasm_wasm_module_instantiate_with_cache(
  	plwasm_call_context_t *cctx,
	Oid fn_oid,
        wasmtime_module_t *module
) {
  char *FUNC_NAME = "plwasm_wasm_module_instantiate_with_cache";
  wasmtime_instance_t *instance;
  plwasm_hs_entry_cache_wasm_instance_t *cache_entry;
  bool found;

  if (!cctx->func_config.cache.instance.enabled) {
    CALL_DEBUG5(cctx, "%s instance cache was disabled. func_name=%s",
	FUNC_NAME, cctx->func_config.func_name);
    instance = palloc(sizeof(wasmtime_instance_t));
    plwasm_wasm_module_instantiate(cctx, instance, module);
    return instance;
  }

  cache_entry = (plwasm_hs_entry_cache_wasm_instance_t*) hash_search(
    cctx->ectx->wasm_instance_cache,
    &fn_oid,
    HASH_ENTER,
    &found);
  if (found) {
    CALL_DEBUG5(cctx, "%s instance cache was found. func_name=%s",
	FUNC_NAME, cctx->func_config.func_name);
    plwasm_log_stopwatch_save(cctx, cctx->times.instantiated);
    cctx->times.entry_point_find_ended = cctx->times.instantiated;
    cctx->times.entry_point_invoked = cctx->times.instantiated;
    return &(cache_entry->instance);
  }

  CALL_DEBUG5(cctx, "%s instance cache was not found. fn_oid=%d",
	FUNC_NAME, fn_oid);
  plwasm_wasm_module_instantiate(cctx, &(cache_entry->instance), module);
  return &(cache_entry->instance);
}

void
plwasm_wasm_module_instantiate(
  	plwasm_call_context_t *cctx,
	wasmtime_instance_t *instance,
        wasmtime_module_t *module
) {
  wasmtime_error_t *error;
  wasm_trap_t *trap = NULL;
  wasmtime_extern_t run;
  
  CALL_DEBUG5(cctx, "Bind pg module.");
  plwasm_wasm_pglib_bind(cctx);

  CALL_DEBUG5(cctx, "Instantiating module.");
  error = wasmtime_linker_instantiate(
	cctx->ectx->rt.linker,
	cctx->ectx->rt.context,
	module,
	instance,
	&trap); 
  if (error != NULL || trap != NULL)
    CALL_WASM_ERROR(cctx, "failed to instantiate", error, trap);
  plwasm_log_stopwatch_save(cctx, cctx->times.instantiated);

  CALL_DEBUG5(cctx, "Calling entry point function.");
  cctx->entry_point_name = plwasm_wasm_find_entry_point(cctx, &run, instance);
  plwasm_log_stopwatch_save(cctx, cctx->times.entry_point_find_ended);
  if (cctx->entry_point_name == NULL) {
    plwasm_log_stopwatch_save(cctx, cctx->times.entry_point_find_ended);
    cctx->times.entry_point_invoked = cctx->times.entry_point_find_ended;
    CALL_DEBUG5(cctx, "Entry point was not found.");
    return;
  }

  CALL_DEBUG5(cctx, "Entry point is %s", cctx->entry_point_name);
  trap = NULL;
  error = wasmtime_func_call(cctx->ectx->rt.context, &run.of.func, NULL, 0, NULL, 0, &trap);
  plwasm_log_stopwatch_save(cctx, cctx->times.entry_point_invoked);
  if (error != NULL || trap != NULL) {
      CALL_WASM_ERROR(cctx, "failed to call entry point function", error, trap);
  }

  CALL_DEBUG5(cctx, "%s called.", cctx->entry_point_name);
}

void
plwasm_wasm_module_extra_init(
  plwasm_call_context_t *cctx
) {
  CALL_DEBUG5(cctx, "Init extra modules.");
  plwasm_wasm_pglib_init(cctx);
}

void
plwasm_wasm_module_extra_release(
  plwasm_call_context_t *cctx
) {
  if (cctx->instance == NULL) {
    CALL_DEBUG5(cctx, "Release extra modules. skipped.");
    return;
  }

  CALL_DEBUG5(cctx, "Release extra modules.");
  plwasm_wasm_pglib_release(cctx);
}

char*
plwasm_wasm_find_entry_point(
	plwasm_call_context_t *cctx,
	wasmtime_extern_t *run,
	wasmtime_instance_t *instance
) {
  bool found;
  char *entry_point_name;

  entry_point_name = ENTRY_POINT_FUNC_NAME_REACTOR;
  found = wasmtime_instance_export_get(
    cctx->ectx->rt.context,
    instance,
    entry_point_name,
    strlen(entry_point_name),
    run);

  if (found) {
    return entry_point_name;
  }

  entry_point_name = ENTRY_POINT_FUNC_NAME_START;
  found = wasmtime_instance_export_get(
    cctx->ectx->rt.context,
    instance,
    entry_point_name,
    strlen(entry_point_name),
    run);
  
  if (found) {
    return entry_point_name;
  }

  return NULL;
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
  char *source)
{
  int fd;
  int save_errno;
  struct stat statbuf;
  long source_len;
  long readed = 0;
  char *outbuf;

  CALL_DEBUG5(cctx, "load file. name=%s", source);
  fd = open(source, O_RDONLY);
  if (fd == -1) {
    save_errno = errno;
    CALL_ERROR(cctx, "open failed. file=%s, reason=%s", source, strerror(save_errno));
  }

  if (fstat(fd, &statbuf) == -1) {
    save_errno = errno;
    close(fd);
    CALL_ERROR(cctx, "fstat failed. file=%s, reason=%s", source, strerror(save_errno));
  }

  source_len = statbuf.st_size;
  CALL_DEBUG5(cctx, "file size = %ld", source_len);

  wasm_byte_vec_new_uninitialized(out, source_len);
  outbuf = out->data;
  do {
    readed = read(fd, outbuf, source_len);
    save_errno = errno;
    CALL_DEBUG5(cctx, "readed = %ld", readed);

    source_len -= readed;
    if (source_len == 0) {
      close(fd);
      break;
    }

    if (readed == -1) {
      close(fd);
      CALL_ERROR(cctx, "read failed. file=%s, reason=%s", source, strerror(save_errno));
    }

    outbuf += readed;
  } while (true);
}

