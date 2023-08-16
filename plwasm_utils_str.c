#include "plwasm_utils_str.h"

#include <postgres.h>
#include <string.h>
#include <mb/pg_wchar.h>
#include <iconv.h>
#include "plwasm_log.h"

char* plwasm_utils_str_enc_iconv(
  const char *src,
  size_t src_len,
  const char *src_enc_name,
  const char *dest_enc_name,
  size_t dest_sz_hint,
  bool force_null_termination,
  size_t *converted_sz
);

char* plwasm_utils_str_enc_utf16_to_utf8(
  const char *src,
  int src_len,
  bool force_null_termination,
  size_t *converted_sz
);

char* plwasm_utils_str_enc_utf8_to_utf16(
  const char *src,
  int src_len,
  bool force_null_termination,
  size_t *converted_sz
);

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

char* plwasm_utils_str_enc_iconv(
  const char *src,
  size_t src_len,
  const char *src_enc_name,
  const char *dest_enc_name,
  size_t dest_sz_hint,
  bool force_null_termination,
  size_t *converted_sz
) {

  iconv_t icd;
  size_t icd_res;
  char *src_p;
  char *dest_begin;
  char *dest;
  size_t dest_len;
  size_t dest_left;
  size_t src_left;
  int null_pad;
  size_t converted_len;
  char tmp[1024];

  null_pad = (force_null_termination) ? 1 : 0;
  src_p = (char*) pnstrdup(src, src_len);
  dest_len = dest_sz_hint;
  dest_begin = dest = (char*) palloc(dest_len + null_pad);
  memset(dest, 0, dest_len);

  ereport(DEBUG5,
    (errmsg("iconv %s to %s, src_len=%ld, dest_len=%ld, force_null_termination=%d",
      src_enc_name,
      dest_enc_name,
      src_len,
      dest_len,
      (int)force_null_termination)));
  if (strncmp(src_enc_name, "UTF-16", 5) != 0) {
    strncpy(tmp, src, 1024);
    ereport(DEBUG5,
      (errmsg("iconv \"%s\" to %s",
        tmp,
        dest_enc_name
      )));
  }
  icd = iconv_open(dest_enc_name, src_enc_name);
  if (icd == (iconv_t)-1) {
    elog(ERROR, "iconv open failed");
  }
  src_left = src_len;
  dest_left = dest_len;
  icd_res = iconv(icd, &src_p, &src_left, &dest, &dest_left);
  iconv_close(icd);

  if (icd_res == (size_t)-1) {
    ereport(ERROR, (errmsg("iconv failed. dest_left=%ld, src_left=%ld", dest_left, src_left)));
  }

  converted_len = dest_len - dest_left;
  *converted_sz = converted_len + null_pad;
  ereport(DEBUG5,
    (errmsg("iconv success. converted=%ld(%ld), icd_res=%ld, dest_left=%ld",
      converted_len,
      *converted_sz,
      icd_res,
      dest_left)));
  if (force_null_termination) {
    dest[converted_len] = '\0';
  }

  return dest_begin;
}

char* plwasm_utils_str_enc_utf16_to_utf8(
  const char *src,
  int src_len,
  bool force_null_termination,
  size_t *converted_sz
) {

  return plwasm_utils_str_enc_iconv(
    src,
    src_len,
    "UTF-16LE",
    "UTF-8",
    src_len * 4,
    force_null_termination,
    converted_sz
  );
}

char* plwasm_utils_str_enc_utf8_to_utf16(
  const char *src,
  int src_len,
  bool force_null_termination,
  size_t *converted_sz
) {

  return plwasm_utils_str_enc_iconv(
    src,
    src_len,
    "UTF-8",
    "UTF-16LE",
    src_len * 4,
    force_null_termination,
    converted_sz
);
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
  char *utf8;
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

  if (src_enc == -1) {
    CALL_DEBUG5(cctx, "%s Performs intermediate conversion to UTF8.", FUNC_NAME);
    utf8 = plwasm_utils_str_enc_utf16_to_utf8(
      src,
      src_len,
      true,
      converted_sz);
    dest = plwasm_utils_str_enc(
      cctx,
      utf8, 
      strlen(utf8), 
      PG_UTF8,
      dest_enc,
      force_null_termination,
      converted_sz);
    pfree(utf8);
    return dest;
  }

  if (dest_enc == -1) {
    CALL_DEBUG5(cctx, "%s Performs intermediate conversion to UTF8.", FUNC_NAME);
    utf8 = plwasm_utils_str_enc(
      cctx,
      src, 
      src_len,
      src_enc,
      PG_UTF8,
      false,
      converted_sz);

    dest = plwasm_utils_str_enc_utf8_to_utf16(
      utf8, 
      *converted_sz - 1, // nil char
      force_null_termination,
      converted_sz);
    pfree(utf8);
    return dest;
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

