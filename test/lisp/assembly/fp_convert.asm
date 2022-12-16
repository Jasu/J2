bits 64

%include "macros.mac"

%macro do_si 2
  cvtsi2ss %1, %2
  db 1
  align 16, db 0
  cvtss2si %2, %1
  db 1
  align 16, db 0
  cvtsi2sd %1, %2
  db 1
  align 16, db 0
  cvtsd2si %2, %1
  db 1
  align 16, db 0
%endmacro

%macro si 1
  with_xmm_regs do_si, %1
%endmacro

global si_fixture:data
si_fixture:
  with_regs 32, si

%macro do_sd_ss 2
  cvtsd2ss %1, %2
  db 1
  align 16, db 0
  cvtss2sd %1, %2
  db 1
  align 16, db 0
%endmacro

%macro sd_ss 1
  with_xmm_regs_2 do_sd_ss, %1
%endmacro

global sd_ss_fixture:data
sd_ss_fixture:
  with_xmm_regs sd_ss
