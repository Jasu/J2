bits 64

%include "macros.mac"

%macro do_bt 2
  bt   %1, %2
  db 1
  align 16, db 0
%endmacro

%macro do_bt_mem 3
  bt   %3 [%1], %2
  db 1
  align 16, db 0
%endmacro

global bt_reg_fixture:data
bt_reg_fixture:
  with_regs 64, do_bt, rax
  with_regs 64, do_bt, r14
  with_regs 32, do_bt, eax
  with_regs 16, do_bt, cx

  with_regs 64, do_bt, 1
  with_regs 32, do_bt, 2
  with_regs 16, do_bt, 4

global bt_mem_fixture:data
bt_mem_fixture:

  with_regs 64, do_bt_mem, rax, QWORD
  with_regs 64, do_bt_mem, r14, QWORD
  with_regs 64, do_bt_mem, eax, DWORD
  with_regs 64, do_bt_mem, cx, WORD

  with_regs 64, do_bt_mem, 1, QWORD
  with_regs 64, do_bt_mem, 2, DWORD
  with_regs 64, do_bt_mem, 3, WORD
