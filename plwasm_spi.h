#ifndef H_PLWASM_UTILS_PG_SPI
#define H_PLWASM_UTILS_PG_SPI

#include "plwasm_types.h"

void 
plwasm_spi_ready(
	plwasm_call_context_t *cctx
);

void 
plwasm_spi_finish(
	plwasm_call_context_t *cctx
);

void
plwasm_spi_internal_transaction_begin(
	plwasm_call_context_t *cctx
);

void
plwasm_spi_internal_transaction_commit(
	plwasm_call_context_t *cctx,
	ResourceOwner oldowner
);

void
plwasm_spi_internal_transaction_abort(
	plwasm_call_context_t *cctx,
	ResourceOwner oldowner
);

ErrorData*
plwasm_spi_err_capture(
	plwasm_call_context_t *cctx
);

plwasm_pg_statement_context_t*
plwasm_spi_statement_create(
	plwasm_call_context_t *cctx,
	char *statement
);

plwasm_pg_statement_context_t*
plwasm_spi_statement_get_context(
	plwasm_call_context_t *cctx,
	int stmt_id
);

void
plwasm_spi_statement_prepare(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t* stmctx
);

uint64_t
plwasm_spi_statement_execute(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t* stmctx,
	int limit
);

bool
plwasm_spi_statement_close(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t* stmctx
);

bool
plwasm_spi_resultset_is_opened(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t* stmctx
);

bool
plwasm_spi_resultset_fetch(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t* stmctx
);

bool
plwasm_spi_resultset_close(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t* stmctx
);

int
plwasm_spi_resultset_meta_get_att_count(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t* stmctx
);

Form_pg_attribute
plwasm_spi_resultset_meta_get_att_desc(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t* stmctx,
	int att_idx
);

Datum
plwasm_spi_resultset_get_val_as(
	plwasm_call_context_t *cctx,
	plwasm_pg_statement_context_t* stmctx,
	int att_index,
	Oid desired_type,
	bool *is_null
);

Datum
plwasm_spi_query_scalar_as(
	plwasm_call_context_t *cctx,
	char	*statement,
	Oid	desired_type,
	bool	*is_null
);

#endif
