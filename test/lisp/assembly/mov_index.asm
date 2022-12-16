bits 64

%include "macros.mac"

%macro do_mov_mr_index_with_src 2
  mov   %2, %1
  db 1
  align 16, db 0
%endmacro

%macro do_mov_rm_index_with_src 2
  mov   %1, %2
  db    1
  align 16, db 0
%endmacro

%macro do_mov_mr_index_with_base 4
  with_regs_2 %3, do_mov_mr_index_with_src, strict width(%2) [%1 + %4]
%endmacro

%macro do_mov_rm_index_with_base 4
  with_regs_2 %3, do_mov_rm_index_with_src, strict width(%2) [%1 + %4]
%endmacro

%macro do_mov_mr_index 4-5+
  with_regs 64, do_mov_mr_index_with_base, %3, %4, %2*%1 %5
%endmacro

%macro do_mov_rm_index 4-5+
  with_regs 64, do_mov_rm_index_with_base, %3, %4, %2*%1 %5
%endmacro

%macro with_scaled_index_regs 2+
  %assign scale 1
  %rep 4
    with_index_regs 64, %1, scale, %2
    %assign scale 2*scale
  %endrep
%endmacro

global mov_index_fixture:data
mov_index_fixture:
  with_scaled_index_regs do_mov_mr_index, 8, 8
  with_scaled_index_regs do_mov_rm_index, 8, 8, + 128
  with_scaled_index_regs do_mov_mr_index, 16, 16, - 129
  with_scaled_index_regs do_mov_rm_index, 16, 16
  with_scaled_index_regs do_mov_mr_index, 32, 32, - 128
  with_scaled_index_regs do_mov_rm_index, 32, 32
  with_scaled_index_regs do_mov_mr_index, 64, 64
  with_scaled_index_regs do_mov_rm_index, 64, 64, + 127
