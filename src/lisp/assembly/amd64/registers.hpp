#pragma once

#include "lisp/common/mem_width.hpp"
#include "lisp/assembly/registers.hpp"

namespace j::lisp::assembly::amd64 {
  namespace preg {
    constexpr inline J_A(HIDDEN) phys_reg rax{0};
    constexpr inline J_A(HIDDEN) phys_reg rcx{1};
    constexpr inline J_A(HIDDEN) phys_reg rdx{2};
    constexpr inline J_A(HIDDEN) phys_reg rbx{3};
    constexpr inline J_A(HIDDEN) phys_reg rsp{4};
    constexpr inline J_A(HIDDEN) phys_reg rbp{5};
    constexpr inline J_A(HIDDEN) phys_reg rsi{6};
    constexpr inline J_A(HIDDEN) phys_reg rdi{7};
    constexpr inline J_A(HIDDEN) phys_reg r8 {8};
    constexpr inline J_A(HIDDEN) phys_reg r9 {9};
    constexpr inline J_A(HIDDEN) phys_reg r10{10};
    constexpr inline J_A(HIDDEN) phys_reg r11{11};
    constexpr inline J_A(HIDDEN) phys_reg r12{12};
    constexpr inline J_A(HIDDEN) phys_reg r13{13};
    constexpr inline J_A(HIDDEN) phys_reg r14{14};
    constexpr inline J_A(HIDDEN) phys_reg r15{15};

    constexpr inline J_A(HIDDEN) phys_reg ymm0 {16};
    constexpr inline J_A(HIDDEN) phys_reg ymm1 {17};
    constexpr inline J_A(HIDDEN) phys_reg ymm2 {18};
    constexpr inline J_A(HIDDEN) phys_reg ymm3 {19};
    constexpr inline J_A(HIDDEN) phys_reg ymm4 {20};
    constexpr inline J_A(HIDDEN) phys_reg ymm5 {21};
    constexpr inline J_A(HIDDEN) phys_reg ymm6 {22};
    constexpr inline J_A(HIDDEN) phys_reg ymm7 {23};
    constexpr inline J_A(HIDDEN) phys_reg ymm8 {24};
    constexpr inline J_A(HIDDEN) phys_reg ymm9 {25};
    constexpr inline J_A(HIDDEN) phys_reg ymm10{26};
    constexpr inline J_A(HIDDEN) phys_reg ymm11{27};
    constexpr inline J_A(HIDDEN) phys_reg ymm12{28};
    constexpr inline J_A(HIDDEN) phys_reg ymm13{29};
    constexpr inline J_A(HIDDEN) phys_reg ymm14{30};
    constexpr inline J_A(HIDDEN) phys_reg ymm15{31};
  }

  constexpr inline J_A(HIDDEN) reg rax = make_reg(0, mem_width::qword);
  constexpr inline J_A(HIDDEN) reg rcx = make_reg(1, mem_width::qword);
  constexpr inline J_A(HIDDEN) reg rdx = make_reg(2, mem_width::qword);
  constexpr inline J_A(HIDDEN) reg rbx = make_reg(3, mem_width::qword);
  constexpr inline J_A(HIDDEN) reg rsp = make_reg(4, mem_width::qword);
  constexpr inline J_A(HIDDEN) reg rbp = make_reg(5, mem_width::qword);
  constexpr inline J_A(HIDDEN) reg rsi = make_reg(6, mem_width::qword);
  constexpr inline J_A(HIDDEN) reg rdi = make_reg(7, mem_width::qword);
  constexpr inline J_A(HIDDEN) reg r8  = make_reg(8,  mem_width::qword);
  constexpr inline J_A(HIDDEN) reg r9  = make_reg(9,  mem_width::qword);
  constexpr inline J_A(HIDDEN) reg r10 = make_reg(10, mem_width::qword);
  constexpr inline J_A(HIDDEN) reg r11 = make_reg(11, mem_width::qword);
  constexpr inline J_A(HIDDEN) reg r12 = make_reg(12, mem_width::qword);
  constexpr inline J_A(HIDDEN) reg r13 = make_reg(13, mem_width::qword);
  constexpr inline J_A(HIDDEN) reg r14 = make_reg(14, mem_width::qword);
  constexpr inline J_A(HIDDEN) reg r15 = make_reg(15, mem_width::qword);

  constexpr inline J_A(HIDDEN) reg eax = make_reg(0U,  mem_width::dword);
  constexpr inline J_A(HIDDEN) reg ecx = make_reg(1U,  mem_width::dword);
  constexpr inline J_A(HIDDEN) reg edx = make_reg(2U,  mem_width::dword);
  constexpr inline J_A(HIDDEN) reg ebx = make_reg(3U,  mem_width::dword);
  constexpr inline J_A(HIDDEN) reg esp = make_reg(4U,  mem_width::dword);
  constexpr inline J_A(HIDDEN) reg ebp = make_reg(5U,  mem_width::dword);
  constexpr inline J_A(HIDDEN) reg esi = make_reg(6U,  mem_width::dword);
  constexpr inline J_A(HIDDEN) reg edi = make_reg(7U,  mem_width::dword);
  constexpr inline J_A(HIDDEN) reg r8d = make_reg(8U,  mem_width::dword);
  constexpr inline J_A(HIDDEN) reg r9d = make_reg(9U,  mem_width::dword);
  constexpr inline J_A(HIDDEN) reg r10d= make_reg(10U, mem_width::dword);
  constexpr inline J_A(HIDDEN) reg r11d= make_reg(11U, mem_width::dword);
  constexpr inline J_A(HIDDEN) reg r12d= make_reg(12U, mem_width::dword);
  constexpr inline J_A(HIDDEN) reg r13d= make_reg(13U, mem_width::dword);
  constexpr inline J_A(HIDDEN) reg r14d= make_reg(14U, mem_width::dword);
  constexpr inline J_A(HIDDEN) reg r15d= make_reg(15U, mem_width::dword);

  constexpr inline J_A(HIDDEN) reg ax   = make_reg(0U,  mem_width::word);
  constexpr inline J_A(HIDDEN) reg cx   = make_reg(1U,  mem_width::word);
  constexpr inline J_A(HIDDEN) reg dx   = make_reg(2U,  mem_width::word);
  constexpr inline J_A(HIDDEN) reg bx   = make_reg(3U,  mem_width::word);
  constexpr inline J_A(HIDDEN) reg sp   = make_reg(4U,  mem_width::word);
  constexpr inline J_A(HIDDEN) reg bp   = make_reg(5U,  mem_width::word);
  constexpr inline J_A(HIDDEN) reg si   = make_reg(6U,  mem_width::word);
  constexpr inline J_A(HIDDEN) reg di   = make_reg(7U,  mem_width::word);
  constexpr inline J_A(HIDDEN) reg r8w  = make_reg(8U,  mem_width::word);
  constexpr inline J_A(HIDDEN) reg r9w  = make_reg(9U,  mem_width::word);
  constexpr inline J_A(HIDDEN) reg r10w = make_reg(10U, mem_width::word);
  constexpr inline J_A(HIDDEN) reg r11w = make_reg(11U, mem_width::word);
  constexpr inline J_A(HIDDEN) reg r12w = make_reg(12U, mem_width::word);
  constexpr inline J_A(HIDDEN) reg r13w = make_reg(13U, mem_width::word);
  constexpr inline J_A(HIDDEN) reg r14w = make_reg(14U, mem_width::word);
  constexpr inline J_A(HIDDEN) reg r15w = make_reg(15U, mem_width::word);

  constexpr inline J_A(HIDDEN) reg al   = make_reg(0U,  mem_width::byte);
  constexpr inline J_A(HIDDEN) reg cl   = make_reg(1U,  mem_width::byte);
  constexpr inline J_A(HIDDEN) reg dl   = make_reg(2U,  mem_width::byte);
  constexpr inline J_A(HIDDEN) reg bl   = make_reg(3U,  mem_width::byte);
  constexpr inline J_A(HIDDEN) reg spl  = make_reg(4U,  mem_width::byte);
  constexpr inline J_A(HIDDEN) reg bpl  = make_reg(5U,  mem_width::byte);
  constexpr inline J_A(HIDDEN) reg sil  = make_reg(6U,  mem_width::byte);
  constexpr inline J_A(HIDDEN) reg dil  = make_reg(7U,  mem_width::byte);
  constexpr inline J_A(HIDDEN) reg r8b  = make_reg(8U,  mem_width::byte);
  constexpr inline J_A(HIDDEN) reg r9b  = make_reg(9U,  mem_width::byte);
  constexpr inline J_A(HIDDEN) reg r10b = make_reg(10U, mem_width::byte);
  constexpr inline J_A(HIDDEN) reg r11b = make_reg(11U, mem_width::byte);
  constexpr inline J_A(HIDDEN) reg r12b = make_reg(12U, mem_width::byte);
  constexpr inline J_A(HIDDEN) reg r13b = make_reg(13U, mem_width::byte);
  constexpr inline J_A(HIDDEN) reg r14b = make_reg(14U, mem_width::byte);
  constexpr inline J_A(HIDDEN) reg r15b = make_reg(15U, mem_width::byte);

  constexpr inline J_A(HIDDEN) reg ymm0  = make_reg(0,  mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm1  = make_reg(1,  mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm2  = make_reg(2,  mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm3  = make_reg(3,  mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm4  = make_reg(4,  mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm5  = make_reg(5,  mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm6  = make_reg(6,  mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm7  = make_reg(7,  mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm8  = make_reg(8,  mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm9  = make_reg(9,  mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm10 = make_reg(10, mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm11 = make_reg(11, mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm12 = make_reg(12, mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm13 = make_reg(13, mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm14 = make_reg(14, mem_width::vec256);
  constexpr inline J_A(HIDDEN) reg ymm15 = make_reg(15, mem_width::vec256);

  constexpr inline J_A(HIDDEN) reg xmm0  = make_reg(0,  mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm1  = make_reg(1,  mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm2  = make_reg(2,  mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm3  = make_reg(3,  mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm4  = make_reg(4,  mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm5  = make_reg(5,  mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm6  = make_reg(6,  mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm7  = make_reg(7,  mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm8  = make_reg(8,  mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm9  = make_reg(9,  mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm10 = make_reg(10, mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm11 = make_reg(11, mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm12 = make_reg(12, mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm13 = make_reg(13, mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm14 = make_reg(14, mem_width::vec128);
  constexpr inline J_A(HIDDEN) reg xmm15 = make_reg(15, mem_width::vec128);
}
