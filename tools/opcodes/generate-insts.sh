#!/bin/sh

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
PROJECT_DIR="$SCRIPT_DIR/../.."
TARGET_DIR="$PROJECT_DIR/src/lisp/assembly/amd64/inst/generated"

cd "$SCRIPT_DIR"
node generate.js generate "test" "$TARGET_DIR/test.hpp"
node generate.js generate bt "$TARGET_DIR/bt.hpp"
node generate.js generate cmp "$TARGET_DIR/cmp.hpp"

node generate.js generate add,sub,neg,mul,imul,div,idiv,cwd,cdq,cqo "$TARGET_DIR/arithmetic.hpp"
node generate.js generate mov,movsx,movsxd,movzx "$TARGET_DIR/mov.hpp"
node generate.js generate lea "$TARGET_DIR/lea.hpp"
node generate.js generate pop "$TARGET_DIR/pop.hpp"
node generate.js generate push "$TARGET_DIR/push.hpp"

node generate.js generate jmp,call "$TARGET_DIR/jmp.hpp"

node generate.js generate \
  jo,jno,jb,jae,je,jne,jbe,ja,js,jns,jp,jnp,jl,jge,jle,jg \
  "$TARGET_DIR/jcc.hpp"

node generate.js generate cmc "$TARGET_DIR/cmc.hpp"

node generate.js generate not,and,or,xor "$TARGET_DIR/binop.hpp"

node generate.js generate sal,sar,shl,shr,rcl,rcr,rol,ror,shld,shrd "$TARGET_DIR/shifts.hpp"

node generate.js generate movd,movq "$TARGET_DIR/fp_mov.hpp"

node generate.js generate \
     cvtsd2si,cvtsd2ss,cvtsi2sd,cvtsi2ss,cvtss2sd,cvtss2si \
     "$TARGET_DIR/fp_convert.hpp"

node generate.js generate ucomisd,ucomiss "$TARGET_DIR/fp_compare.hpp"

node generate.js generate subsd,subss,addsd,addss "$TARGET_DIR/fp_arithmetic.hpp"

node generate.js generate seto,setno,setc,setnc,setz,setnz,setna,seta,sets,setns,setp,setnp,setl,setnl,setle,setnle "$TARGET_DIR/setcc.hpp"

node generate.js generate cmovo,cmovno,cmovc,cmovnc,cmovz,cmovnz,cmovna,cmova,cmovs,cmovns,cmovp,cmovnp,cmovl,cmovnl,cmovle,cmovnle "$TARGET_DIR/cmovcc.hpp"
node generate.js generate xchg "$TARGET_DIR/xchg.hpp"

node generate.js generate std,cld,movsb,movsw,movsd,movsq,lodsb,lodsw,lodsd,lodsq,stosb,stosw,stosd,stosq "$TARGET_DIR/strings.hpp"

node generate.js generate int3 "$TARGET_DIR/debug.hpp"
