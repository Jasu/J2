#!/bin/env python
import sys

from ucd_utils import *

if len(sys.argv) != 4:
  print(f'Usage: {sys.argv[0]} hpp|cpp EastAsianWidth.txt OUTPUT')
  sys.exit(1)

is_cpp = sys.argv[1] == 'cpp'

prop_values = ['N', 'F', 'H', 'W', 'Na', 'A']

ranges = parse_unicode_table(sys.argv[2], end=0xF0000)
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
# for r in ranges:
#   print(f'{r.begin}-{r.end}')
ranges = assign_ranges(ranges, [
  Interval(0x0E0100, 0x0E01F0, 'N'),
])

chunk_size_shift = 9
chunk_size = 1 << chunk_size_shift
chunk_index_shift = chunk_size_shift - 4

chunks = chunk_ranges(ranges, chunk_size)
piece_references, pieces = merge_duplicate_chunks(chunks)
piece_references, pieces = sort_pieces_by_usage(piece_references, pieces)

piece_references, pieces = match_pieces(piece_references, pieces)

outfile = open(sys.argv[3], "w")

if not is_cpp:
  outfile.write("""\
  #pragma once

  // GENERATED HEADER - Do not edit.

  #include <hzd/utility.hpp>

  namespace j::strings::inline unicode {
    enum class east_asian_width : u8_t {
      /// Neutral
      n = 0U,
      /// Full-width
      f,
      /// Half-width
      h,
      /// Wide
      w,
      /// Narrow
      na,
      /// Ambiguous
      a,
    };

  extern constinit const u64_t east_asian_width_table[];
  extern constinit const u8_t east_asian_width_indexes[];

  """)
else:
  outfile.write("""\
  // GENERATED FILE - Do not edit.
  #include <strings/unicode/east_asian_width.hpp>

  #include <hzd/utility.hpp>

  namespace j::strings::inline unicode {
  """)


piece_references, last_piece_value = trim_piece_references(piece_references)
if is_cpp:
  write_nibble_table(outfile, 'east_asian_width_table', pieces, get_value = lambda v : prop_values.index(v), is_inline = False)
  write_index_table(outfile, 'east_asian_width_indexes', piece_references, is_inline = False)
else:
  outfile.write(
    """
    inline constexpr east_asian_width get_east_asian_width(const u32_t c) noexcept {
      switch (c) {
        case 0x00FFFEU: return east_asian_width::n;
        case 0x00FFFFU: return east_asian_width::n;
        case 0x10FFFEU: return east_asian_width::n;
        case 0x10FFFFU: return east_asian_width::n;
        default: break;
      }
      // Private use area
      if (c >= 0x0E0100U) {
        if (J_UNLIKELY(c < 0x0E01F0U)) {
          return east_asian_width::a;
        } else if (J_UNLIKELY(c >= 0x0F0000U)) {
          return east_asian_width::a;
        }
      }
  """)

  outfile.write(f'    const u32_t idx = (c >= 0x{len(piece_references) << (chunk_size_shift):X}U)' + "\n")
  outfile.write(f'      ? {last_piece_value << (chunk_index_shift - 1)}U' + "\n")
  outfile.write(f'      : (east_asian_width_indexes[c >> {chunk_size_shift}U] << {chunk_index_shift - 1}U);' + "\n");
  outfile.write(f'    const u32_t offset = (c >> 4U) & {(1 << (chunk_index_shift)) - 1}U;' + "\n");

  outfile.write("""\
      const u64_t val = east_asian_width_table[idx + offset];
      return static_cast<east_asian_width>((val >> ((c & 15U) << 2U)) & 15U);
    }\n""")

outfile.write("}\n")
