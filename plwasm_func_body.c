#include "plwasm_func_body.h"
#include "plwasm_log.h"
#include "plwasm_utils_str.h"
#include "plwasm_utils_json.h"

#include <stdbool.h>
#include <catalog/pg_collation_d.h>
#include <mb/pg_wchar.h>

bool plwasm_func_body_parse_wat(
  plwasm_call_context_t *cctx,
  char *proname,
  char *source,
  int source_len
);

bool plwasm_func_body_parse_json_config(
  plwasm_call_context_t *cctx,
  char *proname,
  char *source,
  int source_len
);

void
plwasm_func_body_describe(
  plwasm_call_context_t *cctx,
  plwasm_pg_proc_t *pg_proc,
  Oid pg_proc_oid
) {
  HeapTuple     pl_tuple;
  Form_pg_proc  pl_struct;
  Datum         ret;
  bool          isnull;

  /* Fetch the function's pg_proc entry. */
  pl_tuple = SearchSysCache1(PROCOID,
    ObjectIdGetDatum(pg_proc_oid));

  if (!HeapTupleIsValid(pl_tuple)) {
    CALL_ERROR(cctx,
      "cache lookup failed for function %u",
       pg_proc_oid);
  }

  pl_struct = (Form_pg_proc) GETSTRUCT(pl_tuple);
  ret = SysCacheGetAttr(PROCOID, pl_tuple, Anum_pg_proc_prosrc, &isnull);
  if (isnull) {
    CALL_ERROR(cctx,
      "could not find source text of function \"%d\"",
      pg_proc_oid);
  }

  pg_proc->name = pstrdup(NameStr(pl_struct->proname));
  pg_proc->source = DatumGetCString(DirectFunctionCall1(textout, ret));
  pg_proc->source_len = strlen(pg_proc->source);
  pg_proc->ret_type = pl_struct->prorettype;
  ReleaseSysCache(pl_tuple);
}

void plwasm_func_body_parse(
  plwasm_call_context_t *cctx,
  plwasm_pg_proc_t *pg_proc
) {

  char *FUNC_NAME = "plwasm_func_body_parse";
  char *proname;
  char *source;
  int source_len;
  bool success;

  proname = pg_proc->name;
  source = pg_proc->source;
  source_len = pg_proc->source_len;
  if (source_len == 0) {
    CALL_ERROR(cctx, "%s function source is empty. %s", FUNC_NAME, proname);
  }

  if (plwasm_func_body_parse_wat(cctx, proname, source, source_len)) {
    success = true;
  }

  if (plwasm_func_body_parse_json_config(cctx, proname, source, source_len)) {
    success = true;
  }

  if (!success) {
    CALL_ERROR(cctx, "%s invalid function body. %s", FUNC_NAME, proname);
  }

  CALL_DEBUG5(cctx, "func config file=%s", cctx->func_config.file);
  CALL_DEBUG5(cctx, "func config func=%s", cctx->func_config.func_name);
  CALL_DEBUG5(cctx, "func config enc=%s", cctx->func_config.string_enc_name);
  CALL_DEBUG5(cctx, "func config cache.instance.enabled=%d", cctx->func_config.cache.instance.enabled);
  CALL_DEBUG5(cctx, "func config stats=%d", (int)(cctx->func_config.stats));

  cctx->func_config.string_enc = pg_char_to_encoding(cctx->func_config.string_enc_name);
  if (cctx->func_config.string_enc == -1) {
    CALL_ERROR(cctx,
      "%s Invalid json config. string encoding was not supported. enc=%s",
      FUNC_NAME,
      cctx->func_config.string_enc_name);
  }
  cctx->func_config.string_enc_required = cctx->func_config.string_enc != GetDatabaseEncoding();
}

bool plwasm_func_body_parse_wat(
  plwasm_call_context_t *cctx,
  char *proname,
  char *source,
  int source_len) {

  bool is_wat;

  is_wat = false;

  if (source[0] == '(') {
    cctx->func_config.wat = source;
    is_wat = true;
  }

  if (is_wat) {
    cctx->func_config.func_name = pstrdup(proname);
    cctx->func_config.string_enc_name = "UTF-8";
  }
  return is_wat;
}

bool plwasm_func_body_parse_json_config(
  plwasm_call_context_t *cctx,
  char *proname,
  char *source,
  int source_len
) {

  //char *FUNC_NAME = "plwasm_func_body_parse_json_config";
  Jsonb *jb_root;

  if (!plwasm_utils_str_startsWithN_safe(source, source_len, "{", 1)) {
    return false;
  }

  CALL_DEBUG5(cctx, "function is with json config");
  jb_root = (Jsonb*) DirectFunctionCall1Coll(jsonb_in, DEFAULT_COLLATION_OID, (Datum)source);
  cctx->func_config.file = plwasm_json_get_value_as_cstring(jb_root, "file", true);
  cctx->func_config.func_name = plwasm_json_get_value_as_cstring(jb_root, "func", false);
  cctx->func_config.string_enc_name = plwasm_json_get_value_as_cstring(jb_root, "enc", true);
  cctx->func_config.trace = plwasm_json_get_value_as_bool(jb_root, "trace", false, false);
  cctx->func_config.stats = plwasm_json_get_value_as_bool(jb_root, "stats", false, false);
  cctx->func_config.cache.instance.enabled = plwasm_json_get_value_as_bool(jb_root, "cache.instance", false, true);

  if (cctx->func_config.func_name == NULL) {
     cctx->func_config.func_name = pstrdup(proname);
  }
  
  return true;
}
