#include "plwasm_context.h"
#include "plwasm_log.h"

/*
 * Extension Context
 */
void
plwasm_extension_context_init(
  plwasm_extension_context_t *ectx
) {
  ectx->type = plwasm_CTX_TYPE_EXTENSION;
  ectx->modules.pg = NULL;
}

/*
 * Call Context
 */
void
plwasm_call_context_init(
  plwasm_call_context_t *cctx,
  plwasm_extension_context_t *ectx,
  FunctionCallInfo fcinfo
) {
  cctx->type = plwasm_CTX_TYPE_CALL;
  cctx->ectx = ectx;
  cctx->memctx_default = NULL;
  cctx->func_config.file = NULL;
  cctx->func_config.wat = NULL;
  cctx->func_config.func_name = NULL;
  cctx->func_config.string_enc_name = NULL;
  cctx->func_config.cache.instance.enabled = true;
  cctx->func_config.trace = false;
  cctx->func_config.stats = true;
  cctx->module = NULL;
  cctx->instance = NULL;
  cctx->ret.type = VOIDOID;
  cctx->entry_point_name = "";
  cctx->fcinfo = fcinfo;
}

void
plwasm_call_context_destroy(
  plwasm_call_context_t *cctx
) {
}

