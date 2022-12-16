const asmdb = require('asmdb');
const fs = require('fs/promises');
const { strict: assert } = require('assert');

const isa = new asmdb.x86.ISA();

function fail() {
  console.error(`Usage: ${process.argv[0]} ${process.argv[1]} [generate|dump|list] INSTRUCTION_NAMES OUTFILE.HPP`);
  process.exit(1);
}

if (process.argv.length < 3 || process.argv.length > 5) {
  fail();
}

const mode = process.argv[2];
const instructionNames = mode === 'list' ? null : process.argv[3].split(',');
switch (mode) {
  case 'list':
    if (process.argv.length !== 3) { fail(); }
    break;
  case 'dump':
    if (process.argv.length !== 4) { fail(); }
    break;
  case 'generate':
    if (process.argv.length !== 5) { fail(); }
    break;
  default:
    fail();
}

const allowedExtensions = new Set([
  'X64',
  'CMOV', 'I486', 'MOVBE',
  'ADX',
  'MMX', 'MMX2', 'AVX', 'AVX2',
  'SSE', 'SSE2', 'SSE3', 'SSE4_1', 'SSE4_2', 'SSE4A',
  'BMI', 'BMI2',
  'LZCNT', 'POPCNT',
  'PREFETCHW', 'PREFETCHWT1',
  'RDTSC', 'RDTSCP',
]);

const disallowedRegs = new Set([
  'sreg', 'dreg', 'creg', 'gs', 'fs',
  'mm',
]);


const filterInsts = (insts) => insts.filter(
  (inst) => (
    inst.privilege === 'L3'
    && inst.arch !== 'X86'
    && !inst.operands.some(op => (op.memOff || disallowedRegs.has(op.reg)))
    && Object.keys(inst.extensions).every(ext => allowedExtensions.has(ext))
    // mul edx:eax, r/m16 etc. not yet supported
    // && (inst.encoding !== 'M' || inst.implicit !== 3)
    // For some reason, the data has AND EAX, IMM32 listed twice, once as
    // setting RAX to a zero-extended IMM32 argument (which it of course does).
    // This confuses code in some places.
    && (inst.opcodeString !== '25 id' || inst.operands[0].data !== 'rax')
    && (inst.opcodeString !== '81 /4 id' || inst.operands[0].data !== 'r64')));

const outInsts = [];
isa.forEachGroup((name, insts) => {
  if (mode != 'list' && !instructionNames.includes(name)) {
    return;
  }
  insts = filterInsts(insts);
  if (insts.length) {
    if (mode === 'list') {
      console.log(name);
      return;
    }
    insts.sort(encodingCompare);
    outInsts.push({ name, insts });
  }
});

if (mode === 'list') {
  process.exit(0);
}


function bitScore(b) {
  switch (b) {
    case 64: return -3;
    case 32: return -2;
    case 8: return -1;
    case 16: return 1;
    default: return 0;
  }
}
function encodingCompare(lhs, rhs) {
  if (lhs.ri !== rhs.ri) {
    return rhs.ri - lhs.ri;
  }
  if ((lhs.pp === '') !== (rhs.pp == '')) {
    return (rhs.pp === '') - (lhs.pp === '');
  }
  if ((lhs.w === '') != (rhs.w === '')) {
    return (rhs.w === '') - (lhs.w === '');
  }
  const lhsBits = bitScore(Math.max(...lhs.operands.map(o => (o.reg !== '' ? o.regSize : 0))));
  const rhsBits = bitScore(Math.max(...rhs.operands.map(o => (o.reg !== '' ? o.regSize : 0))));
  if (lhsBits != rhsBits) {
    return lhsBits - rhsBits;
  }
  return 0;
}

const skippedOpcodeBytes = new Set([
  // Prefixes
  '66', 'F2', 'F3',
  'REX.W',

  // ModRM
  '/r',
  '/0', '/1', '/2', '/3', '/4', '/5', '/6', '/7',

  // Immediates
  'ib', 'iw', 'id', 'iq',
  // Offsets
  'cb', 'cw', 'cd', 'cq',
]);

function opcodeBytes(inst) {
  return inst.opcodeString
    .split(' ')
    .filter(b => !skippedOpcodeBytes.has(b))
    .map(b => b.substr(0, 2));
}

function hasImplicitRegister(inst) {
  return inst.operands.some(o => ['rax', 'eax', 'ax', 'al',
                                  'rcx', 'ecx', 'cx', 'cl'].includes(o.reg));
}
function implicitIndex(inst) {
  return inst.operands.findIndex(o => ['rdx', 'edx', 'dx', 'dl', 'rax', 'eax', 'ax', 'al', 'rcx', 'ecx', 'cx', 'cl'].includes(o.reg));
}
function getOperandFormat(inst) {
  const implicit = hasImplicitRegister(inst);
  const ri = inst.ri;
  const modrm = ['MI', 'M', 'R', 'MR', 'RM', 'MRI', 'RMI'].includes(inst.encoding);
  if (implicit && modrm) {
    const idx = implicitIndex(inst);
    if (inst.encoding === 'M') {
      if (inst.implicit == 3) {
        return 'implicit2_m';
      }
      assert(idx === 0 || idx === 1);
      // SHL RAX, CL
      return idx ? 'm' : 'implicit_m';
    }
    if (inst.encoding === 'MR' || inst.encoding === 'MRC') {
      assert(idx === 2);
      // SHLD RAX, RCX, CL
      return 'mr_implicit';
    }
    throw new Error(`Unspported encoding with implicit: '${inst.encoding}'`);
  }
  if (implicit) {
    if (ri) {
      const idx = implicitIndex(inst);
      assert(idx === 0 || idx === 1);
      return idx ? 'ri_implicit' : 'implicit_ri';
    }
    return 'implicit';
  }
  if (ri) {
    return 'ri';
  }
  if (modrm) {
    if (inst.encoding === 'MI') {
      return 'm';
    }
    if (inst.encoding === 'MRI') {
      return 'mr';
    }
    if (inst.encoding === 'RMI') {
      return 'rm';
    }
    return inst.encoding.toLowerCase();
  }
  if (inst.modR) {
    return 'm';
  }
  return 'none';
}
function getOperandTypeMask(operand) {
  if (operand.imm) {
    return `operand_type_mask::imm${operand.imm}`;
  } else if (operand.rel) {
    return `operand_type_mask::imm${operand.rel}`;
  } else {
    switch (operand.data) {
      case 'ds:zsi': return 'special_case_any_rsi';
      case 'es:zdi': return 'special_case_any_rdi';
    case 'rdx': case 'edx': case 'dx': case 'dl':
    case 'rax': case 'eax': case 'ax': case 'al':
        return `special_case_${operand.data}`;
    case 'cl':
      return 'special_case_any_rcx';

    case 'mem': // LEA operand (memory width does not matter)
    case 'r8':
    case 'r16':
    case 'r32':
    case 'r64':
    case 'm8':
    case 'm16':
    case 'm32':
    case 'm64':
      return `operand_type_mask::${operand.data}`;
    case 'r8/m8':
    case 'r16/m16':
    case 'r32/m32':
    case 'r64/m64':
      return `operand_type_mask::rm${operand.memSize}`;
    case 'xmm':
      return 'operand_type_mask::xmm';
    case 'ymm':
      return 'operand_type_mask::ymm';
    case 'xmm/m32':
      return 'operand_type_mask::xmm_m32';
    case 'xmm/m64':
      return 'operand_type_mask::xmm_m64';
    default:
      console.log(operand);
      throw new Error(`Unexpected operand ${operand.data}`);
    }
  }
}

function getOperandTypes(operands) {
  return operands.map(getOperandTypeMask);
}

function getImmediateMode(operands) {
  const immOp = operands.find((operand) => (operand.imm && operand.data !== '1')|| operand.rel);
  if (!immOp) {
    return null;
  }
  return immOp.imm ? `imm${immOp.imm}` : `rel${immOp.rel}`;
}

async function writeEncoding(f, inst) {
  await f.write("      {\n");
  if (inst.operands.some(o => o.data === '1')) {
    await f.write(`        .mask = make_shift_by_one_mask(${getOperandTypeMask(inst.operands[0])}),\n`);
  } else {
    await f.write(`        .mask = operand_mask(${getOperandTypes(inst.operands).join(', ')}),\n`);
  }
  let isFirst = true;
  if (inst.pp !== '') {
    await f.write(`        .mandatory_prefix = mandatory_prefix::has_${inst.pp.toLowerCase()},\n`);
  }
  if (inst.prefix === '' && inst.w === 'W1') {
    await f.write("        .has_rex_w = true,\n");
  }
  await f.write(`        .operand_format = operand_format::${getOperandFormat(inst)},\n`);

  const bytes = opcodeBytes(inst);
  if (inst.ri) {
    await f.write(`        .opcode_format = opcode_format::plus_reg_${bytes.length},\n`);
  } else {
    await f.write(`        .opcode_format = opcode_format::plain_${bytes.length},\n`);
  }
  if (inst.modR !== '' && inst.modR !== 'r') {
    await f.write(`        .modrm_forced_r = ${inst.modR},\n`);
  }
  const imm = getImmediateMode(inst.operands);
  if (imm) {
    await f.write(`        .imm_format = imm_format::${imm},\n`);
  }

  await f.write('        .opcode = {');
  for (const b of bytes) {
    await f.write(`0x${b}U, `);
  }
  for (let i = bytes.length; i < 3; ++i) {
    await f.write('0x00U, ');
  }
  await f.write("0x00U},\n");
  await f.write(`        .name = "${inst.opcodeString}",\n`);
  await f.write("      },\n");
}

async function writeInstruction(f, name, insts) {
  let escapedName = name;
  switch (name) {
    case 'not': escapedName = 'bnot'; break;
    case 'and': escapedName = 'band'; break;
    case 'or': escapedName = 'bor'; break;
    case 'xor': escapedName = 'bxor'; break;
  }
  await f.write(`  constexpr inline instruction<${insts.length}> ${escapedName} = {\n`);
  await f.write(`    .name      = "${name}",\n`);
  await f.write("    .encodings = {\n");
  for (const enc of insts) {
    await writeEncoding(f, enc);
  }
  await f.write("      instruction_encoding{},\n");
  await f.write("    },\n");
  await f.write("  };\n");
}

async function generate(fileName)  {
  const f = await fs.open(fileName, 'w');

  await f.write("#pragma once\n");
  await f.write("// GENERATED HEADER\n\n");
  await f.write("#include <lisp/assembly/amd64/instruction.hpp>\n\n");
  await f.write("namespace j::lisp::assembly::amd64::inst {\n");

  for (const inst of outInsts) {
    await writeInstruction(f, inst.name, inst.insts);
  }

  await f.write("}\n");
  await f.close();
}

switch (mode) {
case 'generate':
  const fileName = process.argv[4];
  if (!fileName.endsWith('.hpp')) {
    fail();
  }
  console.log(`Generating ${fileName}...`);
  generate(fileName);
  break;
case 'dump':
  for (const inst of outInsts) {
    console.log(`${inst.name}:`);
    for (const enc of inst.insts) {
      console.log(`  ${enc.encoding.padEnd(4)} ${enc.opcodeString}`);
      let i = 1;
      for (const op of enc.operands) {
        console.log(`    Op #${i}: ${op.data}`);
        if (op.data === 'rax') {
          console.log(op);
        }
        ++i;
      }
        console.log(enc);
      //break;
    }
  }
  break;
}
