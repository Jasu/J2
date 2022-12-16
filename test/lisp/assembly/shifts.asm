bits 64

%include "macros.mac"

%macro do_shift 2+.nolist
  inst  %1, %2
  db    1
  align 16, db 0
%endmacro

%macro do_mem_shift 3.nolist
  inst  %3 [%1], %2
  db    1
  align 16, db 0
%endmacro


%macro create_fixture 1
  %xdefine inst %1
  global %1_fixture:data
  %1_fixture:

  with_regs 8, do_shift, 1
  with_regs 16, do_shift, 1
  with_regs 32, do_shift, 1
  with_regs 64, do_shift, 1

  with_regs 8, do_shift,  cl
  with_regs 16, do_shift, cl
  with_regs 32, do_shift, cl
  with_regs 64, do_shift, cl

  with_regs 8, do_shift,  5
  with_regs 16, do_shift, 5
  with_regs 32, do_shift, 5
  with_regs 64, do_shift, 5

  with_regs 64, do_mem_shift, 1, BYTE
  with_regs 64, do_mem_shift, cl, BYTE
  with_regs 64, do_mem_shift, 6, BYTE

  with_regs 64, do_mem_shift, 1, WORD
  with_regs 64, do_mem_shift, cl, WORD
  with_regs 64, do_mem_shift, 6, WORD

  with_regs 64, do_mem_shift, 1, DWORD
  with_regs 64, do_mem_shift, cl, DWORD
  with_regs 64, do_mem_shift, 6, DWORD

  with_regs 64, do_mem_shift, 1, QWORD
  with_regs 64, do_mem_shift, cl, QWORD
  with_regs 64, do_mem_shift, 6, QWORD
%endmacro

create_fixture rcl
create_fixture rcr
create_fixture rol
create_fixture ror
create_fixture sal
create_fixture sar
create_fixture shl
create_fixture shr
