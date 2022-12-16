bits 64

%include "macros.mac"

DEFAULT REL

%macro do_jcc 0-2+.nolist
            inst  STRICT %2 %%rip0 %1
    %%rip0: db    1
            align 16, db 0
%endmacro

%macro create_fixture 1
  %xdefine inst %1
  global %1_fixture:data
  %1_fixture:
    do_jcc + 0, SHORT
    do_jcc + 1, SHORT
    do_jcc - 1, SHORT
    do_jcc + 127, SHORT
    do_jcc - 128, SHORT
    do_jcc + 128
    do_jcc - 129
    do_jcc + 0x81001
    do_jcc - 0x98765
%endmacro

create_fixture ja
create_fixture jae
create_fixture jb
create_fixture jbe
create_fixture je
create_fixture jg
create_fixture jge
create_fixture jl
create_fixture jle
create_fixture jne
create_fixture jno
create_fixture jnp
create_fixture jns
create_fixture jo
create_fixture jp
create_fixture js
