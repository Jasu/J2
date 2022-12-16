bits 64

%include "macros.mac"

%macro do_shift 3.nolist
  inst  %1, %2, %3
  db    1
  align 16, db 0
%endmacro

%macro do_mem_shift 4.nolist
  inst  %4 [%1], %2, %3
  db    1
  align 16, db 0
%endmacro

%macro do_shifts 4.nolist
  with_regs %2, do_shift, %1, %3
  with_regs 64, do_mem_shift, %1, %3, %4
%endmacro

%macro shifts 2.nolist
  with_regs_2 %1, do_shifts, %1, 3, %2
  with_regs_2 %1, do_shifts, %1, cl, %2
%endmacro


%macro create_fixture 1
  %xdefine inst %1
  global %1_fixture:data
  %1_fixture:
    shifts 16, WORD
    shifts 32, DWORD
    shifts 64, QWORD
%endmacro

create_fixture shld
create_fixture shrd
