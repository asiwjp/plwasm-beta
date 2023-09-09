#include "plwasm_spi.h"
#include "plwasm_log.h"
#include "plwasm_utils_pg.h"
#include "mb/pg_wchar.h"

#define STATEMENT_CONTEXT_VEC_SZ 32

/*
 * Declare functions
 */

static void
spi_abort(
	plwasm_call_context_t *cctx,
	ResourceOwner oldowner
);

static plwasm_pg_statement_context_t*
statement_context_vec_new(
	int sz
);

static plwasm_pg_statement_context_t*
statement_context_vec_find_free(
	plwasm_call_context_t *cctx
);

static
plwasm_pg_statement_context_t*
statement_context_vec_get(
	plwasm_call_context_t *cctx,
	int stmt_id
);

static
bool
statement_context_is_free(
	plwasm_pg_statement_context_t *stmctx
);

static void
cursor_context_init(
  plwasm_pg_statement_context_t *stmctx
);

static void
statement_context_init(
  plwasm_pg_statement_context_t *stmctx
);

static void
check_resultset_ready(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx
);

static void
check_resultset_fetch_status(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx
);

static Form_pg_attribute
get_att_desc(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx,
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
	cctx->spi.stmctx_vec = NULL;
	cctx->spi.stmctx_vec_sz = 0;
	CALL_DEBUG5(cctx, "SPI connected.");
}

void
plwasm_spi_finish(
	plwasm_call_context_t *cctx
) {
	plwasm_pg_statement_context_t *stmt_it;
	plwasm_pg_statement_context_t *stmt_end;
	CALL_DEBUG5(cctx, "SPI finish begin.");

	if (!cctx->spi.connected) {
		CALL_DEBUG5(cctx, "SPI is not connected or already finished. Skip SPI finishing.");
		return;
	}

	if (cctx->spi.stmctx_vec != NULL) {
		CALL_DEBUG5(cctx, "SPI statement context release bgin.");
		stmt_it = cctx->spi.stmctx_vec;
		stmt_end = stmt_it + cctx->spi.stmctx_vec_sz;
		for (; stmt_it < stmt_end; ++stmt_it) {
			if (statement_context_is_free(stmt_it)) {
				continue;
			}

			plwasm_spi_statement_close(cctx, stmt_it);	
		}
		CALL_DEBUG5(cctx, "SPI statement context release end.");
		pfree(cctx->spi.stmctx_vec);
		cctx->spi.stmctx_vec_sz = 0;
		cctx->spi.stmctx_vec = NULL;
		CALL_DEBUG5(cctx, "SPI statement context vector release end.");
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

plwasm_pg_statement_context_t*
plwasm_spi_statement_new(
  plwasm_call_context_t *cctx,
  char *statement
) {
	plwasm_pg_statement_context_t	*stmctx;

	CALL_DEBUG5(cctx, "SPI create statement. text=%s", statement);

	pg_verifymbstr(statement, strlen(statement), false);

	if (cctx->spi.stmctx_vec == NULL) {
        	cctx->spi.stmctx_vec = statement_context_vec_new(STATEMENT_CONTEXT_VEC_SZ);
		cctx->spi.stmctx_vec_sz = STATEMENT_CONTEXT_VEC_SZ;
	}

	stmctx = statement_context_vec_find_free(cctx);
	stmctx->statement_text = statement;

	CALL_DEBUG5(cctx, "SPI create statement success. text=%s", statement);
	return stmctx;
}

plwasm_pg_statement_context_t*
plwasm_spi_statement_get_context(
  plwasm_call_context_t *cctx,
  int stmt_id
) {
	return statement_context_vec_get(cctx, stmt_id);
}

void
plwasm_spi_statement_prepare(
  plwasm_call_context_t *cctx,
  plwasm_pg_statement_context_t *stmctx
) {
	int				spi_rv;
	SPIPlanPtr			plan;
	bool				is_cursor_plan;

	CALL_DEBUG5(cctx, "SPI prepare statement. text=%s", stmctx->statement_text);

	plan = SPI_prepare(stmctx->statement_text, 0, NULL);
	spi_rv = SPI_result;
	if (spi_rv < 0) {
        	CALL_ERROR(cctx, "SPI prepare failed. reason=%s",
			SPI_result_code_string(spi_rv));
        }
	is_cursor_plan = SPI_is_cursor_plan(plan);
	if (!is_cursor_plan) {
		CALL_ERROR(cctx, "statement is not query.");
	}
	CALL_DEBUG5(cctx, "SPI prepare statement success.");

        stmctx->status = spi_rv;
        stmctx->plan = plan;
        stmctx->plan_is_query = is_cursor_plan;
	cursor_context_init(stmctx);
}

uint64_t
plwasm_spi_statement_execute(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx,
	int limit
) {
	int				spi_rv;

	CALL_DEBUG5(cctx, "SPI execute statement. text=%s", stmctx->statement_text);

	if (stmctx->cursor.portal != NULL) {
		CALL_ERROR(cctx, "cursor is active.");
	}

	spi_rv = SPI_execute_plan(stmctx->plan, NULL, NULL, stmctx->plan_is_query, limit);
	if (spi_rv < 0) {
        	CALL_ERROR(cctx, "SPI execute_plan failed. reason=%s",
			SPI_result_code_string(spi_rv));
        }

	cursor_context_init(stmctx);
        stmctx->status = spi_rv;

	if (stmctx->plan_is_query) {
		stmctx->cursor.portal = SPI_cursor_open(NULL, stmctx->plan, NULL, NULL, true);
		stmctx->processed = 0;
		CALL_DEBUG5(cctx, "SPI cursor opened. text=%s", stmctx->statement_text);
	} else {
        	stmctx->cursor.portal = NULL;
		stmctx->processed = SPI_processed;
	}
	return stmctx->processed;
}

bool
plwasm_spi_statement_close(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx
) {
	bool result = false;

	CALL_DEBUG5(cctx, "SPI close statement. text=%s", stmctx->statement_text);

	if (plwasm_spi_resultset_is_opened(cctx, stmctx)) {
		plwasm_spi_resultset_close(cctx, stmctx);
	}

	if (stmctx->plan != NULL) {
		SPI_freeplan(stmctx->plan);
		stmctx->plan = NULL;
		result = true;
	}

	statement_context_init(stmctx);
	return result;
}

bool
plwasm_spi_resultset_is_opened(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx
) {
	bool is_opened = (stmctx->cursor.portal != NULL);
	CALL_DEBUG5(cctx, "SPI cursor is opened=%d. text=%s", is_opened, stmctx->statement_text);
	return is_opened;
}

bool
plwasm_spi_resultset_fetch(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx
) {
	CALL_DEBUG5(cctx, "SPI fetch. text=%s", stmctx->statement_text);

	check_resultset_ready(cctx, stmctx);

	SPI_cursor_fetch(stmctx->cursor.portal, true, 1);
	CALL_DEBUG5(cctx, "SPI_cursor_fetch processed=%ld", SPI_processed);
        stmctx->tuptable = SPI_tuptable;
        stmctx->processed = SPI_processed;
	if (stmctx->processed <= 0) {
		plwasm_spi_resultset_close(cctx, stmctx);
		return false;
	}
	stmctx->cursor.pos++;
	return true;
}

bool
plwasm_spi_resultset_close(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx
) {
	CALL_DEBUG5(cctx, "SPI cursor close. text=%s", stmctx->statement_text);

	if (stmctx->cursor.portal == NULL) {
		CALL_DEBUG5(cctx, "Skipped, because cursor is already closed.");
		return false;
	}

	SPI_cursor_close(stmctx->cursor.portal);
	cursor_context_init(stmctx);
	CALL_DEBUG5(cctx, "Closed.");
	return true;
}

int plwasm_spi_resultset_meta_get_att_count(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx
) {
	check_resultset_ready(cctx, stmctx);
        return stmctx->tuptable->tupdesc->natts;
}

Form_pg_attribute
plwasm_spi_resultset_meta_get_att_desc(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx,
	int att_idx
) {
	check_resultset_ready(cctx, stmctx);
	return get_att_desc(cctx, stmctx, att_idx);
}

Datum
plwasm_spi_resultset_get_val_as(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx,
	int att_idx,
	Oid desired_type,
	bool *is_null
) {
	const char	*FUNC_NAME = "plwasm_spi_resultset_get_val_as";
	Form_pg_attribute att;
        Datum val;

	CALL_DEBUG5(cctx, "%s begin. index=%d", FUNC_NAME, att_idx);

	check_resultset_ready(cctx, stmctx);
	check_resultset_fetch_status(cctx, stmctx);
	att = get_att_desc(cctx, stmctx, att_idx);

	CALL_DEBUG5(cctx, "%s get value. index=%d", FUNC_NAME, att_idx);
        val = heap_getattr(
		//stmctx->tuptable->vals[stmctx->cursor.pos],
		stmctx->tuptable->vals[0],
		att_idx + 1,
		stmctx->tuptable->tupdesc,
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
	char	*stmt_txt,
	Oid	desired_type,
	bool	*is_null
) {
  volatile MemoryContext oldctx;
  volatile ResourceOwner oldowner;
  plwasm_pg_statement_context_t *stmctx;
  Datum val;

  oldowner = CurrentResourceOwner;
  oldctx = plwasm_utils_pg_memctx_switch_to_proc_context(cctx);
  plwasm_spi_ready(cctx);
  PG_TRY();
  {
    plwasm_spi_internal_transaction_begin(cctx);
    stmctx = plwasm_spi_statement_new(cctx, stmt_txt);
    plwasm_spi_statement_prepare(cctx, stmctx);
    plwasm_spi_statement_execute(cctx, stmctx, 1);
    if (!plwasm_spi_resultset_fetch(cctx, stmctx)) {
      CALL_ERROR(cctx, "no result set");
    }
    val = plwasm_spi_resultset_get_val_as(
	cctx, stmctx, 0, desired_type, is_null);
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


static plwasm_pg_statement_context_t*
statement_context_vec_new(
	int sz
) {
	plwasm_pg_statement_context_t *stmctx_vec;
	plwasm_pg_statement_context_t *stmctx;
	stmctx_vec = (plwasm_pg_statement_context_t*) palloc(
		sizeof(plwasm_pg_statement_context_t) * sz);
	stmctx = stmctx_vec;
	for (int i = 0; i < sz; ++i) {
		stmctx->id = i;
		statement_context_init(stmctx);
		++stmctx;
	}
	return stmctx_vec;
}

static plwasm_pg_statement_context_t*
statement_context_vec_find_free(
	plwasm_call_context_t *cctx
) {
	plwasm_pg_statement_context_t* it = cctx->spi.stmctx_vec;
	for (int i = 0; i < cctx->spi.stmctx_vec_sz; ++i) {
		if (statement_context_is_free(it)) {
			return it;
		}
		++it;
	}
	CALL_ERROR(cctx, "The number of active statements has reached ther upper limit.");
}

static
plwasm_pg_statement_context_t*
statement_context_vec_get(
	plwasm_call_context_t *cctx,
	int stmt_id
) {
	plwasm_pg_statement_context_t *stmctx;

	if (cctx->spi.stmctx_vec == NULL) {
		CALL_ERROR(cctx, "Statement was not created.");
	}

	if (stmt_id < 0 || stmt_id >= cctx->spi.stmctx_vec_sz) {
		CALL_ERROR(cctx, "Invalid statement id. id=%d", stmt_id);
	}
	
	stmctx = &(cctx->spi.stmctx_vec[stmt_id]);
	if (stmctx->statement_text == NULL) {
		CALL_ERROR(cctx, "Invalid statement index. Statement was not created. id=%d", stmt_id);
	}
	return stmctx;
}

static
bool
statement_context_is_free(
	plwasm_pg_statement_context_t *stmctx
) {
  return stmctx->statement_text == NULL;
}

static void
statement_context_init(
  plwasm_pg_statement_context_t *stmctx
) {
        stmctx->status = 0;
        stmctx->statement_text = NULL;
        stmctx->plan = NULL;
        stmctx->plan_is_query = false;
        stmctx->tuptable = NULL;
        stmctx->processed = -1;
        cursor_context_init(stmctx);
}

static void
cursor_context_init(
  plwasm_pg_statement_context_t *stmctx
) {
	stmctx->cursor.portal = NULL;
        stmctx->cursor.pos = -1;
        stmctx->cursor.is_null = false;
}

static void
check_resultset_ready(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx
) {
	if (stmctx->status <= 0) {
		CALL_ERROR(cctx, "statement was not succeeded.");
	}

	if (stmctx->cursor.portal == NULL) {
		CALL_ERROR(cctx, "cursor was not opened.");
	}
}

static void
check_resultset_fetch_status(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx
) {
	if (stmctx->tuptable == NULL) {
		CALL_ERROR(cctx, "no results");
	}

        if (stmctx->cursor.pos < 0) {
		CALL_ERROR(cctx, "invalid current position.");
        }
}

static Form_pg_attribute
get_att_desc(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t *stmctx,
	int att_idx
) {
	const char *FUNC_NAME = "get_att_desc";
	Form_pg_attribute att;

	CALL_DEBUG5(cctx, "%s begin. index=%d", FUNC_NAME, att_idx);
        if (att_idx < 0 || att_idx >= stmctx->tuptable->tupdesc->natts) {
		CALL_ERROR(cctx, "attribute index is out of range.");
        }

	att = TupleDescAttr(stmctx->tuptable->tupdesc, att_idx);
        if (att->attisdropped) {
		CALL_ERROR(cctx, "attribute was dropped.");
        }
	return att;
}
