bits 64

global and_ri_fixture:data
global and_mi_fixture:data
global and_rax_fixture:data
global and_mr_fixture:data

global or_ri_fixture:data
global or_mi_fixture:data
global or_rax_fixture:data
global or_mr_fixture:data

global xor_ri_fixture:data
global xor_mi_fixture:data
global xor_rax_fixture:data
global xor_mr_fixture:data

global add_ri_fixture:data
global add_mi_fixture:data
global add_rax_fixture:data
global add_mr_fixture:data

global sub_ri_fixture:data
global sub_mi_fixture:data
global sub_rax_fixture:data
global sub_mr_fixture:data

global cmp_ri_fixture:data
global cmp_mi_fixture:data
global cmp_rax_fixture:data
global cmp_mr_fixture:data

global test_ri_fixture:data
global test_mi_fixture:data
global test_rax_fixture:data
global test_mr_fixture:data

%include "macros.mac"

%macro do 2.nolist
  do_op   %1, %2
  db 1
  align 16, db 0
%endmacro

%macro do_mi 3+.nolist
  do_op   %3 [%1], %2
  db 1
  align 16, db 0
%endmacro

%macro do_mr 2-3+.nolist
  do_op  [%1 %3], %2
  db 1
  align 16, db 0

  %ifnidn do_op, test
    do_op  %2, [%1 %3]
    db 1
    align 16, db 0
  %else
    times 16 db 1
  %endif
%endmacro

%macro with_ri 2.nolist
  with_regs 8,  %1, strict BYTE %2
  with_regs 16, %1, strict WORD %2
  with_regs 32, %1, strict DWORD %2
  with_regs 64, %1, strict QWORD %2
%endmacro

%macro with_mr_base 1-2+.nolist
  %if %0 > 1
    with_regs 64, do_mr, %1, %2
  %else
    with_regs 64, do_mr, %1
  %endif
%endmacro

%macro with_mr_srcs 0-1+.nolist
  %if %0 > 0
    with_regs_2 8, with_mr_base, %1
    with_regs_2 16, with_mr_base, %1
    with_regs_2 32, with_mr_base, %1
    with_regs_2 64, with_mr_base, %1
  %else
    with_regs_2 8, with_mr_base
    with_regs_2 16, with_mr_base
    with_regs_2 32, with_mr_base
    with_regs_2 64, with_mr_base
  %endif
%endmacro

%macro create_fixture 1
  %xdefine do_op %1
  %1_ri_fixture:
    with_ri do, 0
    with_ri do, 1
    with_ri do, -1

  %1_mi_fixture:
    with_regs 64, do_mi, strict BYTE 1, strict BYTE
    with_regs 64, do_mi, strict WORD 1, strict WORD
    with_regs 64, do_mi, strict DWORD 1, strict DWORD
    with_regs 64, do_mi, strict QWORD 1, strict QWORD

  %1_mr_fixture:
    with_mr_srcs
    with_mr_srcs + 718
    with_mr_srcs + 2*rax

  %1_rax_fixture:
    do al, strict BYTE 0
    do al, strict BYTE 1
    do al, strict BYTE -1
    do al, strict BYTE 0x7F
    do al, strict BYTE -0x80

    do ax, strict WORD 0
    do ax, strict WORD 1
    do ax, strict WORD -1
    do ax, strict WORD 0x7FFF
    do ax, strict WORD -0x8000

    do eax, strict DWORD 0
    do eax, strict DWORD 1
    do eax, strict DWORD -1
    do eax, strict DWORD 0x7FFFFFFF
    do eax, strict DWORD -0x80000000

    do rax, strict QWORD 0
    do rax, strict QWORD 1
    do rax, strict QWORD -1
    do rax, strict QWORD 0x7FFFFFFF
    do rax, strict QWORD -0x80000000
%endmacro

create_fixture add
create_fixture sub
create_fixture cmp
create_fixture and
create_fixture or
create_fixture xor
create_fixture test
