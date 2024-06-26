import * as pglib from "./pglib"

export function log() : void {
  const argbuf = new ArrayBuffer(8192);
  pglib.args_get_text_unsafe(argbuf, argbuf.byteLength, 0, argbuf.byteLength);
  const arg = String.UTF8.decode(argbuf, true);

  let msg = String.UTF8.encode(arg);
  pglib.log_unsafe(0, msg, msg.byteLength);
}

export function ret_null() : void {
  const is_null = pglib.args_is_null(0);
  if (is_null) {
    pglib.returns_set_null();
  } else {
    pglib.returns_set_int32(pglib.args_get_int32(0));
  }
}

export function ret_int() : void {
  const arg = pglib.args_get_int32(0);
  pglib.returns_set_int32(arg + 1);
}

export function ret_bigint() : void {
  const arg = pglib.args_get_int64(0);
  pglib.returns_set_int64(arg + 1);
}

export function ret_real() : void {
  const arg = pglib.args_get_float32(0);
  pglib.returns_set_float32(arg);
}

export function ret_double() : void {
  const arg = pglib.args_get_float64(0);
  pglib.returns_set_float64(arg);
}

export function ret_text() : void {
  const argbuf = new ArrayBuffer(8192);
  pglib.args_get_text_unsafe(argbuf, argbuf.byteLength, 0, argbuf.byteLength);
  const arg = String.UTF8.decode(argbuf, true);

  let ret = String.UTF8.encode(arg, true);
  pglib.returns_set_text_unsafe(ret, ret.byteLength);
}

export function ret_bytea() : void {
  const buf = new ArrayBuffer(8192);
  const n = pglib.args_get_bytea_unsafe(buf, buf.byteLength, 0, 0, 0, -1);
  pglib.returns_set_bytea_unsafe(buf, buf.byteLength, 0, n);
}

export function query_int() : void {
  const stmt_buf = String.UTF8.encode("select max(id) from test", true);
  const ret = pglib.query_int32_unsafe(stmt_buf, stmt_buf.byteLength)

  pglib.returns_set_int32(ret);
}

export function query_text() : void {
  const stmt_buf = String.UTF8.encode("select max(id)::text from test", true);
  const outbuf = new ArrayBuffer(8192);
  const outbuflen = pglib.query_text_unsafe(outbuf, outbuf.byteLength, stmt_buf, stmt_buf.byteLength)

  pglib.returns_set_text_unsafe(outbuf, outbuflen);
}

export function fetch() : void {
  let ret : i64 = 0;
  const query = "select id"
     + ", id::bigint + 2147483647::bigint"
     + ", id::real + 0.1::real"
     + ", id::double precision + 2147483647.1::double precision"
     + ", id::text"
     + "  from test limit 10";
  const stmt_buf = String.UTF8.encode(query, true);
  const stmt_id = pglib.statement_new_unsafe(stmt_buf, stmt_buf.byteLength);
  pglib.statement_prepare(stmt_id);
  pglib.statement_execute(stmt_id);

  const text_buf = new ArrayBuffer(8192);
  while (pglib.resultset_fetch(stmt_id)) {
    const int32_val = pglib.resultset_get_int32(stmt_id, 0);
    const int64_val = pglib.resultset_get_int64(stmt_id, 1);
    const float32_val = pglib.resultset_get_float32(stmt_id, 2);
    const float64_val = pglib.resultset_get_float64(stmt_id, 3);
    const text_len = pglib.resultset_get_text_unsafe(text_buf, text_buf.byteLength, stmt_id, 4);
    const text_val = String.UTF8.decode(text_buf, true);

    const log = "rec=" + int32_val.toString()
        + "," + int64_val.toString()
        + "," + float32_val.toString()
        + "," + float64_val.toString()
        + "," + text_val.toString() + "(" + text_len.toString() + ")";
    const logbuf = String.UTF8.encode(log);
    pglib.log_unsafe(0, logbuf, logbuf.byteLength);

    ret += int64_val;
  }
  pglib.resultset_close(stmt_id);
  pglib.statement_close(stmt_id);

  pglib.returns_set_int64(ret);
}

function myAbort(message: usize, fileName: usize, line: u32, column: u32) : void {
  //nop
}
