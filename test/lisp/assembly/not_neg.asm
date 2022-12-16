bits 64

%include "macros.mac"

%macro do_neg 1.nolist
  inst  %1
  db    1
  align 16, db 0
%endmacro

%macro do_mem_neg 2.nolist
  inst  %2 [%1]
  db    1
  align 16, db 0
  inst  %2 [%1 + rax]
  db    1
  align 16, db 0
  inst  %2 [%1 + 4*r14 - 10]
  db    1
  align 16, db 0
%endmacro


%macro create_fixture 1
  %xdefine inst %1
  global %1_fixture:data
  %1_fixture:

  with_regs 8, do_neg
  with_regs 16, do_neg
  with_regs 32, do_neg
  with_regs 64, do_neg

  with_regs 64, do_mem_neg, BYTE
  with_regs 64, do_mem_neg, WORD
  with_regs 64, do_mem_neg, DWORD
  with_regs 64, do_mem_neg, QWORD
%endmacro

create_fixture not
create_fixture neg
