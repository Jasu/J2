#!/bin/env python

import sys

if len(sys.argv) != 3:
  print(f'Usage: {sys.argv[0]} INPUT OUTPUT')
  sys.exit(1)

classes = {}
infile = open(sys.argv[1], 'r')
for line in infile:
  line = line.split('#')[0].strip()
  if not line:
    continue
  rng, cls = [s.strip() for s in line.split(';')]
  rng = [int(s, 16) for s in rng.split('..')]
  if len(rng) == 1:
    rng = [rng[0], rng[0]]
  if not cls in classes:
    classes[cls] = [rng]
  elif classes[cls][-1][1] == rng[0] - 1:
    classes[cls][-1][1] = rng[1]
  else:
    classes[cls].append(rng)

def range_count(rng):
  return sum([r[1] - r[0] + 1 for r in rng])

if_threshold = 5

outfile = open(sys.argv[2], "w")
outfile.write("#pragma once\n\n")
outfile.write("// GENERATED HEADER - Do not edit.\n\n")
outfile.write("#include <hzd/types.hpp>\n\n")
outfile.write("namespace j::strings::inline unicode {")

for cls, rngs in classes.items():
  rng_cnt = range_count(rngs)
  if cls == 'L':
    name = 'hangul_syllable_l'
  elif cls == 'LV':
    name = 'hangul_syllable_lv'
  elif cls == 'V':
    name = 'hangul_syllable_v'
  elif cls == 'T':
    name = 'hangul_syllable_t'
  elif cls == 'SpacingMark':
    name = 'spacing_mark'
  elif cls in ['Extended_Pictographic', 'Regional_Indicator', 'Control', 'Extend', 'Prepend']:
    name = cls.lower()
  else:
    continue

  outfile.write("\n")
  outfile.write(f'  constexpr bool is_{name}(u32_t c) noexcept ')
  outfile.write("{\n")
  if rngs[0][0] != 0:
    outfile.write(f'    if (c < {rngs[0][0]} || c > {rngs[-1][1]})')
  else:
    outfile.write(f'    if (c > {rngs[-1][1]})')
  outfile.write(" {\n      return false;\n    }\n")

  had_switch_case = False
  for start, end in rngs:
    if end - start + 1 < if_threshold:
      had_switch_case = True
      continue
    if start == rngs[0][0]:
      outfile.write(f'    if (c <= {end}) ')
    elif end == rngs[-1][1]:
      outfile.write(f'    if (c >= {start}) ')
    else:
      outfile.write(f'    if (c >= {start} && c <= {end}) ')
    outfile.write("{\n      return true;\n    }\n")
  if had_switch_case:
    outfile.write("    switch (c) {\n")
    for start, end in rngs:
      if end - start + 1 >= if_threshold:
        continue
      for x in range(start, end + 1):
        outfile.write(f'    case {x}: return true;')
        outfile.write("\n")
    outfile.write("    default: return false;\n    }\n")
  else:
    outfile.write("    return false;\n")
  outfile.write("  }\n")

outfile.write("}\n")
outfile.close()
