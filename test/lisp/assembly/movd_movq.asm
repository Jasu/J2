bits 64

%include "macros.mac"

%macro do_movd 2+
  movd   %1, %2
  db 1
  align 16, db 0
  movd   %2, %1
  db 1
  align 16, db 0
%endmacro

%macro do_movq 2+
  movq   %1, %2
  db 1
  align 16, db 0
  movq   %2, %1
  db 1
  align 16, db 0
%endmacro

%macro do_movd_mem 2+
  movd   %1, DWORD [%2]
  db 1
  align 16, db 0
  movd   DWORD [%2], %1
  db 1
  align 16, db 0
%endmacro

%macro do_movq_mem 2+
  movq   %1, QWORD [%2]
  db 1
  align 16, db 0
  movq   QWORD [%2], %1
  db 1
  align 16, db 0
%endmacro

%macro do_movds 1
  with_xmm_regs do_movd, %1
%endmacro
%macro do_movqs 1
  with_xmm_regs do_movq, %1
%endmacro

%macro do_movd_mems 1
  with_xmm_regs do_movd_mem, %1
%endmacro
%macro do_movq_mems 1
  with_xmm_regs do_movq_mem, %1
%endmacro

global movd_fixture:data
movd_fixture:
  with_regs 32, do_movds
  with_regs 64, do_movd_mems

global movq_fixture:data
movq_fixture:
  with_regs 64, do_movqs
  with_regs 64, do_movq_mems
