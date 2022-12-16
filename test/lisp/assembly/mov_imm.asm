bits 64

%include "macros.mac"

%macro do 2.nolist
  mov   %$lhs(%1), %$rhs(%2)
  db    1
  align 16, db 0
%endmacro

%macro mov_ri 3
  %push mov_ri
  %define %$lhs(a) strict a
  %define %$rhs(a) strict width(%2) a
  with_regs %1, do, %3
  %pop
%endmacro

%macro mov_mi 3
  %push mov_mi
  %define %$lhs(a) strict width(%1) [a]
  %define %$rhs(a) strict width(%2) a
  with_regs 64, do, %3
  %pop
%endmacro

%macro mov_mi_disp 4+
  %push mov_mi
  %define %$lhs(a) strict width(%1) [a + %4]
  %define %$rhs(a) strict width(%2) a
  with_regs 64, do, %3
  %pop
%endmacro

%macro do_mov_mi_index 5+
  mov_mi_disp %2, %3, %4, %5 %1
%endmacro

%macro mov_mi_index 3-4+
  %assign scale 1
  %rep 4
    with_index_regs 64, do_mov_mi_index, %1, %2, %3, %4 scale *
    %assign scale 2*scale
  %endrep
%endmacro

global mov_imm_fixture:data
mov_imm_fixture:
  mov_ri 64, 64, 1
  mov_ri 64, 32, 1
  mov_ri 32, 32, 1
  mov_ri 16, 16, 1
  mov_ri 8, 8, 1

  mov_mi 64, 32, 1
  mov_mi 32, 32, 1
  mov_mi 16, 16, 1
  mov_mi 8, 8, 1

  mov_mi_disp 64, 32, 1, 123
  mov_mi_disp 32, 32, 1, 123
  mov_mi_disp 16, 16, 1, 123
  mov_mi_disp 8, 8, 1, 123

  mov_mi_disp 64, 32, 1, 991291
  mov_mi_disp 32, 32, 1, 991291
  mov_mi_disp 16, 16, 1, 991291
  mov_mi_disp 8, 8, 1, 991291

  mov_mi_index 8, 8, 1
  mov_mi_index 16, 16, 1
  mov_mi_index 32, 32, 1
  mov_mi_index 64, 32, 1

  mov_mi_index 8, 8, 1, 123 +
  mov_mi_index 16, 16, 1, 123 +
  mov_mi_index 32, 32, 1, 123 +
  mov_mi_index 64, 32, 1, 123 +

  mov_mi_index 8, 8, 1, 1219919 +
  mov_mi_index 16, 16, 1, 1219919 +
  mov_mi_index 32, 32, 1, 1219919 +
  mov_mi_index 64, 32, 1, 1219919 +

  mov_mi_index 8, 8, 1, 127 +
  mov_mi_index 16, 16, 1, 127 +
  mov_mi_index 32, 32, 1, 127 +
  mov_mi_index 64, 32, 1, 127 +

  mov_mi_index 8, 8, 1, -128 +
  mov_mi_index 16, 16, 1, -128 +
  mov_mi_index 32, 32, 1, -128 +
  mov_mi_index 64, 32, 1, -128 +

  mov_mi_index 8, 8, 1, -129 +
  mov_mi_index 16, 16, 1, -129 +
  mov_mi_index 32, 32, 1, -129 +
  mov_mi_index 64, 32, 1, -129 +

  mov_mi_index 8, 8, 1, 128 +
  mov_mi_index 16, 16, 1, 128 +
  mov_mi_index 32, 32, 1, 128 +
	mov_mi_index 64, 32, 1, 128 +
