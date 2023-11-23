CREATE OR REPLACE FUNCTION plwasm_assembly_script_drop_funcs(prefix text) RETURNS VOID AS $$
DECLARE
  procs RECORD;
BEGIN
  FOR procs IN
    SELECT 'DROP FUNCTION ' || oid::regprocedure as drop_fn_sql
    FROM pg_proc
    where proname like prefix
      AND pg_function_is_visible(oid)
      AND proname not like '%handler%'
  LOOP
    EXECUTE procs.drop_fn_sql;
  END LOOP;
END;
$$ LANGUAGE plpgsql;

SELECT plwasm_assembly_script_drop_funcs('plwasm_assembly_script%');


create or replace function plwasm_assembly_script_query_int(p1 int) returns int language plwasm as
$${
  "file": "plwasm/assembly_script.wasm",
  "enc": "utf-8",
  "func": "query_int"
}$$;

create or replace function plwasm_assembly_script_query_text(p1 int) returns text language plwasm as
$${
  "file": "plwasm/assembly_script.wasm",
  "enc": "utf-8",
  "func": "query_text"
}$$;

create or replace function plwasm_assembly_script_log(p1 text) returns void language plwasm as
$${
  "file": "plwasm/assembly_script.wasm",
  "enc": "utf-8",
  "func": "log"
}$$;

create or replace function plwasm_assembly_script_ret_null(p1 int) returns int language plwasm as
$${
  "file": "plwasm/assembly_script.wasm",
  "enc": "utf-8",
  "func": "ret_null"
}$$;

create or replace function plwasm_assembly_script_ret_int(p1 int) returns int language plwasm as
$${
  "file": "plwasm/assembly_script.wasm",
  "enc": "utf-8",
  "func": "ret_int"
}$$;

create or replace function plwasm_assembly_script_ret_bigint(p1 bigint) returns bigint language plwasm as
$${
  "file": "plwasm/assembly_script.wasm",
  "enc": "utf-8",
  "func": "ret_bigint"
}$$;

create or replace function plwasm_assembly_script_ret_text(p1 text) returns text language plwasm as
$${
  "file": "plwasm/assembly_script.wasm",
  "enc": "utf-8",
  "func": "ret_text"
}$$;

create or replace function plwasm_assembly_script_fetch_int(p1 int) returns bigint language plwasm as
$${
  "file": "plwasm/assembly_script.wasm",
  "enc": "utf-8",
  "func": "fetch_int"
}$$;

create or replace function plwasm_assembly_script_fetch_text(p1 int) returns int language plwasm as
$${
  "file": "plwasm/assembly_script.wasm",
  "enc": "utf-8",
  "func": "fetch_text"
}$$;
