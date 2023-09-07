#include "plwasm_utils_json.h"
#include "plwasm_log.h"

JsonbValue*
plwasm_json_get_prop_value(
  Jsonb *jb,
  char *key,
  bool required
) {
  JsonbValue kval;
  JsonbValue *v = NULL;

  kval.type = jbvString;
  kval.val.string.val = key;
  kval.val.string.len = strlen(key);
  
  v = findJsonbValueFromContainer(&(jb->root), JB_FOBJECT, &kval);
  if (v == NULL && required) {
    CALL_ERROR(cctx, "Invalid json config. property not found. name=%s.", key);
  }

  return v;
}

JsonbValue*
plwasm_json_get_value(
  Jsonb *jb,
  char *path,
  bool required
) {
  char *p = strtok(pstrdup(path), ".");
  JsonbValue *jv = plwasm_json_get_prop_value(jb, p, required);
  
  while (jv != NULL) {
    p = strtok(NULL, ".");
    if (p == NULL) {
      return jv;
    }

    jv = plwasm_json_get_prop_value(JsonbValueToJsonb(jv), p, required);
  }
  return NULL;
}

char*
plwasm_json_get_value_as_cstring(
  Jsonb *jb,
  char *path,
  bool required
) {
  JsonbValue *v = NULL;
  v = plwasm_json_get_value(jb, path, required); 
  if (v == NULL) return NULL;
  return pnstrdup(v->val.string.val, v->val.string.len);
}

bool
plwasm_json_get_value_as_bool(
  Jsonb *jb,
  char *path,
  bool required,
  bool default_value
) {
  JsonbValue *v = NULL;
  v = plwasm_json_get_value(jb, path, required); 
  if (v == NULL) return default_value;
  return v->val.boolean;
}
