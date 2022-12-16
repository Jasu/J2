bits 64

%include "macros.mac"

%macro do_lea 2+
  lea   %1, [%2]
  db 1
  align 16, db 0
%endmacro

%macro do_lea_with_target 2-3+
  with_regs_2 %2, do_lea, %1 %3
%endmacro

%macro do_lea_with_base 2+
  with_regs %1, do_lea_with_target, %2
%endmacro

global lea_fixture:data
lea_fixture:
  do_lea_with_base 64, 64, + 10
  do_lea_with_base 64, 64, + rbx
  do_lea_with_base 64, 64, + 2*rax
  do_lea_with_base 64, 64, + 4*rbp + 81
  do_lea_with_base 64, 64, + 8*r15 - 1111

  do_lea_with_base 64, 32, + 10
  do_lea_with_base 64, 32, + rbx
  do_lea_with_base 64, 32, + 2*rax
  do_lea_with_base 64, 32, + 4*rbp + 81
  do_lea_with_base 64, 32, + 8*r15 - 1111
