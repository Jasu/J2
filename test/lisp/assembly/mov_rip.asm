bits 64

%include "macros.mac"

DEFAULT REL

%macro read_rip 1.nolist
            mov   %$ri(%1), %$rip(%%rip0)
    %%rip0: db    1
            align 16, db 0
%endmacro

%macro write_rip 1.nolist
            mov   %$rip(%%rip0), %$ri(%1)
    %%rip0: db    1
            align 16, db 0
%endmacro

%macro simple_write_rip 3.nolist
            mov   width(%1) [%%rip0 + %2], %3
    %%rip0: db    1
            align 16, db 0
%endmacro

%macro read_rip_reg 2-3+
  %push read_rip_reg
  %define %$rip(r) strict width(%2) [r %3]
  %define %$ri(a) strict a
  with_regs %1, read_rip
  %pop
%endmacro

%macro write_rip_reg 2-3+
  %push write_rip_reg
  %define %$rip(r) strict width(%2) [r %3]
  %define %$ri(a) strict a
  with_regs %1, write_rip
  %pop
%endmacro

%macro read_rip_regs 1+
  read_rip_reg 8, 8, %1
  read_rip_reg 16, 16, %1
  read_rip_reg 32, 32, %1
  read_rip_reg 64, 64, %1
%endmacro

%macro write_rip_regs 1+
  write_rip_reg 8, 8, %1
  write_rip_reg 16, 16, %1
  write_rip_reg 32, 32, %1
  write_rip_reg 64, 64, %1
%endmacro

global mov_rip_fixture:data
mov_rip_fixture:
  read_rip_regs + 0
  read_rip_regs + 123
  read_rip_regs + 192919
  read_rip_regs - 1
  read_rip_regs - 9329188

  write_rip_regs + 0
  write_rip_regs + 123
  write_rip_regs + 192919
  write_rip_regs - 1
  write_rip_regs - 9329188

  simple_write_rip 64, 7, strict dword 1
  simple_write_rip 32, -8, strict dword 2
  simple_write_rip 16, 9, strict word 3
  simple_write_rip 8, -19, strict byte 5
