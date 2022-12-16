#!/bin/env python
from array import array
from bisect import insort_right
from collections import namedtuple
from argparse import ArgumentParser, Action
from functools import partial, partialmethod
import os.path
import re
import itertools
from ucd_utils import *


Phases = ('orig', 'split', 'filt', 'resolved', 'chunk', 'fill', 'final')
Callback = namedtuple('Callback', ('fn', 'args', 'kwargs'))
ByPhase = namedtuple('ByPhase', Phases)
PhaseNames = ByPhase('Original', 'Split 256+', 'Filtered', 'Ranges resolved', 'Chunked', 'Gaps filled', 'Final')

def em_nop(*args):
  pass

def em_range_orig(state, em):
  state.ranges = erase_ranges(state.ranges, *state.range_to_int(em.begin, em.end))

def em_table_split(state, em):
  extracted, state.ranges = extract_ranges(state.ranges, *state.range_to_int(em.begin, em.end))
  em.opts['ranges'] = extracted
  validate_ranges(extracted)

def em_remove_orig(state, em):
  state.remove_codepoints(em.begin, em.end)

DefEncodingMethodActions = partial(ByPhase, **dict([(p, em_nop) for p in Phases]))

# ArgConst = namedtuple('ArgConst', ['value'])
# ArgAt = namedtuple('ArgAt', ('index'))
# OptArgAt = namedtuple('OptArgAt', ('index', 'default'), defaults=[None])
# KwArgAt = namedtuple('KwArgAt', ('name', 'default'), defaults=[None])

# def arg_getter(opt):
#   if callable(opt):
#     return opt
#   if isinstance(opt, ArgAt) or isinstance(opt, int):
#     index = opt.index if isinstance(opt, ArgAt) else opt
#     return lambda args, kwargs: args[index]
#   if isinstance(opt, OptArgAt):
#     index, default = opt
#     return lambda args, kwargs: args[index] if len(args) > index else default
#   if isinstance(opt, KwArgAt) or isinstance(opt, str):
#     name, default = opt if isinstance(opt, KwArgAt) else (opt, None)
#     return lambda args, kwargs: kwargs.get(name, default)
#   if isinstance(opt, ArgConst):
#     value = opt.value
#     return lambda args, kwargs: value
#   raise ValueError("Invalid arg getter type")

EncodingMethod = namedtuple('EncodingMethod', ('actions', 'begin', 'end', 'opts', 'metadata'))

def DefEncodingMethod(positional_opts=None, default_opts=dict(), **kwargs):
  actions = DefEncodingMethodActions(**kwargs)
  def MakeEncodingMethod(begin, end, *args, comment=None, **kwargs):
    opts = {**default_opts, **kwargs}
    if args:
      assert len(args) <= len(positional_opts)
      opts.update(zip(positional_opts, args))
    return EncodingMethod(actions, begin, end, opts, dict(comment=comment))
  return MakeEncodingMethod

RemoveCodepoints = DefEncodingMethod(orig=em_remove_orig)
TableLookup = DefEncodingMethod(split=em_table_split, default_opts=dict(inline=False))
Range = DefEncodingMethod(positional_opts=['value'])
FixedRange = DefEncodingMethod(positional_opts=['value'])

def ByPhaseArray():
  return ByPhase(*[[] for _ in Phases])

def MakeCallback(fn, *args, **kwargs):
  assert callable(fn)
  return Callback(fn, list(args), kwargs.copy())

def CallbackFactory(fn):
  return lambda *outer_args, **outer_kwargs: lambda *args, **kwargs: fn(*outer_args, *args, **outer_kwargs, **kwargs)

set_kwarg = CallbackFactory(lambda name, value, _, kwargs: kwargs.update([(name, value)]))
append_kwarg = CallbackFactory(lambda name, value, _, kwargs: kwargs.update([(name, [*kwargs[name], value] if name in kwargs else [value])]))

HistogramOpt = {
  'no-full-chunk':  append_kwarg('no_full_chunk', True),
  'by-codepoint':   append_kwarg('types', 'by_codepoint'),
  'codepoint':      append_kwarg('types', 'by_codepoint'),
  'by-range':       append_kwarg('types', 'by_range'),
  'range':          append_kwarg('types', 'by_range'),
  'no-name':        set_kwarg('key_map', None),
  'bar-linear':     set_kwarg('bar_log', False),
}

BlockOpt = {
  'by-codepoint': append_kwarg('types', 'by_codepoint'),
  'codepoint':    append_kwarg('types', 'by_codepoint'),
  'by-range':     append_kwarg('types', 'by_range'),
  'range':        append_kwarg('types', 'by_range'),
  'no-histogram': set_kwarg('with_histogram', False),
}

def opt_none(cb, _):
  return cb
def opt_str_array(cb, opts):
  return cb._replace(args=[*cb.args, opts.split(',')])

def opt_histogram(cb, opts):
  args = cb.args.copy()
  kwargs = cb.kwargs.copy()
  for opt in opts.split(','):
    if opt:
      HistogramOpt[opt](args, kwargs)
  return cb._replace(args=args, kwargs=kwargs)

def opt_blocks(cb, opts):
  args = cb.args.copy()
  kwargs = cb.kwargs.copy()
  for opt in opts.split(','):
    if opt:
      BlockOpt[opt](args, kwargs)
  return cb._replace(args=args, kwargs=kwargs)

def dbg_codepoint_remap(state):
  print(f'Codepoint remapping at {state.phase_name()}')
  state.codepoint_remaps.dump()
  print('')

def dbg_gcs_chunks(state, gcs, gcs_set, **kwargs):
  print(f'Chunks containing general classes at {state.phase_name()}')
  for c in state.chunks:
    if chunk_contains_any(gcs_set):
      print_chunk(c, indent=1, **kwargs)

def dbg_gcs_ranges(state, gcs, _, **kwargs):
  print(f'Intervals containing general classes at {state.phase_name()}')
  print_intervals(state.ranges, indent=1, collapse=True, **kwargs)

def dbg_gcs(state, gcs):
  gcs_set=frozenset(gcs)
  kwargs = dict(interval_filter=lambda i: i.value in gcs_set,
              key_map=state.aliases,
              with_key=True,
              filter_after=state.args.dump_gcs_show_after,
              unicode_blocks=state.unicode_blocks)
  if state.chunks is not None:
    dbg_gcs_chunks(state, gcs, gcs_set, **kwargs)
  else:
    dbg_gcs_ranges(state, gcs, gcs_set, **kwargs)

def dbg_histogram(state, **kwargs):
  call_kwargs = dict(label=state.phase_name(), key_map=state.aliases, bar_log=True, with_key=True)
  call_kwargs.update(kwargs)
  if state.chunks is not None:
    print_chunk_histogram(state.chunks, **call_kwargs)
  else:
    print_range_histogram(state.ranges, **call_kwargs)

def dbg_unicode_blocks(state, with_histogram=True, **kwargs):
  min_ranges = state.args.filter_min_ranges
  block_name = to_search_regexes(state.args.filter_block_name) if state.args.filter_block_name else None
  print(f'Unicode Blocks at {state.phase_name()}:')
  it = chunks_intervals(state.chunks) if state.chunks is not None else iter(state.ranges)
  cur = next(it)
  for b in state.unicode_blocks:
    if block_name and not re.search(block_name, b.search_name, flags=re.IGNORECASE):
      continue
    h = dict()
    if with_histogram or min_ranges > 0:
      while cur is not None and cur.begin < b.end:
        accumulate_histogram_interval(h, cur, begin=b.begin, end=b.end)
        if cur.end <= b.end:
          cur = next(it, None)
        else:
          break
      if min_ranges > 0 and min_ranges > sum(e[0] for e in h.values()):
        continue

    state.print_range(b.begin, b.end, indent=1, after=f' {b.name}', nl=True)
    if with_histogram:
      print_mini_histograms(h, indent=2, **kwargs)
  print("")

def dbg_unhandled_ranges(state, **kwargs):
  print(f'Unhandled ranges at {state.phase_name()}:')
  for begin, end, ranges in state.unhandled_ranges():
    h = dict()
    state.print_range(begin, end, before=f'  Unhandled region ', nl=True)
    for i in iter(ranges):
      # state.print_range(i.begin, i.end, before=f'  Interval', nl=True)
      accumulate_histogram_interval(h, i, begin=begin, end=end)
    print_mini_histograms(h, indent=2, **kwargs)
  print('')

def dbg_large_ranges(state):
  print(f'Large regions at {state.phase_name()}:')
  prev = None
  for r, resolved in itertools.zip_longest(state.large_ranges, state.resolved_regions):
    if r.min_begin != r.max_begin:
      if prev and prev.max_end == r.max_begin:
        state.print_range(prev.min_end, r.max_begin, before='    ⇅ Conflict: ', after=f' {prev.value} vs. {r.value}', nl=True)
        if resolved:
          state.print_codepoint(resolved.begin, before='      Resolved: ', nl=True)
      state.print_codepoint(r.min_begin, before='  Range [', after=']')
    else:
      print('  Range         ', end='')
    state.print_range(r.max_begin, r.min_end)
    if r.min_end != r.max_end:
      state.print_codepoint(r.max_end, before='[', after=']')
    else:
      print(f'        ', end='')
    if (r.min_end != r.max_end or r.min_begin != r.max_begin) and resolved:
      state.print_range(resolved.begin, resolved.end, before=' Resolved: ')

    min_size = large_region_min_size(r)
    max_size = large_region_max_size(r)
    print(f' Sz=0x{min_size:05X}', end='')
    if min_size != max_size:
      print(f'-{max_size:05X}', end='')
    else:
      print('      ', end='')
    print(f' {r.value}')
    prev = r
  print('')

def act_dry_run(state, args, file_type):
  print(f'Dry run - would generate {file_type} to {args.output}.')

def act_generate(state, args, file_type):
  print(f'Generating {file_type} to {args.output}.')

class CodepointRemaps:
  def __init__(self):
    self.tables = ([], [])

  def dump(self):
    total = 0
    for begin, size in self.tables[1]:
      total += size
      end = begin + size
      print(f'  DEL {begin:06X}-{end:06X}: MAP ABOVE {end:06X} TO {end - total:06X} (size {size})')
    print(f'  TOTAL DELETED: {total}')

  def delete(self, begin, end):
    insort_right(self.tables[1], (begin, end - begin))
    begin_int, end_int = self.range_to_internal(begin, end)
    insort_right(self.tables[0], (begin_int, begin_int - end_int))

  def convert(self, idx, cp):
    for begin, delta in self.tables[idx]:
      if cp >= begin:
        cp += delta
        assert cp >= begin, f'Codepoint {cp:06X} not mappable.'
    return cp

  def convert_range(self, idx, cp0, cp1, allow_empty=False):
    for begin, delta in self.tables[idx]:
      if cp1 > begin:
        cp1 += delta
        if allow_empty and cp1 < begin:
          return begin, begin
        assert cp1 >= begin, f'Codepoint {cp1:06X} not mappable.'
        if cp0 >= begin:
          cp0 += delta
          if allow_empty and cp0 < begin:
            return begin, begin
          assert cp0 >= begin, f'Codepoint {cp0:06X} not mappable. End is {cp1:06X} allow_empty={allow_empty}.'
    return cp0, cp1

  to_internal = partialmethod(convert, 0)
  to_unicode = partialmethod(convert, 1)
  range_to_internal = partialmethod(convert_range, 0)
  range_to_unicode = partialmethod(convert_range, 1)

class EncodingMethods:
  def __init__(self, defaults=[]):
    self.methods = defaults
    self.unhandled_cache = None

  def enter_phase(self, state, phase_idx):
    for index, em in enumerate(self.methods):
      em.actions[phase_idx](state, em)

  def unhandled_intervals(self, state):
    if not self.unhandled_cache:
      begin = 0
      self.unhandled_cache = []
      for m in self.methods:
        m_begin, m_end = state.range_to_int(m.begin, m.end, allow_empty=True)
        if m_begin == m_end:
          continue
        if m_begin != begin:
          assert m_begin > begin
          self.unhandled_cache.append((begin, m_begin))
        begin = m_end
      if begin != state.end:
        self.unhandled_cache.append((begin, state.end))
    return self.unhandled_cache

class State:
  def unhandled_ranges(self):
    return ((begin, end, ranges_window(self.ranges, begin, end)) for begin, end in self.encoding_methods.unhandled_intervals(self))

  def format_codepoint(self, cp):
    return f'{self.codepoint_remaps.to_unicode(cp):06X}'

  def print_codepoint(self, cp, indent=0, nl=False, before = '', after=''):
    print_indent(indent)
    print(before + f'{self.codepoint_remaps.to_unicode(cp):06X}' + after, end=('\n' if nl else ''))

  def print_range(self, begin, end, fixed_width=False, nl=False, after='', **kwargs):
    self.print_codepoint(begin, **kwargs)
    if begin != end:
      self.print_codepoint(end, before='..')
    elif fixed_width:
      print('        ', end='')
    print(after, end=('\n' if nl else ''))

  def cp_to_unicode(self, cp):
    return self.codepoint_remaps.to_unicode(cp)
  def cp_to_int(self, cp):
    return self.codepoint_remaps.to_internal(cp)
  def range_to_unicode(self, begin, end, **kwargs):
    return self.codepoint_remaps.range_to_unicode(begin, end, **kwargs)
  def range_to_int(self, begin, end, **kwargs):
    return self.codepoint_remaps.range_to_internal(begin, end, **kwargs)

  def remove_codepoints(self, begin, end):
    int_begin, int_end = self.codepoint_remaps.range_to_internal(begin, end)
    self.ranges = delete_and_remap_codepoints(self.ranges, int_begin, int_end)
    self.unicode_blocks = delete_and_remap_codepoints(self.unicode_blocks, int_begin, int_end)
    self.end -= int_end - int_begin
    self.codepoint_remaps.delete(begin, end)

  def __init__(self, args):
    self.codepoint_remaps = CodepointRemaps()
    self.args = args
    self.ucd_path = args.ucd_path
    self.phase = None
    self.aliases = parse_aliases(self.ucd('PropertyValueAliases'), 'gc', first_only=True)
    self.unicode_blocks = parse_unicode_blocks(self.ucd('Blocks'))
    self.resolved_regions = []

    self.encoding_methods = EncodingMethods([
      TableLookup(0x00000, 0x00100, inline=True,
                  comment='Inline lookup for the first 256 codepoints.'),

      TableLookup(0x00400, 0x00500,
                  comment='Cyrillic 0400-0500'),
      TableLookup(0x01E00, 0x01F00, comment='Latin Extended Additional 1E00-1F00'),
      TableLookup(0x02100, 0x02200,
                  comment="Letterlike Symbols 2100-2150\n"
                  "Number Forms 2150-2190, and\n"
                  "Arrows 2190-2200"),

      TableLookup(0x02C80, 0x02D00,
                  comment="Added because of Coptic, others added just to align.\n"
                  "Glagolitic 02C00-02C60,\n"
                  "Latin Extended-C 02C60-02C80, and\n"
                  "Coptic 02C80-02D00"),
      TableLookup(0x03000, 0x03100,
                  comment="CJK Symbols and Punctuation 3000-3040,\n"
                  "Hiragana 3040-30A0, and\n"
                  "Katakana 30A0-3100"),
      TableLookup(0x0A600, 0x0A800, comment="Modifier Tone Letters and Latin Extended-D"),

      RemoveCodepoints(0x0D800, 0x0F900,
                       comment="Remove High Surrogates D800-DB80,\n"
                       "High Private Use Surrogates DB80-DC00,\n"
                       "Low Surrogates DC00-E000, and\n"
                       "Private Use Area E000-F900"),
      TableLookup(0xFE00, 0x10000,
                  comment=''
                  "Generate direct table for the exceedingly miscellaneous 256 codepoints\n"
                  "at the end of BMP:\n"
                  "\n"
                  "Variation Selectors (FE00-FE10),\n"
                  "Vertical Forms (FE10-FE20),\n"
                  "Combining Half Marks (FE20-FE30),\n"
                  "CJK Compatibility Forms (FE30-FE50),\n"
                  "Small Form Variants (FE50-FE70),\n"
                  "Arabic Presentation Forms-B (FE70-FF00),\n"
                  "Halfwidth and Fullwidth Forms (FF00-FFF0), and\n"
                  "Specials 0FFF0-10000"),

      RemoveCodepoints(0x14800, 0x16800, comment="Unallocated space in SMP"),

      FixedRange(0x1AFF0, 0x1B000, 'Lm',
                 comment="Manual handling of Kana Extended, that was removed along with unused space."),
      RemoveCodepoints(0x19000, 0x1B000,
                       comment=""
                       "Remove unallocated space in SMP (Plane 1, Supplementary Multilingual Plane)\n"
                       "The end has a 16-codepoint block for \"Kana Extended\",\n"
                       "so that needs to be special-cased, to make the removals align."),

      RemoveCodepoints(0x31400, 0xE0000,
                       comment=""
                       "Remove unallocated space in TIP (Plane 3, Tertiary Idiographic Plane)\n"
                       "Also remove unallocated planes 4-13."),
    ])

    self.chunks = None
    self.special_gcs = frozenset([
      'Zl', # Line_Separator, single character
      'Zp', # Paragraph_Separator, single character
      'Co', # Private_Use
      'Cs', # Surrogate
      'Pc', # Connector_Punctuation (very few characters)
      'Pf', # Final_Punctuation
      'Pi', # Initial_Punctuation
      'Nl', # Letter number
      'Pd', # Dash_Punctuation
      'Me', # Enclosing_Mark
      'Zs', # Space_Separator
      'Cf', # Format
      'Lt', # Titlecase_Letter
      'Sc', # Currency_Symbol
    ])
    self.chunk_size_shift = 8
    self.chunk_size = 1 << self.chunk_size_shift
    self.chunk_index_shift = self.chunk_size_shift - 4
    self.end = 0xE01F0 # Supplementary Private Use Area-A

  def round_to_boundary(self, min, max):
    assert min < max
    if not min & 0xFF or not max & 0xFF:
      return min if (min ^ (min - 1)) > (max ^ (max - 1)) else max
    if not max & 0xFF:
      return max
    cross = max & ~min
    boundary = 0x100000
    while not cross & boundary:
      boundary //= 2
    return (min | boundary) & -boundary

  def resolve_large_regions(self):
    prev_begin = None
    prev = None
    for r in self.large_ranges:
      begin = r.min_begin
      if prev:
        prev_end = prev.max_end
        if prev.max_end != prev.min_end:
          if prev.max_end == r.max_begin:
            begin = self.round_to_boundary(r.min_begin, r.max_begin)
            prev_end = begin
            assert begin >= r.min_begin and begin <= r.max_begin
        self.resolved_regions.append(Region(prev_begin, prev_end, prev.value, prev.special_cases))
      prev_begin = begin
      prev = r
    if prev:
      self.resolved_regions.append(Region(prev_begin, prev.max_end, prev.value, prev.special_cases))

  def apply_large_regions(self):
    idx = 0
    result = []
    it = iter(self.resolved_regions)
    cur = next(it)
    cur_inserted = False
    for r in self.ranges:
      if cur and r.begin >= cur.end:
        assert cur_inserted
        cur_inserted = False
        cur = next(it, None)
      if not cur or r.end <= cur.begin:
        result.append(r)
      elif not cur_inserted:
        cur_inserted = True
        result.append(Interval(cur.begin, cur.end, cur.value))
    assert len(result) <= len(self.ranges)
    self.ranges = result

  def dump_aliases(self):
    print("GeneralClass aliases:")
    for key, value in sorted(self.aliases.items()):
      special =  '[SPECIAL] ' if key in self.special_gcs else '          '
      print(f'  {special}{key:2}: {value}')
    print('')

  def process(self):
    if self.args.dump_aliases:
      self.dump_aliases()

    self.ranges = parse_unicode_table(self.ucd('UnicodeData'),
                                      end=self.end, # Supplementary Private Use Area-A
                                      is_unicodedata=True,
                                      value_index=2)
    self.enter_phase('orig')
    self.enter_phase('split')
    self.ranges = filter_ranges(self.ranges, ignored_vals=self.special_gcs)
    self.large_ranges = find_large_ranges(self.ranges, end=self.end)
    self.enter_phase('filt')
    self.resolve_large_regions()
    self.apply_large_regions()
    self.enter_phase('resolved')
    self.chunks = chunk_ranges(self.ranges, self.chunk_size, begin=256)
    self.enter_phase('chunk')
    self.chunks = [extend_ranges(c) for c in self.chunks]
    self.enter_phase('fill')
    self.enter_phase('final')

  def ucd(self, sub):
    return os.path.join(self.ucd_path, f'{sub}.txt')

  def phase_name(self):
    return "None" if self.phase is None else getattr(PhaseNames, self.phase)

  def enter_phase(self, phase):
    phase_idx = 0 if self.phase == None else Phases.index(self.phase) + 1
    assert phase == Phases[phase_idx], 'Entered invalid phase'
    self.phase = phase
    self.encoding_methods.enter_phase(self, phase_idx)
    validate_ranges(self.ranges)
    for dbg_cb in self.args.debug_callbacks[phase_idx]:
      dbg_cb.fn(self, *dbg_cb.args, **dbg_cb.kwargs)

class DebugAction(Action):
  def __init__(self, *args, default_phases=Phases, cb=None, cb_args=None, **kwargs):
    self.cb = cb if isinstance(cb, Callback) else MakeCallback(cb)
    self.default_phases = default_phases
    self.cb_arg_parser = cb_args
    super().__init__(*args, **kwargs)

  def parse_value_str(self, value):
    value = value.split(':', 1)
    return (value[0].split(',') if len(value) == (2 if self.cb_arg_parser else 1) else self.default_phases,
            self.cb_arg_parser(self.cb, value[-1]) if self.cb_arg_parser else self.cb)

  def __call__(self, parser, namespace, values, option_string=None):
    by_phase = getattr(namespace, self.dest)
    for phases, cb in (self.parse_value_str(v) for v in values):
      for p in phases:
        getattr(by_phase, p).append(cb)

parser = ArgumentParser()
parser.set_defaults(debug_callbacks=ByPhaseArray())
parser.add_argument('--ucd-path', default='/usr/share/unicode/')
parser.add_argument('--dump-aliases', action='store_true', default=False)
parser.add_argument('--dump-gcs-show-after', type=int, nargs='?', const=1, default=0)
parser.add_argument('--dump-gcs', action=DebugAction, nargs='*', cb=dbg_gcs, dest='debug_callbacks', cb_args=opt_str_array)
parser.add_argument('--dump-hist', action=DebugAction, nargs='*', cb=dbg_histogram, dest='debug_callbacks', cb_args=opt_histogram)
parser.add_argument('--dump-blocks', action=DebugAction, nargs='*', cb=dbg_unicode_blocks, dest='debug_callbacks', cb_args=opt_blocks)
parser.add_argument('--dump-unhandled', action=DebugAction, nargs='*', cb=dbg_unhandled_ranges, dest='debug_callbacks', cb_args=opt_blocks)
parser.add_argument('--filter-min-ranges', type=int, default=0)
parser.add_argument('--filter-block-name', type=str, nargs='*')
parser.add_argument('--dump-large-ranges', action=DebugAction, nargs='*', cb=dbg_large_ranges, dest='debug_callbacks')
parser.add_argument('--dump-codepoint-remap', action=DebugAction, nargs='*', cb=dbg_codepoint_remap, dest='debug_callbacks')

subparsers = parser.add_subparsers(required=True)

# parser_dump = subparsers.add_parser('dump', aliases=['d'])
# parser_dump.set_defaults(fn=act_dump, fn_args=[])
# parser_dump.add_argument('gcs', nargs='+')

parser_gen_hpp = subparsers.add_parser('hpp')
parser_gen_hpp.set_defaults(fn=act_generate, fn_args=['hpp'])
parser_gen_hpp.add_argument('output')
parser_gen_hpp.add_argument('--dry-run', action='store_const', const=act_dry_run, dest='fn')

parser_gen_cpp = subparsers.add_parser('cpp')
parser_gen_cpp.set_defaults(fn=act_generate, fn_args=['cpp'])
parser_gen_cpp.add_argument('output')
parser_gen_cpp.add_argument('--dry-run', action='store_const', const=act_dry_run, dest='fn')

args = parser.parse_args()
state = State(args)

state.process()

if state.phase != 'final':
  raise ValueError(f'Expected state to be final when running action, got {state.phase}.')
args.fn(state, args, *args.fn_args)
