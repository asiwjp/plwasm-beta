#ifndef H_PLWASM_TYPES
#define H_PLWASM_TYPES

#include "postgres.h"
#include "catalog/pg_proc.h"
#include "catalog/pg_type.h"
#include "commands/event_trigger.h"
#include "commands/trigger.h"
#include "executor/spi.h"
#include "funcapi.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include "utils/syscache.h"

#ifdef _MSC_VER
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <wasm.h>
#include <wasmtime.h>

typedef struct plwasm_pg_cursor_context {
	Portal		portal;
	uint64_t	pos;
	bool		is_null;
} plwasm_pg_cursor_context_t;

typedef struct plwasm_pg_statement_context {
	int8_t				id;
	int				status;
	char				*statement_text;
	SPIPlanPtr			plan;
	bool				plan_is_query;
	SPITupleTable			*tuptable;
	int				processed;
	plwasm_pg_cursor_context_t	cursor;
} plwasm_pg_statement_context_t;

typedef struct plwasm_spi_context {
	bool				connected;
	plwasm_pg_statement_context_t	*stmctx_vec;
	int				stmctx_vec_sz;
} plwasm_spi_context_t;

typedef struct plwasm_pg_proc {
  char *name;
  char *source;
  int  source_len;
  Oid  ret_type;
  char *entry_point_name;
} plwasm_pg_proc_t;

typedef struct plwasm_func_cache_config {
  bool enabled;
} plwasm_func_cache_config_t;

typedef struct plwasm_func_cache_configs {
  plwasm_func_cache_config_t instance;
} plwasm_func_cache_configs_t;

typedef struct plwasm_func_config {
  char *file;
  char *func_name;
  char *wat;
  char *string_enc_name;
  int   string_enc;
  bool  string_enc_required;
  bool trace;
  int  trace_threshold;
  bool timing;
  plwasm_func_cache_configs_t cache;
} plwasm_func_config_t;

typedef struct plwasm_extension_config {
  bool trace;
  int  trace_threshold;
  bool timing;
  plwasm_func_cache_configs_t cache;
} plwasm_extension_config_t;

typedef struct plwasm_hs_entry_cache_wasm_instance {
  Oid key;
  wasmtime_instance_t instance;
} plwasm_hs_entry_cache_wasm_instance_t;

typedef struct plwasm_hs_entry_cache_wasm_module {
  Oid key;
  MemoryContext memctx;
  wasmtime_module_t *module;
  plwasm_pg_proc_t pg_proc;
  plwasm_func_config_t config;
} plwasm_hs_entry_cache_wasm_module_t;

typedef struct plwasm_wasm_modules {
  wasmtime_module_t *pg;
} plwasm_wasm_modules_t;


typedef struct plwasm_wasm_runtime {
  wasmtime_store_t		*store;
  wasmtime_linker_t		*linker;
  wasmtime_context_t		*context;
} plwasm_wasm_runtime_t;

typedef struct plwasm_wasm_ret {
  Oid type;
  union {
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;
    struct {
      char *ptr;
      int size;
    } cstring;
    text *text;
    bytea *bytea;
  } of;
} plwasm_wasm_ret_t;

typedef struct plwasm_times {
  struct timespec begin;
  struct timespec initted;
  struct timespec loaded;
  struct timespec instantiated;
  struct timespec entry_point_find_ended;
  struct timespec entry_point_invoked;
  struct timespec func_find_ended;
  struct timespec func_invoked;
  struct timespec invoked;
  struct timespec ended;
} plwasm_times_t;

typedef struct plwasm_extension_context {
  int				type;
  volatile MemoryContext	memctx;
  plwasm_extension_config_t	config;
  wasm_engine_t			*engine;
  plwasm_wasm_runtime_t		rt;
  plwasm_wasm_modules_t		modules;
  HTAB				*wasm_module_cache;
  HTAB				*wasm_instance_cache;
} plwasm_extension_context_t;

typedef struct plwasm_call_context {
  int				type;
  volatile MemoryContext	memctx_default;

  plwasm_extension_context_t	*ectx;
  plwasm_pg_proc_t		pg_proc;
  wasmtime_module_t		*module;
  wasmtime_instance_t           *instance;
  plwasm_func_config_t		func_config;
  plwasm_wasm_ret_t		ret;
  PG_FUNCTION_ARGS;
  char*				entry_point_name;
  plwasm_times_t		times;
  plwasm_spi_context_t		spi;
} plwasm_call_context_t;

enum plwasm_context_types {
	plwasm_CTX_TYPE_EXTENSION,
	plwasm_CTX_TYPE_CALL
};
#endif
