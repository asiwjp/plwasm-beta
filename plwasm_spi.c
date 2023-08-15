#include "plwasm_spi.h"
#include "plwasm_log.h"
#include "plwasm_utils_pg.h"
#include "mb/pg_wchar.h"

#define COMMAND_CONTEXT_VEC_SZ 32

/*
 * Declare functions
 */

static void
spi_abort(
	plwasm_call_context_t *cctx,
	ResourceOwner oldowner
);

static plwasm_pg_command_context_t*
command_context_vec_create(
	int sz
);

static plwasm_pg_command_context_t*
command_context_vec_find_free(
	plwasm_call_context_t *cctx
);

static
plwasm_pg_command_context_t*
command_context_vec_get(
	plwasm_call_context_t *cctx,
	int cmd_id
);

static
bool
command_context_is_free(
	plwasm_pg_command_context_t *cmdctx
);

static void
cursor_context_init(
  plwasm_pg_command_context_t *cmdctx
);

static void
command_context_init(
  plwasm_pg_command_context_t *cmdctx
);

static void
check_resultset_ready(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx
);

static void
check_resultset_fetch_status(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx
);

static Form_pg_attribute
get_att_desc(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx,
	int att_idx
);



void
plwasm_spi_ready(
	plwasm_call_context_t *cctx
) {
	if (cctx->spi.connected) {
		CALL_DEBUG5(cctx, "SPI already connected.");
		return;
	}

	if (SPI_connect_ext(0) != SPI_OK_CONNECT) {
		CALL_ERROR(cctx, "SPI connect failed.");
	}

	cctx->spi.connected = true;
	cctx->spi.cmdctx_vec = NULL;
	cctx->spi.cmdctx_vec_sz = 0;
	CALL_DEBUG5(cctx, "SPI connected.");
}

void
plwasm_spi_finish(
	plwasm_call_context_t *cctx
) {
	plwasm_pg_command_context_t *cmd_it;
	plwasm_pg_command_context_t *cmd_end;
	CALL_DEBUG5(cctx, "SPI finish begin.");

	if (!cctx->spi.connected) {
		CALL_DEBUG5(cctx, "SPI is not connected or already finished. Skip SPI finishing.");
		return;
	}

	if (cctx->spi.cmdctx_vec != NULL) {
		CALL_DEBUG5(cctx, "SPI command context release bgin.");
		cmd_it = cctx->spi.cmdctx_vec;
		cmd_end = cmd_it + cctx->spi.cmdctx_vec_sz;
		for (; cmd_it < cmd_end; ++cmd_it) {
			if (command_context_is_free(cmd_it)) {
				continue;
			}

			plwasm_spi_command_close(cctx, cmd_it);	
		}
		CALL_DEBUG5(cctx, "SPI command context release end.");
		pfree(cctx->spi.cmdctx_vec);
		cctx->spi.cmdctx_vec_sz = 0;
		cctx->spi.cmdctx_vec = NULL;
		CALL_DEBUG5(cctx, "SPI command context vector release end.");
	}

	if (SPI_finish() != SPI_OK_FINISH) {
		CALL_ERROR(cctx, "SPI finish failed.");
	}

	cctx->spi.connected = false;
	CALL_DEBUG5(cctx, "SPI finished.");
}

void
plwasm_spi_internal_transaction_begin(
  plwasm_call_context_t *cctx
) {
	BeginInternalSubTransaction(NULL);
	MemoryContextSwitchTo(cctx->memctx_default);
	CALL_DEBUG5(cctx, "SPI sub-transaction begin.");
}

void
plwasm_spi_internal_transaction_commit(
  plwasm_call_context_t *cctx,
  ResourceOwner oldowner
) {
	ReleaseCurrentSubTransaction();
	MemoryContextSwitchTo(cctx->memctx_default);
	CurrentResourceOwner = oldowner;
	CALL_DEBUG5(cctx, "SPI sub-transaction committed.");
}

void
plwasm_spi_internal_transaction_abort(
  plwasm_call_context_t *cctx,
  ResourceOwner oldowner
) {
	RollbackAndReleaseCurrentSubTransaction();
	MemoryContextSwitchTo(cctx->memctx_default);
	CurrentResourceOwner = oldowner;
	CALL_DEBUG5(cctx, "SPI sub-transaction aborted.");
}

ErrorData*
plwasm_spi_err_capture(
  plwasm_call_context_t *cctx
) {
	ErrorData *edata;
	MemoryContextSwitchTo(cctx->memctx_default);
	edata = CopyErrorData();
	FlushErrorState();
	return edata;
}

plwasm_pg_command_context_t*
plwasm_spi_command_create(
  plwasm_call_context_t *cctx,
  char *command
) {
	plwasm_pg_command_context_t	*cmdctx;

	CALL_DEBUG5(cctx, "SPI create command. text=%s", command);

	pg_verifymbstr(command, strlen(command), false);

	if (cctx->spi.cmdctx_vec == NULL) {
        	cctx->spi.cmdctx_vec = command_context_vec_create(COMMAND_CONTEXT_VEC_SZ);
		cctx->spi.cmdctx_vec_sz = COMMAND_CONTEXT_VEC_SZ;
	}

	cmdctx = command_context_vec_find_free(cctx);
	cmdctx->command_text = command;

	CALL_DEBUG5(cctx, "SPI create command success. text=%s", command);
	return cmdctx;
}

plwasm_pg_command_context_t*
plwasm_spi_command_get_context(
  plwasm_call_context_t *cctx,
  int cmd_id
) {
	return command_context_vec_get(cctx, cmd_id);
}

void
plwasm_spi_command_prepare(
  plwasm_call_context_t *cctx,
  plwasm_pg_command_context_t *cmdctx
) {
	int				spi_rv;
	SPIPlanPtr			plan;
	bool				is_cursor_plan;

	CALL_DEBUG5(cctx, "SPI prepare command. text=%s", cmdctx->command_text);

	plan = SPI_prepare(cmdctx->command_text, 0, NULL);
	spi_rv = SPI_result;
	if (spi_rv < 0) {
        	CALL_ERROR(cctx, "SPI prepare failed. reason=%s",
			SPI_result_code_string(spi_rv));
        }
	is_cursor_plan = SPI_is_cursor_plan(plan);
	if (!is_cursor_plan) {
		CALL_ERROR(cctx, "command is not query.");
	}
	CALL_DEBUG5(cctx, "SPI prepare command success.");

        cmdctx->status = spi_rv;
        cmdctx->plan = plan;
        cmdctx->plan_is_query = is_cursor_plan;
	cursor_context_init(cmdctx);
}

uint64_t
plwasm_spi_command_execute(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx,
	int limit
) {
	int				spi_rv;

	CALL_DEBUG5(cctx, "SPI execute command. text=%s", cmdctx->command_text);

	if (cmdctx->cursor.portal != NULL) {
		CALL_ERROR(cctx, "cursor is active.");
	}

	spi_rv = SPI_execute_plan(cmdctx->plan, NULL, NULL, cmdctx->plan_is_query, limit);
	if (spi_rv < 0) {
        	CALL_ERROR(cctx, "SPI execute_plan failed. reason=%s",
			SPI_result_code_string(spi_rv));
        }

	cursor_context_init(cmdctx);
        cmdctx->status = spi_rv;

	if (cmdctx->plan_is_query) {
		cmdctx->cursor.portal = SPI_cursor_open(NULL, cmdctx->plan, NULL, NULL, true);
		cmdctx->processed = 0;
		CALL_DEBUG5(cctx, "SPI cursor opened. text=%s", cmdctx->command_text);
	} else {
        	cmdctx->cursor.portal = NULL;
		cmdctx->processed = SPI_processed;
	}
	return cmdctx->processed;
}

bool
plwasm_spi_command_close(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx
) {
	bool result = false;

	CALL_DEBUG5(cctx, "SPI close command. text=%s", cmdctx->command_text);

	if (plwasm_spi_resultset_is_opened(cctx, cmdctx)) {
		plwasm_spi_resultset_close(cctx, cmdctx);
	}

	if (cmdctx->plan != NULL) {
		SPI_freeplan(cmdctx->plan);
		cmdctx->plan = NULL;
		result = true;
	}

	command_context_init(cmdctx);
	return result;
}

bool
plwasm_spi_resultset_is_opened(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx
) {
	bool is_opened = (cmdctx->cursor.portal != NULL);
	CALL_DEBUG5(cctx, "SPI cursor is opened=%d. text=%s", is_opened, cmdctx->command_text);
	return is_opened;
}

bool
plwasm_spi_resultset_fetch(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx
) {
	CALL_DEBUG5(cctx, "SPI fetch. text=%s", cmdctx->command_text);

	check_resultset_ready(cctx, cmdctx);

	SPI_cursor_fetch(cmdctx->cursor.portal, true, 1);
	CALL_DEBUG5(cctx, "SPI_cursor_fetch processed=%ld", SPI_processed);
        cmdctx->tuptable = SPI_tuptable;
        cmdctx->processed = SPI_processed;
	if (cmdctx->processed <= 0) {
		plwasm_spi_resultset_close(cctx, cmdctx);
		return false;
	}
	cmdctx->cursor.pos++;
	return true;
}

bool
plwasm_spi_resultset_close(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx
) {
	CALL_DEBUG5(cctx, "SPI cursor close. text=%s", cmdctx->command_text);

	if (cmdctx->cursor.portal == NULL) {
		CALL_DEBUG5(cctx, "Skipped, because cursor is already closed.");
		return false;
	}

	SPI_cursor_close(cmdctx->cursor.portal);
	cursor_context_init(cmdctx);
	CALL_DEBUG5(cctx, "Closed.");
	return true;
}

int plwasm_spi_resultset_meta_get_att_count(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx
) {
	check_resultset_ready(cctx, cmdctx);
        return cmdctx->tuptable->tupdesc->natts;
}

Form_pg_attribute
plwasm_spi_resultset_meta_get_att_desc(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx,
	int att_idx
) {
	check_resultset_ready(cctx, cmdctx);
	return get_att_desc(cctx, cmdctx, att_idx);
}

Datum
plwasm_spi_resultset_get_val_as(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx,
	int att_idx,
	Oid desired_type,
	bool *is_null
) {
	const char	*FUNC_NAME = "plwasm_spi_resultset_get_val_as";
	Form_pg_attribute att;
        Datum val;

	CALL_DEBUG5(cctx, "%s begin. index=%d", FUNC_NAME, att_idx);

	check_resultset_ready(cctx, cmdctx);
	check_resultset_fetch_status(cctx, cmdctx);
	att = get_att_desc(cctx, cmdctx, att_idx);

	CALL_DEBUG5(cctx, "%s get value. index=%d", FUNC_NAME, att_idx);
        val = heap_getattr(
		//cmdctx->tuptable->vals[cmdctx->cursor.pos],
		cmdctx->tuptable->vals[0],
		att_idx + 1,
		cmdctx->tuptable->tupdesc,
		is_null);
        if (*is_null) {
		CALL_DEBUG5(cctx, "%s attribute is null. index=%d", FUNC_NAME, att_idx);
		return (Datum)0;
	}

        if (att->atttypid == desired_type) {
		CALL_DEBUG5(cctx, "%s success. index=%d", FUNC_NAME, att_idx);
		return val;
	}

	CALL_DEBUG5(cctx,
		"%s attribute type to cast. index=%d, type=%u, desired_type=%u",
		FUNC_NAME, att_idx, att->atttypid, desired_type);
	val = plwasm_utils_pg_type_cast(
		cctx, val, att->atttypid, desired_type);
	CALL_DEBUG5(cctx, "%s success. index=%d", FUNC_NAME, att_idx);
	return val;
}

Datum
plwasm_spi_query_scalar_as(
	plwasm_call_context_t *cctx,
	char	*cmd_txt,
	Oid	desired_type,
	bool	*is_null
) {
  volatile MemoryContext oldctx;
  volatile ResourceOwner oldowner;
  plwasm_pg_command_context_t *cmdctx;
  Datum val;

  oldowner = CurrentResourceOwner;
  oldctx = plwasm_utils_pg_memctx_switch_to_proc_context(cctx);
  plwasm_spi_ready(cctx);
  PG_TRY();
  {
    plwasm_spi_internal_transaction_begin(cctx);
    cmdctx = plwasm_spi_command_create(cctx, cmd_txt);
    plwasm_spi_command_prepare(cctx, cmdctx);
    plwasm_spi_command_execute(cctx, cmdctx, 1);
    if (!plwasm_spi_resultset_fetch(cctx, cmdctx)) {
      CALL_ERROR(cctx, "no result set");
    }
    val = plwasm_spi_resultset_get_val_as(
	cctx, cmdctx, 0, desired_type, is_null);
    plwasm_spi_internal_transaction_commit(cctx, oldowner);
  }
  PG_CATCH();
  {
	spi_abort(cctx, oldowner);
  }
  PG_END_TRY();
  return val;
}

static void
spi_abort(
	plwasm_call_context_t *cctx,
	ResourceOwner oldowner
) {
    plwasm_spi_internal_transaction_abort(cctx, oldowner);
    PG_RE_THROW();
}


static plwasm_pg_command_context_t*
command_context_vec_create(
	int sz
) {
	plwasm_pg_command_context_t *cmdctx_vec;
	plwasm_pg_command_context_t *cmdctx;
	cmdctx_vec = (plwasm_pg_command_context_t*) palloc(
		sizeof(plwasm_pg_command_context_t) * sz);
	cmdctx = cmdctx_vec;
	for (int i = 0; i < sz; ++i) {
		cmdctx->id = i;
		command_context_init(cmdctx);
		++cmdctx;
	}
	return cmdctx_vec;
}

static plwasm_pg_command_context_t*
command_context_vec_find_free(
	plwasm_call_context_t *cctx
) {
	plwasm_pg_command_context_t* it = cctx->spi.cmdctx_vec;
	for (int i = 0; i < cctx->spi.cmdctx_vec_sz; ++i) {
		if (command_context_is_free(it)) {
			return it;
		}
		++it;
	}
	CALL_ERROR(cctx, "The number of active commands has reached ther upper limit.");
}

static
plwasm_pg_command_context_t*
command_context_vec_get(
	plwasm_call_context_t *cctx,
	int cmd_id
) {
	plwasm_pg_command_context_t *cmdctx;

	if (cctx->spi.cmdctx_vec == NULL) {
		CALL_ERROR(cctx, "Command was not created.");
	}

	if (cmd_id < 0 || cmd_id >= cctx->spi.cmdctx_vec_sz) {
		CALL_ERROR(cctx, "Invalid command id. id=%d", cmd_id);
	}
	
	cmdctx = &(cctx->spi.cmdctx_vec[cmd_id]);
	if (cmdctx->command_text == NULL) {
		CALL_ERROR(cctx, "Invalid command index. Command was not created. id=%d", cmd_id);
	}
	return cmdctx;
}

static
bool
command_context_is_free(
	plwasm_pg_command_context_t *cmdctx
) {
  return cmdctx->command_text == NULL;
}

static void
command_context_init(
  plwasm_pg_command_context_t *cmdctx
) {
        cmdctx->status = 0;
        cmdctx->command_text = NULL;
        cmdctx->plan = NULL;
        cmdctx->plan_is_query = false;
        cmdctx->tuptable = NULL;
        cmdctx->processed = -1;
        cursor_context_init(cmdctx);
}

static void
cursor_context_init(
  plwasm_pg_command_context_t *cmdctx
) {
	cmdctx->cursor.portal = NULL;
        cmdctx->cursor.pos = -1;
        cmdctx->cursor.is_null = false;
}

static void
check_resultset_ready(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx
) {
	if (cmdctx->status <= 0) {
		CALL_ERROR(cctx, "command was not succeeded.");
	}

	if (cmdctx->cursor.portal == NULL) {
		CALL_ERROR(cctx, "cursor was not opened.");
	}
}

static void
check_resultset_fetch_status(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx
) {
	if (cmdctx->tuptable == NULL) {
		CALL_ERROR(cctx, "no results");
	}

        if (cmdctx->cursor.pos < 0) {
		CALL_ERROR(cctx, "invalid current position.");
        }
}

static Form_pg_attribute
get_att_desc(
	plwasm_call_context_t *cctx,
	plwasm_pg_command_context_t *cmdctx,
	int att_idx
) {
	const char *FUNC_NAME = "get_att_desc";
	Form_pg_attribute att;

	CALL_DEBUG5(cctx, "%s begin. index=%d", FUNC_NAME, att_idx);
        if (att_idx < 0 || att_idx >= cmdctx->tuptable->tupdesc->natts) {
		CALL_ERROR(cctx, "attribute index is out of range.");
        }

	att = TupleDescAttr(cmdctx->tuptable->tupdesc, att_idx);
        if (att->attisdropped) {
		CALL_ERROR(cctx, "attribute was dropped.");
        }
	return att;
}
