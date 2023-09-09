//
// Declare PL/wasm built-in functions
//
@external("pg", "log_unsafe")
declare function pg_log_unsafe(level : i32, value : ArrayBuffer, sz : i32) : void;

@external("pg", "args_is_null")
declare function pg_args_is_null(
  index : i32
) : i32;

@external("pg", "args_get_int32")
declare function pg_args_get_int32(
  index : i32
) : i32;

@external("pg", "args_get_text_unsafe")
declare function pg_args_get_text_unsafe(
  buf : ArrayBuffer,
  sz : i32,
  arg_index : i32,
  n : i32
) : i32;

@external("pg", "args_get_bytea_unsafe")
declare function pg_args_get_bytea_unsafe(
  buf : ArrayBuffer, 
  sz : i32,
  buf_offset : i32,
  arg_index : i32,
  bytea_offset : i32,
  n : i32
) : i32;

@external("pg", "returns_set_null")
declare function pg_returns_set_null(
) : void;

@external("pg", "returns_set_int32")
declare function pg_returns_set_int32(
  value : i32
) : void;

@external("pg", "returns_set_text_unsafe")
declare function pg_returns_set_text_unsafe(
  buf : ArrayBuffer,
  sz : i32
) : void;

@external("pg", "returns_set_bytea_unsafe")
declare function pg_returns_set_bytea_unsafe(
  buf : ArrayBuffer,
  sz : i32,
  offset : i32,
  n : i32
) : void;

@external("pg", "query_int32_unsafe")
declare function pg_query_int32_unsafe(
  stmt_buf : ArrayBuffer,
  stmt_buf_sz : i32,
) : i32;

@external("pg", "query_text_unsafe")
declare function pg_query_text_unsafe(
  outbuf : ArrayBuffer,
  outbuf_sz : i32,
  stmt_buf : ArrayBuffer,
  stmt_buf_sz : i32
) : i32;

@external("pg", "statement_new_unsafe")
declare function pg_statement_new_unsafe(
  stmt_buf : ArrayBuffer,
  stmt_buf_sz : i32
) : i32;

@external("pg", "statement_prepare")
declare function pg_statement_prepare(
  stmt_id : i32
) : void;

@external("pg", "statement_execute")
declare function pg_statement_execute(
  stmt_id : i32,
) : i64;

@external("pg", "statement_close")
declare function pg_statement_close(
  stmt_id : i32,
) : i32;

@external("pg", "resultset_fetch")
declare function pg_resultset_fetch(
  stmt_id : i32
) : i32;

@external("pg", "resultset_close")
declare function pg_resultset_close(
  stmt_id : i32
) : i32;

@external("pg", "resultset_get_int32")
declare function pg_resultset_get_int32(
  stmt_id : i32,
  fld_idx : i32
) : i32;

@external("pg", "resultset_get_text_unsafe")
declare function pg_resultset_get_text_unsafe(
  buf : ArrayBuffer,
  sz : i32,
  stmt_id : i32,
  fld_idx : i32
) : i32;


//
// Functions to be published for PL/wasm
//
export function log() : void {
  const argbuf = new ArrayBuffer(8192);
  pg_args_get_text_unsafe(argbuf, argbuf.byteLength, 0, argbuf.byteLength);
  const arg = String.UTF8.decode(argbuf, true);

  let msg = String.UTF8.encode(arg);
  pg_log_unsafe(0, msg, msg.byteLength);
}

export function ret_null() : void {
  const is_null = pg_args_is_null(0);
  if (is_null) {
    pg_returns_set_null();
  } else {
    pg_returns_set_int32(pg_args_get_int32(0));
  }
}

export function ret_int() : void {
  const arg = pg_args_get_int32(0);
  pg_returns_set_int32(arg + 1);
}

export function ret_text() : void {
  const argbuf = new ArrayBuffer(8192);
  pg_args_get_text_unsafe(argbuf, argbuf.byteLength, 0, argbuf.byteLength);
  const arg = String.UTF8.decode(argbuf, true);

  let ret = String.UTF8.encode(arg, true);
  pg_returns_set_text_unsafe(ret, ret.byteLength);
}

export function ret_bytea() : void {
  const buf = new ArrayBuffer(8192);
  const n = pg_args_get_bytea_unsafe(buf, buf.byteLength, 0, 0, 0, -1);
  pg_returns_set_bytea_unsafe(buf, buf.byteLength, 0, n);
}

export function query_int() : void {
  const stmt_buf = String.UTF8.encode("select max(id) from test", true);
  const ret = pg_query_int32_unsafe(stmt_buf, stmt_buf.byteLength)

  pg_returns_set_int32(ret);
}

export function query_text() : void {
  const stmt_buf = String.UTF8.encode("select max(id)::text from test", true);
  const outbuf = new ArrayBuffer(8192);
  const outbuflen = pg_query_text_unsafe(outbuf, outbuf.byteLength, stmt_buf, stmt_buf.byteLength)

  pg_returns_set_text_unsafe(outbuf, outbuflen);
}

export function fetch_int() : void {
  let ret : i32 = 0;
  const stmt_buf = String.UTF8.encode("select id from test limit 10", true);
  const stmt_id = pg_statement_new_unsafe(stmt_buf, stmt_buf.byteLength);
  pg_statement_prepare(stmt_id);
  pg_statement_execute(stmt_id);
  while (pg_resultset_fetch(stmt_id)) {
    const int32_val = pg_resultset_get_int32(stmt_id, 0);
    const log = "rec=" + int32_val.toString();
    const logbuf = String.UTF8.encode(log);
    pg_log_unsafe(0, logbuf, logbuf.byteLength);
    ret++;
  }
  pg_resultset_close(stmt_id);
  pg_statement_close(stmt_id);

  pg_returns_set_int32(ret);
}

export function fetch_text() : void {
  let ret : i32 = 0;
  const stmt_buf = String.UTF8.encode("select id::text from test limit 10", true);
  const stmt_id = pg_statement_new_unsafe(stmt_buf, stmt_buf.byteLength);
  pg_statement_prepare(stmt_id);
  pg_statement_execute(stmt_id);
  const fldbuf = new ArrayBuffer(8192);
  while (pg_resultset_fetch(stmt_id)) {
    const text_len = pg_resultset_get_text_unsafe(fldbuf, fldbuf.byteLength, stmt_id, 0);
    const log = String.UTF8.decode(fldbuf, true) + "(" + text_len.toString() + ")";
    const logbuf = String.UTF8.encode(log);
    pg_log_unsafe(0, logbuf, logbuf.byteLength);
    ret++;
  }
  pg_resultset_close(stmt_id);
  pg_statement_close(stmt_id);

  pg_returns_set_int32(ret);
}

function myAbort(message: usize, fileName: usize, line: u32, column: u32) : void {
  //nop
}
