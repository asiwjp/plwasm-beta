//
// Declare PL/wasm built-in functions
//
@external("pg", "log_unsafe")
export declare function log_unsafe(
  level : i32,
  value : ArrayBuffer,
  sz : i32
) : void;

@external("pg", "args_is_null")
export declare function args_is_null(
  index : i32
) : i32;

@external("pg", "args_get_int32")
export declare function args_get_int32(
  index : i32
) : i32;

@external("pg", "args_get_int64")
export declare function args_get_int64(
  index : i32
) : i64;

@external("pg", "args_get_float32")
export declare function args_get_float32(
  index : i32
) : f32;

@external("pg", "args_get_float64")
export declare function args_get_float64(
  index : i32
) : f64;

@external("pg", "args_get_text_unsafe")
export declare function args_get_text_unsafe(
  buf : ArrayBuffer,
  sz : i32,
  arg_index : i32,
  n : i32
) : i32;

@external("pg", "args_get_bytea_unsafe")
export declare function args_get_bytea_unsafe(
  buf : ArrayBuffer, 
  sz : i32,
  buf_offset : i32,
  arg_index : i32,
  bytea_offset : i32,
  n : i32
) : i32;

@external("pg", "returns_set_null")
export declare function returns_set_null(
) : void;

@external("pg", "returns_set_int32")
export declare function returns_set_int32(
  value : i32
) : void;

@external("pg", "returns_set_int64")
export declare function returns_set_int64(
  value : i64
) : void;

@external("pg", "returns_set_float32")
export declare function returns_set_float32(
  value : f32
) : void;

@external("pg", "returns_set_float64")
export declare function returns_set_float64(
  value : f64
) : void;

@external("pg", "returns_set_text_unsafe")
export declare function returns_set_text_unsafe(
  buf : ArrayBuffer,
  sz : i32
) : void;

@external("pg", "returns_set_bytea_unsafe")
export declare function returns_set_bytea_unsafe(
  buf : ArrayBuffer,
  sz : i32,
  offset : i32,
  n : i32
) : void;

@external("pg", "query_int32_unsafe")
export declare function query_int32_unsafe(
  stmt_buf : ArrayBuffer,
  stmt_buf_sz : i32,
) : i32;

@external("pg", "query_text_unsafe")
export declare function query_text_unsafe(
  outbuf : ArrayBuffer,
  outbuf_sz : i32,
  stmt_buf : ArrayBuffer,
  stmt_buf_sz : i32
) : i32;

@external("pg", "statement_new_unsafe")
export declare function statement_new_unsafe(
  stmt_buf : ArrayBuffer,
  stmt_buf_sz : i32
) : i32;

@external("pg", "statement_prepare")
export declare function statement_prepare(
  stmt_id : i32
) : void;

@external("pg", "statement_execute")
export declare function statement_execute(
  stmt_id : i32,
) : i64;

@external("pg", "statement_close")
export declare function statement_close(
  stmt_id : i32,
) : i32;

@external("pg", "resultset_fetch")
export declare function resultset_fetch(
  stmt_id : i32
) : i32;

@external("pg", "resultset_close")
export declare function resultset_close(
  stmt_id : i32
) : i32;

@external("pg", "resultset_get_int32")
export declare function resultset_get_int32(
  stmt_id : i32,
  fld_idx : i32
) : i32;

@external("pg", "resultset_get_int64")
export declare function resultset_get_int64(
  stmt_id : i32,
  fld_idx : i32
) : i64;

@external("pg", "resultset_get_float32")
export declare function resultset_get_float32(
  stmt_id : i32,
  fld_idx : i32
) : f32;

@external("pg", "resultset_get_float64")
export declare function resultset_get_float64(
  stmt_id : i32,
  fld_idx : i32
) : f64;

@external("pg", "resultset_get_text_unsafe")
export declare function resultset_get_text_unsafe(
  buf : ArrayBuffer,
  sz : i32,
  stmt_id : i32,
  fld_idx : i32
) : i32;

