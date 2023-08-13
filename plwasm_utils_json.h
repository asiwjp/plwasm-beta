#ifndef H_PLWASM_UTILS_JSON
#define H_PLWASM_UTILS_JSON

#include <stdbool.h>
#include <postgres.h>
#include <utils/jsonb.h>

JsonbValue* plwasm_json_get_value(
  Jsonb *json,
  char *key,
  bool required
);

char* plwasm_json_get_value_as_cstring(
  Jsonb *json,
  char *key,
  bool required
);

bool plwasm_json_get_value_as_bool(
  Jsonb *jb,
  char *key,
  bool required
)
;

#endif
