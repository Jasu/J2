#!/bin/env python

import sys
import collections

Interval = collections.namedtuple('Range', ['begin', 'end', 'value'])

if len(sys.argv) != 4:
  print(f'Usage: {sys.argv[0]} LINEBREAK GENERAL_CATEGORY OUTPUT')
  sys.exit(1)

def parse_unicode_table(path):
  f = open(path, 'r')
  result = []
  for line in f:
    line = line.split('#')[0].strip()
    if not line:
      continue
    rng, val = line.split(';')
    rng = rng.split('..')
    begin = int(rng[0], 16)
    end = (begin if len(rng) == 1 else int(rng[1], 16)) + 1
    result.append(Interval(begin, end, val))
  f.close()
  result.sort()
  return result

general_categories = parse_unicode_table(sys.argv[2])

def general_category(c):
  for cat in general_categories:
    if cat.begin <= c:
      return cat.value
  raise("Not found")

lbr_map = {
  'XX': 'AL',
  'CJ': 'NS',
  'AI': 'AL',
  'SG': 'AL',
  'NL': 'BK',
}

vals = []
for val in parse_unicode_table(sys.argv[1]):
  if val.value == 'XX':
    continue
  if val.value in lbr_map:
    val = val._replace(value = lbr_map[val.value])
  elif val.value == 'SA':
    val = val._replace(value = 'CM' if general_category(val.begin) in ['Mn', 'Mc'] else 'AL')
  vals.append(val)

outfile = open(sys.argv[3], "w")
outfile.write("// GENERATED FILE - Do not edit.\n\n")
outfile.write("#include <detail/preamble.hpp>\n\n")
outfile.write("#include <strings/unicode/line_break_property.hpp>\n\n")
outfile.write("TEST_SUITE_BEGIN(\"UTF-8 Rope - Line break property (Generated)\");\n\n")
outfile.write("namespace s = j::strings;\n\n")
outfile.write("namespace {\n")

outfile.write("  J_NO_DEBUG const u32_t code_points[] = {");
i = 0
for v in vals:
  if i % 8 == 0:
    outfile.write("\n   ")
  outfile.write(f' 0x{v.begin:06X}U,')
  i += 1
  if v.begin + 1 != v.end:
    if i % 8 == 0:
      outfile.write("\n   ")
    outfile.write(f' 0x{v.end - 1:06X}U,')
    i += 1
outfile.write("\n  };\n\n")

outfile.write("  J_NO_DEBUG const s::lbr_t lbrs[] = {");
i = 0
for v in vals:
  if i % 8 == 0:
    outfile.write("\n   ")
  outfile.write(f' s::lbr_t::{v.value.lower()},')
  i += 1
  if v.begin + 1 != v.end:
    if i % 8 == 0:
      outfile.write("\n   ")
    outfile.write(f' s::lbr_t::{v.value.lower()},')
    i += 1
outfile.write("\n  };\n\n")


outfile.write("TEST_CASE(\"Line break properties (Generated)\") {\n")
outfile.write(f'  for (u32_t i = 0U; i < {i}U; ++i) ')
outfile.write("{\n")
outfile.write(f'    CAPTURE(code_points[i]);\n')
outfile.write(f'    REQUIRE(s::lbr_category(code_points[i]) == lbrs[i]);')
outfile.write("  }\n\n}\n\n}\n")

outfile.write("TEST_SUITE_END();\n")
