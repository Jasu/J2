bits 64

%include "macros.mac"

global push_r64_fixture:data
global push_m64_fixture:data
global push_r16_fixture:data
global push_m16_fixture:data
global push_imm_fixture:data

%macro do 1.nolist
  push %1
  db 1
  align 16, db 0
%endmacro

%macro do_mem 2.nolist
  push  %2 [%1]
  db 1
  align 16, db 0
%endmacro



push_r64_fixture:
  with_regs 64, do

push_m64_fixture:
  with_regs 64, do_mem, strict qword

push_r16_fixture:
  with_regs 16, do

push_m16_fixture:
  with_regs 64, do_mem, strict word

push_imm_fixture:
  do strict BYTE 0
  do strict BYTE -0x80
  do strict BYTE 0x7F

  do strict WORD 0
  do strict WORD -0x8000
  do strict WORD 0x7FFF

  do strict DWORD 0
  do strict DWORD -0x80000000
  do strict DWORD 0x7FFFFFFF
