bits 64

%include "macros.mac"

DEFAULT REL

%macro do_setcc 1.nolist
  inst  %1
  db    1
  align 16, db 0
%endmacro

%macro do_setcc_mem 1.nolist
  do_setcc BYTE [%1]
  do_setcc BYTE [%1 + 4*rax]
  do_setcc BYTE [%1 + 8*r11 + 71]
%endmacro

%macro create_fixture 1
  %xdefine inst %1
  global %1_fixture:data
  %1_fixture:
    with_regs 8, do_setcc
    with_regs 64, do_setcc_mem
%endmacro

create_fixture seto
create_fixture setno
create_fixture setc
create_fixture setnc
create_fixture setz
create_fixture setnz
create_fixture setna
create_fixture seta
create_fixture sets
create_fixture setns
create_fixture setp
create_fixture setnp
create_fixture setl
create_fixture setnl
create_fixture setle
create_fixture setnle
