bits 64

%include "macros.mac"

DEFAULT REL

%macro jmp_rel 0-2+.nolist
            jmp   %2 %%rip0 %1
    %%rip0: db    1
            align 16, db 0
%endmacro

global jmp_rel_fixture:data
jmp_rel_fixture:
  jmp_rel + 0, SHORT
  jmp_rel + 1, SHORT
  jmp_rel - 1, SHORT
  jmp_rel + 127, SHORT
  jmp_rel - 128, SHORT
  jmp_rel + 128
  jmp_rel - 129
  jmp_rel + 0x81001
  jmp_rel - 0x98765

%macro jmp_reg 1.nolist
            jmp   %1
            db    1
            align 16, db 0
%endmacro


global jmp_reg_fixture:data
jmp_reg_fixture:
  with_regs 64, jmp_reg

%macro jmp_mem 1-2+.nolist
            jmp   [%1 %2]
            db    1
            align 16, db 0
%endmacro

global jmp_mem_fixture:data
jmp_mem_fixture:
  with_regs 64, jmp_mem
  with_regs 64, jmp_mem, + rax
  with_regs 64, jmp_mem, + 2*r8
  with_regs 64, jmp_mem, + 178
