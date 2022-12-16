bits 64

%include "macros.mac"

%macro do 2
  inst %1, %2
  db 1
  align 16, db 0
%endmacro

%macro do_xmms 2
  with_xmm_regs_2 %2, %1
%endmacro

%macro do_xmms_mem 3
  with_xmm_regs_2 %2, %3 [%1]
%endmacro

%macro do_mem 2
  with_regs 64, do_xmms_mem, %1, %2
%endmacro

%macro make_fixture 2
  %xdefine inst %1
  global %1_fixture:data
  %1_fixture:
    with_xmm_regs do_xmms, do
    do_mem do, %2
%endmacro

make_fixture ucomiss, DWORD
make_fixture ucomisd, QWORD
make_fixture subss, DWORD
make_fixture subsd, QWORD
make_fixture addss, DWORD
make_fixture addsd, QWORD
