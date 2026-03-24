.macro incbin name file
    .section .rodata
    .global \name
    .global \name\()_size
\name:
    .incbin "\file"
\name\()_size: .int . - \name
.endm

incbin syntax_asm "./syntax/asm.syx"
incbin syntax_c "./syntax/c.syx"
incbin syntax_fl "./syntax/fl.syx"
incbin syntax_js "./syntax/js.syx"
incbin syntax_json "./syntax/json.syx"
incbin syntax_lua "./syntax/lua.syx"
