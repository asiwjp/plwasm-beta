#ifndef H_PLWASM_UTILS_STR
#define H_PLWASM_UTILS_STR

#include <stdio.h>
#include <stdbool.h>
#include "plwasm_types.h"

bool plwasm_utils_str_eq_safe(
  const char *str1,
  int str1_len,
  const char *str2,
  int str2_len
);

bool plwasm_utils_str_startsWithN_safe(
  const char *str,
  int str_len,
  const char *search,
  int search_len
);

bool plwasm_utils_str_endsWithN_safe(
  const char *str,
  int str_len,
  const char *search,
  int search_len
);

char* plwasm_utils_str_enc(
  plwasm_call_context_t *cctx,
  const char *str,
  int str_len,
  int src_enc,
  int dest_enc,
  bool force_null_termination,
  size_t *dest_sz
);

#endif
