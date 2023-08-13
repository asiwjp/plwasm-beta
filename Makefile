MODULE_big = plwasm

EXTENSION = plwasm
DATA = plwasm--0.1.sql
OBJS = \
  plwasm_wasm.o \
  plwasm_wasm_engine.o \
  plwasm_wasm_memory.o \
  plwasm_wasm_module.o \
  plwasm_wasm_pglib_core.o \
  plwasm_wasm_pglib_command.o \
  plwasm_wasm_pglib_resultset.o \
  plwasm_wasm_pglib.o \
  plwasm_wasm_utils.o \
  plwasm_log.o \
  plwasm_spi.o \
  plwasm_utils_json.o \
  plwasm_utils_pg.o \
  plwasm_utils_str.o \
  plwasm_func_body.o \
  plwasm_handler.o
PGFILEDESC = "PL/wasm"

WASMRTM_HOME:=$(realpath $(WASMTIME_HOME))
WASMAPI_HOME:=$(WASMRTM_HOME)/c-api

PG_CFLAGS = -Wall -I$(WASMAPI_HOME)/include
PG_LDFLAGS= -L $(WASMAPI_HOME)/lib -lpthread -ldl -lm -lwasmtime

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

