CREATE FUNCTION plwasm_call_handler() RETURNS language_handler AS
    '$libdir/plwasm' LANGUAGE C;

CREATE TRUSTED PROCEDURAL LANGUAGE plwasm
    HANDLER plwasm_call_handler
;
