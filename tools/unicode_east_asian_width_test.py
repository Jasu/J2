#!/bin/env python
import sys

from ucd_utils import *

if len(sys.argv) != 3:
  print(f'Usage: {sys.argv[0]} EastAsianWidth.txt OUTPUT')
  sys.exit(1)

ranges = parse_unicode_table(sys.argv[1], end=0xF0000)
ranges = default_ranges(ranges, [
  Interval(0x000000, 0x003400, 'N'),
  Interval(0x003400, 0x004DC0, 'W'),
  Interval(0x004DC0, 0x004E00, 'N'),
  Interval(0x004E00, 0x00A000, 'W'),
  Interval(0x00A000, 0x00F900, 'N'),
  Interval(0x00F900, 0x00FB00, 'W'),
  Interval(0x00FB00, 0x020000, 'N'),
  Interval(0x020000, 0x02FFFE, 'W'),
  Interval(0x02FFFE, 0x030000, 'N'),
  Interval(0x030000, 0x03FFFE, 'W'),
  Interval(0x03FFFE, 0x0F0000, 'N'),
  # Interval(0x03FFFE, 0x110000, 'N'),
])
outfile = open(sys.argv[2], "w")
outfile.write("""\
// GENERATED FILE - Do not edit.

#include <detail/preamble.hpp>
#include <strings/unicode/east_asian_width.hpp>

TEST_SUITE_BEGIN("UTF-8 - East Asian Width (Generated)");

namespace s = j::strings;

TEST_CASE("UTF-8 - East Asian Width (Generated)") {
""")

for r in ranges:
  outfile.write(f'  REQUIRE(s::get_east_asian_width(0x{r.begin:06X}U) == s::east_asian_width::{r.value.lower()});')
  outfile.write("\n")
  if r.begin + 1 != r.end:
    outfile.write(f'  REQUIRE(s::get_east_asian_width(0x{r.end - 1:06X}U) == s::east_asian_width::{r.value.lower()});')
    outfile.write("\n")

outfile.write("""\
}

TEST_SUITE_END();
""")
