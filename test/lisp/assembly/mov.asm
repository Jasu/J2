bits 64

%macro do 2.nolist
  mov   %$lhs(%1), %$rhs(%2)
  db    1
  align 16, db 0
%endmacro

%macro with_regs_rhs_64 1.nolist
  do %1, rax
  do %1, rcx
  do %1, rdx
  do %1, rbx
  do %1, rsp
  do %1, rbp
  do %1, rsi
  do %1, rdi
  do %1, r8
  do %1, r9
  do %1, r10
  do %1, r11
  do %1, r12
  do %1, r13
  do %1, r14
  do %1, r15
%endmacro

%macro with_regs_rhs_32 1.nolist
  do %1, eax
  do %1, ecx
  do %1, edx
  do %1, ebx
  do %1, esp
  do %1, ebp
  do %1, esi
  do %1, edi
  do %1, r8d
  do %1, r9d
  do %1, r10d
  do %1, r11d
  do %1, r12d
  do %1, r13d
  do %1, r14d
  do %1, r15d
%endmacro

%macro with_regs_rhs_16 1.nolist
  do %1, ax
  do %1, cx
  do %1, dx
  do %1, bx
  do %1, sp
  do %1, bp
  do %1, si
  do %1, di
  do %1, r8w
  do %1, r9w
  do %1, r10w
  do %1, r11w
  do %1, r12w
  do %1, r13w
  do %1, r14w
  do %1, r15w
%endmacro

%macro with_regs_rhs_8 1.nolist
  do %1, al
  do %1, cl
  do %1, dl
  do %1, bl
  do %1, spl
  do %1, bpl
  do %1, sil
  do %1, dil
  do %1, r8b
  do %1, r9b
  do %1, r10b
  do %1, r11b
  do %1, r12b
  do %1, r13b
  do %1, r14b
  do %1, r15b
%endmacro

%macro with_regs_lhs_64_64 0.nolist
  with_regs_rhs_64 rax
  with_regs_rhs_64 rcx
  with_regs_rhs_64 rdx
  with_regs_rhs_64 rbx
  with_regs_rhs_64 rsp
  with_regs_rhs_64 rbp
  with_regs_rhs_64 rsi
  with_regs_rhs_64 rdi
  with_regs_rhs_64 r8
  with_regs_rhs_64 r9
  with_regs_rhs_64 r10
  with_regs_rhs_64 r11
  with_regs_rhs_64 r12
  with_regs_rhs_64 r13
  with_regs_rhs_64 r14
  with_regs_rhs_64 r15
%endmacro

%macro with_regs_lhs_64_32 0.nolist
  with_regs_rhs_32 rax
  with_regs_rhs_32 rcx
  with_regs_rhs_32 rdx
  with_regs_rhs_32 rbx
  with_regs_rhs_32 rsp
  with_regs_rhs_32 rbp
  with_regs_rhs_32 rsi
  with_regs_rhs_32 rdi
  with_regs_rhs_32 r8
  with_regs_rhs_32 r9
  with_regs_rhs_32 r10
  with_regs_rhs_32 r11
  with_regs_rhs_32 r12
  with_regs_rhs_32 r13
  with_regs_rhs_32 r14
  with_regs_rhs_32 r15
%endmacro

%macro with_regs_lhs_64_16 0.nolist
  with_regs_rhs_16 rax
  with_regs_rhs_16 rcx
  with_regs_rhs_16 rdx
  with_regs_rhs_16 rbx
  with_regs_rhs_16 rsp
  with_regs_rhs_16 rbp
  with_regs_rhs_16 rsi
  with_regs_rhs_16 rdi
  with_regs_rhs_16 r8
  with_regs_rhs_16 r9
  with_regs_rhs_16 r10
  with_regs_rhs_16 r11
  with_regs_rhs_16 r12
  with_regs_rhs_16 r13
  with_regs_rhs_16 r14
  with_regs_rhs_16 r15
%endmacro

%macro with_regs_lhs_64_8 0.nolist
  with_regs_rhs_8 rax
  with_regs_rhs_8 rcx
  with_regs_rhs_8 rdx
  with_regs_rhs_8 rbx
  with_regs_rhs_8 rsp
  with_regs_rhs_8 rbp
  with_regs_rhs_8 rsi
  with_regs_rhs_8 rdi
  with_regs_rhs_8 r8
  with_regs_rhs_8 r9
  with_regs_rhs_8 r10
  with_regs_rhs_8 r11
  with_regs_rhs_8 r12
  with_regs_rhs_8 r13
  with_regs_rhs_8 r14
  with_regs_rhs_8 r15
%endmacro

%macro with_regs_lhs_32_64 0.nolist
  with_regs_rhs_64 eax
  with_regs_rhs_64 ecx
  with_regs_rhs_64 edx
  with_regs_rhs_64 ebx
  with_regs_rhs_64 esp
  with_regs_rhs_64 ebp
  with_regs_rhs_64 esi
  with_regs_rhs_64 edi
  with_regs_rhs_64 r8d
  with_regs_rhs_64 r9d
  with_regs_rhs_64 r10d
  with_regs_rhs_64 r11d
  with_regs_rhs_64 r12d
  with_regs_rhs_64 r13d
  with_regs_rhs_64 r14d
  with_regs_rhs_64 r15d
%endmacro

%macro with_regs_lhs_16_64 0.nolist
  with_regs_rhs_64 ax
  with_regs_rhs_64 cx
  with_regs_rhs_64 dx
  with_regs_rhs_64 bx
  with_regs_rhs_64 sp
  with_regs_rhs_64 bp
  with_regs_rhs_64 si
  with_regs_rhs_64 di
  with_regs_rhs_64 r8w
  with_regs_rhs_64 r9w
  with_regs_rhs_64 r10w
  with_regs_rhs_64 r11w
  with_regs_rhs_64 r12w
  with_regs_rhs_64 r13w
  with_regs_rhs_64 r14w
  with_regs_rhs_64 r15w
%endmacro

%macro with_regs_lhs_8_64 0.nolist
  with_regs_rhs_64 al
  with_regs_rhs_64 cl
  with_regs_rhs_64 dl
  with_regs_rhs_64 bl
  with_regs_rhs_64 spl
  with_regs_rhs_64 bpl
  with_regs_rhs_64 sil
  with_regs_rhs_64 dil
  with_regs_rhs_64 r8b
  with_regs_rhs_64 r9b
  with_regs_rhs_64 r10b
  with_regs_rhs_64 r11b
  with_regs_rhs_64 r12b
  with_regs_rhs_64 r13b
  with_regs_rhs_64 r14b
  with_regs_rhs_64 r15b
%endmacro

%macro with_regs_lhs_32_32 0.nolist
  with_regs_rhs_32 eax
  with_regs_rhs_32 ecx
  with_regs_rhs_32 edx
  with_regs_rhs_32 ebx
  with_regs_rhs_32 esp
  with_regs_rhs_32 ebp
  with_regs_rhs_32 esi
  with_regs_rhs_32 edi
  with_regs_rhs_32 r8d
  with_regs_rhs_32 r9d
  with_regs_rhs_32 r10d
  with_regs_rhs_32 r11d
  with_regs_rhs_32 r12d
  with_regs_rhs_32 r13d
  with_regs_rhs_32 r14d
  with_regs_rhs_32 r15d
%endmacro

%macro with_regs_lhs_16_16 0.nolist
  with_regs_rhs_16 ax
  with_regs_rhs_16 cx
  with_regs_rhs_16 dx
  with_regs_rhs_16 bx
  with_regs_rhs_16 sp
  with_regs_rhs_16 bp
  with_regs_rhs_16 si
  with_regs_rhs_16 di
  with_regs_rhs_16 r8w
  with_regs_rhs_16 r9w
  with_regs_rhs_16 r10w
  with_regs_rhs_16 r11w
  with_regs_rhs_16 r12w
  with_regs_rhs_16 r13w
  with_regs_rhs_16 r14w
  with_regs_rhs_16 r15w
%endmacro

%macro with_regs_lhs_8_8 0.nolist
  with_regs_rhs_8 al
  with_regs_rhs_8 cl
  with_regs_rhs_8 dl
  with_regs_rhs_8 bl
  with_regs_rhs_8 spl
  with_regs_rhs_8 bpl
  with_regs_rhs_8 sil
  with_regs_rhs_8 dil
  with_regs_rhs_8 r8b
  with_regs_rhs_8 r9b
  with_regs_rhs_8 r10b
  with_regs_rhs_8 r11b
  with_regs_rhs_8 r12b
  with_regs_rhs_8 r13b
  with_regs_rhs_8 r14b
  with_regs_rhs_8 r15b
%endmacro

%macro mov_rr_64 0.nolist
  %push mov_rr
  %define %$lhs(a) a
  %define %$rhs(b) b
  with_regs_lhs_64_64
  %pop
%endmacro

%macro mov_rr_32 0.nolist
  %push mov_rr
  %define %$lhs(a) a
  %define %$rhs(b) b
  with_regs_lhs_32_32
  %pop
%endmacro

%macro mov_rr_16 0.nolist
  %push mov_rr
  %define %$lhs(a) a
  %define %$rhs(b) b
  with_regs_lhs_16_16
  %pop
%endmacro

%macro mov_rr_8 0.nolist
  %push mov_rr
  %define %$lhs(a) a
  %define %$rhs(b) b
  with_regs_lhs_8_8
  %pop
%endmacro

%macro mov_rm_direct_64 0.nolist
  %push mov_rm_direct
  %define %$lhs(a) a
  %define %$rhs(b) [b]
  with_regs_lhs_64_64
  %pop
%endmacro

%macro mov_rm_direct_32 0.nolist
  %push mov_rm_direct
  %define %$lhs(a) a
  %define %$rhs(b) [b]
  with_regs_lhs_32_64
  %pop
%endmacro

%macro mov_rm_direct_16 0.nolist
  %push mov_rm_direct
  %define %$lhs(a) a
  %define %$rhs(b) [b]
  with_regs_lhs_16_64
  %pop
%endmacro

%macro mov_rm_direct_8 0.nolist
  %push mov_rm_direct
  %define %$lhs(a) a
  %define %$rhs(b) [b]
  with_regs_lhs_8_64
  %pop
%endmacro

%macro mov_mr_direct_64 0.nolist
  %push mov_mr_direct
  %define %$lhs(a) [a]
  %define %$rhs(b) b
  with_regs_lhs_64_64
  %pop
%endmacro

%macro mov_mr_direct_32 0.nolist
  %push mov_mr_direct
  %define %$lhs(a) [a]
  %define %$rhs(b) b
  with_regs_lhs_64_32
  %pop
%endmacro

%macro mov_mr_direct_16 0.nolist
  %push mov_mr_direct
  %define %$lhs(a) [a]
  %define %$rhs(b) b
  with_regs_lhs_64_16
  %pop
%endmacro

%macro mov_mr_direct_8 0.nolist
  %push mov_mr_direct
  %define %$lhs(a) [a]
  %define %$rhs(b) b
  with_regs_lhs_64_8
  %pop
%endmacro

global mov_fixture:data
mov_fixture:
  mov_rr_64
  mov_rr_32
  mov_rr_16
  mov_rr_8

  mov_rm_direct_64
  mov_rm_direct_32
  mov_rm_direct_16
  mov_rm_direct_8

  mov_mr_direct_64
  mov_mr_direct_32
  mov_mr_direct_16
  mov_mr_direct_8
