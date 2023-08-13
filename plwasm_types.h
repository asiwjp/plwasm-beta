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

#include <sys/time.h>
#include <wasm.h>
#include <wasmtime.h>


typedef struct plwasm_pg_cursor_context {
	Portal		portal;
	uint64_t	pos;
	bool		is_null;
} plwasm_pg_cursor_context_t;

typedef struct plwasm_pg_command_context {
	int8_t				id;
	int				status;
	char				*command_text;
	SPIPlanPtr			plan;
	bool				plan_is_query;
	SPITupleTable			*tuptable;
	int				processed;
	plwasm_pg_cursor_context_t	cursor;
} plwasm_pg_command_context_t;

typedef struct plwasm_spi_context {
	bool				connected;
	plwasm_pg_command_context_t	*cmdctx_vec;
	int				cmdctx_vec_sz;
} plwasm_spi_context_t;

typedef struct plwasm_pg_proc {
  char *name;
  char *source;
  int  source_len;
  Oid  ret_type;
  char *entry_point_name;
} plwasm_pg_proc_t;

typedef struct plwasm_func_config {
  char *file;
  char *func_name;
  char *wat;
  char *string_enc_name;
  int   string_enc;
  bool  string_enc_required;
  bool trace;
  bool stats;
} plwasm_func_config_t;

typedef struct plwasm_wasm_module_cache_entry {
  Oid key;
  MemoryContext memctx;
  wasmtime_module_t *module;
  plwasm_pg_proc_t pg_proc;
  plwasm_func_config_t config;
} plwasm_wasm_module_cache_entry_t;

typedef struct plwasm_wasm_modules {
  wasmtime_module_t *pg;
} plwasm_wasm_modules_t;


typedef struct plwasm_wasm_runtime {
  wasmtime_store_t		*store;
  wasmtime_context_t		*context;
  wasmtime_instance_t		instance;
} plwasm_wasm_runtime_t;

typedef struct plwasm_wasm_ret {
  Oid type;
  union {
    int32_t i32;
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
  volatile MemoryContext			memctx;
  wasm_engine_t			*engine;
  wasmtime_linker_t		*linker;
  plwasm_wasm_modules_t	 modules;
  HTAB				*wasm_module_cache;
} plwasm_extension_context_t;

typedef struct plwasm_call_context {
  int				type;
  volatile MemoryContext	memctx_default;

  plwasm_extension_context_t	*ectx;
  plwasm_pg_proc_t		pg_proc;
  plwasm_wasm_runtime_t	rt;
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
