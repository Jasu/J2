#!/bin/env python

import sys
import re
import collections

Case = collections.namedtuple('Case', ['name', 'chars', 'breaks'])

if len(sys.argv) != 3:
  print(f'Usage: {sys.argv[0]} LINEBREAKTEST OUTPUT')
  sys.exit(1)

outfile = open(sys.argv[2], "w")
outfile.write("""\
// GENERATED FILE - Do not edit.

#include <strings/lbr.hpp>

TEST_SUITE_BEGIN(\"UTF-8 Rope - Line Break (Generated)\");

struct J_TYPE_HIDDEN lbr_case {
  const char * name;
  const char * input;
  u32_t sz;
  const char * const breaks[32] = { nullptr };
};

TEST_CASE(\"UTF-8 Rope - Line Break (Generated)\") {
  const lbr_case cases[]{
""")

def codepoints(b):
  return [s.strip() for s in b.split('×')]
def cstr(b):
  return ''.join(["\\" + (f'U{int(c, 16):08x}' if len(c) > 4 else f'u{int(c, 16):04x}') for c in codepoints(b)])

infile = open(sys.argv[1], 'r')
cases = []
for line in infile:
  line = line.split('#')
  if len(line) != 2 or not line[0]:
    continue
  test, name = [s.strip().strip("×").strip("÷").strip() for s in line]
  # The test data tailors number handling
  if '(NU)' in name or re.search(r'\(P[OR]\).*\(OP', name) or re.search(r'\(C[PL].*\(P[OR]\)', name):
    continue
  # Don't assume that XX defaults to AL
  if '(XX_AL)' in name:
    continue

  breaks = [s.strip() for s in test.split('÷')]
  break_strs = [cstr(s) for s in breaks]
  cases.append(Case(name, ''.join(break_strs), break_strs))

for c in cases:
  outfile.write("    { " + f'"{c.name}", "{c.chars}", {len(c.breaks)}, ' + "{\"" + '", "'.join(c.breaks) + "\"} },\n");
outfile.write("""\
  };

  for (auto & c : cases) {
    CAPTURE(c.name);
    s::utf8_rope r;
    r += c.input;
    assert_line_breaks(r, c.sz, c.breaks);
  }
}

TEST_SUITE_END();
""")
