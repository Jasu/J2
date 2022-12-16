#!/bin/env python

import sys

if len(sys.argv) != 3:
  print(f'Usage: {sys.argv[0]} INPUT OUTPUT')
  sys.exit(1)

classes = {}
infile = open(sys.argv[1], 'r')
outfile = open(sys.argv[2], "w")
outfile.write("// GENERATED FILE - Do not edit.\n\n")
outfile.write("#include <strings/egc.hpp>\n\n")
outfile.write("TEST_SUITE_BEGIN(\"UTF-8 Rope - Grapheme Clusters (Generated)\");\n\n")
outfile.write("namespace s = j::strings;\n\n")

for line in infile:
  parts = [s.strip().strip('÷ ') for s in line.split('#', maxsplit = 2)]
  if len(parts) != 2:
    continue
  line, name = parts
  if not line or not name:
    continue
  graphemes = [s.strip() for s in line.split('÷')]
  outfile.write(f'TEST_CASE("{name}") ')
  outfile.write("{\n  s::utf8_rope r;\n  r += \"")
  for g in graphemes:
    code_points = [s.strip() for s in g.split('×')]
    for p in code_points:
      if len(p) > 4:
        outfile.write("\\U" + p.zfill(8))
      else:
        outfile.write("\\u" + p);
  outfile.write("\";\n  assert_grapheme_clusters(r, { ")
  for g in graphemes:
    outfile.write("\"")
    code_points = [s.strip() for s in g.split('×')]
    for p in code_points:
      if len(p) > 4:
        outfile.write("\\U" + p.zfill(8))
      else:
        outfile.write("\\u" + p);
    outfile.write("\", ")

  outfile.write("});\n}\n\n")


outfile.write("TEST_SUITE_END();\n")
