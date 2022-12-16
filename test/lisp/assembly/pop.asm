bits 64

%include "macros.mac"

global pop_r64_fixture:data
global pop_m64_fixture:data
global pop_r16_fixture:data
global pop_m16_fixture:data

%macro do 1.nolist
  pop %1
  db 1
  align 16, db 0
%endmacro

%macro do_mem 2.nolist
  pop  %2 [%1]
  db 1
  align 16, db 0
%endmacro



pop_r64_fixture:
  with_regs 64, do

pop_m64_fixture:
  with_regs 64, do_mem, strict qword

pop_r16_fixture:
  with_regs 16, do

pop_m16_fixture:
  with_regs 64, do_mem, strict word
