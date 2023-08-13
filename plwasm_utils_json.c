#include "plwasm_utils_json.h"
#include "plwasm_log.h"

JsonbValue* plwasm_json_get_value(Jsonb *jb, char *key, bool required) {
  JsonbValue kval;
  JsonbValue *v = NULL;

  kval.type = jbvString;
  kval.val.string.val = key;
  kval.val.string.len = strlen(key);
  
  v = findJsonbValueFromContainer(&jb->root, JB_FOBJECT, &kval);
  if (v == NULL && required) {
    CALL_ERROR(cctx, "Invalid json config. property not found. name=%s.", key);
  }

  return v;
}

char* plwasm_json_get_value_as_cstring(Jsonb *jb, char *key, bool required) {
  JsonbValue *v = NULL;
  v = plwasm_json_get_value(jb, key, required); 
  if (v == NULL) return NULL;
  return pnstrdup(v->val.string.val, v->val.string.len);
}

bool plwasm_json_get_value_as_bool(Jsonb *jb, char *key, bool required) {
  JsonbValue *v = NULL;
  v = plwasm_json_get_value(jb, key, required); 
  if (v == NULL) return false;
  return v->val.boolean;
}
