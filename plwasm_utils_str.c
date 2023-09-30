#include "plwasm_utils_str.h"

#include <string.h>
#include "postgres.h"
#include "mb/pg_wchar.h"
#include "plwasm_log.h"

bool plwasm_utils_str_eq_safe(
  const char *str1,
  int str1_len, 
  const char *str2,
  int str2_len
) {
   if (str1_len != str2_len) {
     return false;
   }

   if (memcmp(str1, str2, str2_len)) {
     return false;
   }

   return true;
}

bool plwasm_utils_str_startsWithN_safe(
  const char *str,
  int str_len, 
  const char *search,
  int search_len
) {
   if (str_len < search_len) {
     return false;
   }

   if (memcmp(str, search, search_len)) {
     return false;
   }

   return true;
}

bool plwasm_utils_str_endsWithN_safe(
  const char *str,
  int str_len, 
  const char *search,
  int search_len
) {
   if (str_len < search_len) {
     return false;
   }

   if (memcmp(str + str_len - search_len, search, search_len)) {
     return false;
   }

   return true;
}

char* plwasm_utils_str_enc(
  plwasm_call_context_t *cctx,
  const char *src,
  int src_len,
  int src_enc,
  int dest_enc,
  bool force_null_termination,
  size_t *converted_sz
) {
  const char *FUNC_NAME = "plwasm_utils_str_enc";
  char *dest;

  CALL_DEBUG5(cctx,
    "%s begin. from=%d, to=%d, src_len=%d, force_null_termiation=%d",
    FUNC_NAME, src_enc, dest_enc, src_len, force_null_termination);

  // TODO UTF16-UTF16

  if (dest_enc == src_enc) {
    CALL_DEBUG5(cctx, "%s skip.", FUNC_NAME);
    dest = pnstrdup(src, src_len); // add null termination
    *converted_sz = strlen(dest) + 1;
    return dest;
  }

  if (src_enc < 0) {
    CALL_ERROR(cctx, "Invalid src encoding.");
  }

  if (dest_enc < 0) {
    CALL_ERROR(cctx, "Invalid dest encoding.");
  }

  dest = (char*)pg_do_encoding_conversion(
    (unsigned char*)src,
    src_len,
    src_enc,
    dest_enc);
  *converted_sz = strlen(dest) + 1;

  CALL_DEBUG5(cctx,
    "%s success. from=%d, to=%d, src_len=%d, force_null_termiation=%d",
    FUNC_NAME, src_enc, dest_enc, src_len, force_null_termination);
  return dest;
}

