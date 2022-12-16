bits 64

%include "macros.mac"

%macro do_mov 2
  inst  %1, %2
  db 1
  align 16, db 0
%endmacro

%macro do_movs_sized 2
  with_regs_2 %2, do_mov, %1
%endmacro

%macro do_movs_mem 3
  with_regs_2 %2, do_mov, %3 [%1]
%endmacro

%macro make_fixture 1
  global %1_fixture:data
  %xdefine inst %1
%1_fixture:
  with_regs 8, do_movs_sized, 16
  with_regs 8, do_movs_sized, 32
  with_regs 8, do_movs_sized, 64
  with_regs 16, do_movs_sized, 32
  with_regs 16, do_movs_sized, 64

  with_regs 64, do_movs_mem, 16, BYTE
  with_regs 64, do_movs_mem, 32, BYTE
  with_regs 64, do_movs_mem, 64, BYTE
  with_regs 64, do_movs_mem, 32, WORD
  with_regs 64, do_movs_mem, 64, WORD
%endmacro

make_fixture movsx
make_fixture movzx

global movsxd_fixture:data
  %xdefine inst movsxd
movsxd_fixture:
  with_regs 32, do_movs_sized, 64
  with_regs 64, do_movs_mem, 64, DWORD
