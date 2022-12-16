#include <lisp/reader/read.hpp>
#include <lisp/mem/heap.hpp>
#include <exceptions/assert.hpp>
#include <logging/global.hpp>
#include <lisp/reader/state.hpp>
#include <lisp/sources/source.hpp>
#include <lisp/reader/build_ast.hpp>
#include <lisp/reader/interactive_reader.hpp>
#include <lisp/values/lisp_str.hpp>
#include <lisp/values/lisp_imms.hpp>
#include <mem/unique_ptr.hpp>
#include <util/hex.hpp>
#include <lisp/reader/read_types.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#include <string-to-double.h>
#pragma clang diagnostic pop

/*!header:re2c:on */
#ifndef READ_TYPES_H_INCLUDED
#define READ_TYPES_H_INCLUDED

#include <mem/bump_pool.hpp>
#include <strings/unicode/utf8.hpp>

namespace j::lisp::reader {
  /*!types:re2c*/
  struct J_TYPE_HIDDEN string_part final {
    string_part * next = nullptr;
    u32_t size = 0U;

    J_A(AI,ND) inline explicit string_part(u32_t size) noexcept
      : size(size)
    { }

    J_A(AI,ND) inline string_part(const char * J_NOT_NULL begin, u32_t size) noexcept
      : size(size)
    {
      j::memcpy((char*)(this + 1U), begin, size);
    }

    J_A(AI,RNN,NODISC) inline char * data() noexcept {
      return (char*)(this + 1);
    }

    J_A(AI,RNN,NODISC) inline const char * data() const noexcept {
      return (const char*)(this + 1);
    }

    J_A(RNN) inline char * copy_to(char * J_NOT_NULL to, u32_t sz) const noexcept {
      J_ASSERT(sz);
      return (char*)::j::mempcpy(to, (const void *)(this + 1), sz = J_MIN(sz, size));
    }
  };

  struct lex_state final {
    lex_state(const char * J_NOT_NULL buffer, u32_t size, u16_t source_index) noexcept
      : cursor(buffer),
        limit(buffer + size),
        marker(buffer),
        line_start(buffer),
        str_next(&str_first),
        source_index(source_index)
    {
    }

    /// The current character being read.
    const char * cursor;
    /// End of character in buffer.
    const char * limit;
    /// The last token that was read.
    const char * marker;
    u32_t yyaccept    = 0U;
    i32_t re_state     = -1;
    YYCONDTYPE cond    = yycinit;

    // Autogenerate chars used for tag symbols like @foo would use yy4 etc.
    /*!stags:re2c format = "const char *@@ = nullptr;\n"; */
    const char * line_start = nullptr;

    struct string_part * str_first = nullptr;
    struct string_part ** str_next = nullptr;
    u32_t str_size = 0U;
    u32_t line_number = 0U;
    u16_t source_index = 0U;

    j::mem::bump_pool temp_pool;

    J_A(AI) inline void reset_string() noexcept {
      str_first = nullptr;
      str_next = &str_first;
      str_size = 0U;
    }

    [[nodiscard]] strings::const_string_view save_string(const char * J_NOT_NULL begin, u32_t sz) noexcept {
      return sz ? strings::const_string_view((const char*)::j::memcpy((char*)temp_pool.allocate(sz), begin, sz), sz) : strings::const_string_view();
    }

    void write_string(const char * J_NOT_NULL begin, u32_t size) noexcept {
      J_ASSERT(size);
      auto part = &temp_pool.emplace_with_padding<string_part>(size, begin, size);
      *str_next = part;
      str_next = &(part->next);
      str_size += size;
    }

    void write_utf8_code_point(u32_t codepoint) noexcept {
      u8_t size = strings::unicode::utf8_code_point_bytes(codepoint);
      str_size += size;
      auto part = &temp_pool.emplace<string_part>(size);
      *str_next = part;
      str_next = &(part->next);
      strings::utf8_encode(part->data(), codepoint);
    }

    J_A(NODISC,ND) inline sources::source_location current_location(const char * J_NOT_NULL cur_line_pos) noexcept {
      J_ASSERT(line_start && line_start <= cur_line_pos);
      return sources::source_location(source_index, line_number, cur_line_pos - line_start);
    }

    J_A(AI,ND) inline void begin_line() noexcept {
      this->line_start = cursor;
      ++line_number;
    }
  };
}
#endif
/*!header:re2c:off */

namespace j::lisp::reader {
  namespace {
    J_A(ND,NODESTROY) const double_conversion::StringToDoubleConverter g_converter{
      double_conversion::StringToDoubleConverter::ALLOW_CASE_INSENSITIVITY,
      0.0,
      0.0,
      nullptr,
      nullptr,
      '_'
    };

    template<i64_t Mul, u32_t Prefix>
    [[maybe_unused]] inline u64_t read_num(const char * J_NOT_NULL start,
                                           const char * J_NOT_NULL end,
                                           u64_t (*get_num)(char) noexcept) noexcept {
      const bool is_neg = *start == '-';
      if (is_neg || *start == '+') {
        ++start;
      }
      start += Prefix;
      i64_t result = 0;
      do {
        result *= Mul;
        result += get_num(*start);
        if (*++start == '_') {
          ++start;
          continue;
        }
      } while (start < end);
      return is_neg ? -result : result;
    }

    [[maybe_unused]] inline i64_t read_dec(const char * J_NOT_NULL start,
                                           const char * J_NOT_NULL end) noexcept
    {
      return read_num<10, 0>(start, end, [](char c) noexcept -> u64_t { return c - '0'; });
    }

    [[maybe_unused]] inline i64_t read_hex(const char * J_NOT_NULL start,
                                           const char * J_NOT_NULL end) noexcept
    {
      return read_num<16, 2>(start, end, [](char c) noexcept -> u64_t {
        c |= 0x20;
        return c <= '9' ? c - '0' : c - 'a' + 10;
      });
    }

    [[maybe_unused]] inline u64_t read_bin(const char * J_NOT_NULL start,
                                           const char * J_NOT_NULL end) noexcept
    {
      return read_num<2, 2>(start, end, [](char c) noexcept -> u64_t { return c - '0'; });
    }
  }

  [[nodiscard]] static read_status read_terminated(struct state & state, interactive_reader * reader) {
    [[maybe_unused]] lex_state & lex_state = state.lex_state;

    [[maybe_unused]] const char *before = nullptr, *match = nullptr;
    [[maybe_unused]] char yych;

    /*!getstate:re2c*/
    for (;;) {
      /*!re2c
        re2c:api:style             = free-form;
        re2c:define:YYCTYPE        = "char";
        re2c:define:YYCURSOR       = "lex_state.cursor";
        re2c:define:YYMARKER       = "lex_state.marker";
        re2c:define:YYLIMIT        = "lex_state.limit";
        re2c:define:YYGETSTATE     = "lex_state.re_state";
        re2c:define:YYSETSTATE     = "lex_state.re_state = @@;";
        re2c:define:YYFILL         = "return read_status::waiting;";
        re2c:define:YYGETCONDITION = "lex_state.cond";
        re2c:define:YYSETCONDITION = "lex_state.cond     = @@;";
        re2c:variable:yyaccept     = "lex_state.yyaccept";
        re2c:tags:expression       = "lex_state.@@{tag}";

        // Allow rules to have @var_name to store the position in the variable.
        re2c:flags:tags            = 1;
        re2c:yyfill:enable         = 1;
        re2c:eof                   = 0;

        COMMENT    = ";" [^\r\n]*;
        WHITESPACE = [ \t];
        NEWLINE    = "\r\n" | "\r" | "\n";
        ESC        = "\\";
        QUOTE      = "\"";
        DEC        = [0-9]+ ("_" [0-9]+)*;
        HEXDIGIT   = [0-9a-fA-F];
        HEX        = "0x" HEXDIGIT+ ("_" HEXDIGIT+)*;
        BIN        = "0b" [01]+ ("_" [01]+)*;
        EXP        = 'e' "-"? [0-9]+;

        ID_EDGE    = [^.\x00\r\n:()\[\]{}',`"; \t];
        ID_MIDDLE  = ID_EDGE | ".";
        ID         = ID_EDGE (ID_MIDDLE* ID_EDGE)?;

        POST_CHAR  = [)\];\r\n \t];

        <*> $ { goto success_out; }
        <*> * { goto error_out; }

        <init,post_ellipsis,after_id> WHITESPACE+ | COMMENT :=> init
        <init,post_ellipsis,after_id> NEWLINE => init {
          lex_state.begin_line();
          continue;
        }

        <init> "..." / ( ID_EDGE | [(,`'] ) {
          state.has_pre_ellipsis = true;
          continue;
        }

        <init> [(,'`] {
          state.push_token(lex_state.cursor - 1, state.has_pre_ellipsis);
          state.has_pre_ellipsis = false;
          continue;
        }

        <init, after_id, post_ellipsis> ")" => post_ellipsis {
          state.push_close_paren();
          goto check_top;
        }

        <init, post_ellipsis,after_id> "]" => after_id {
          state.push_close_bracket();
          goto check_top;
        }

        <after_id, post_ellipsis> "..." / POST_CHAR => init {
          state.last_token->has_post_ellipsis = true;
          goto check_top;
        }

        <after_id> "[" => init {
          state.push_open_bracket(lex_state.cursor - 1);
          continue;
        }

        <init> QUOTE :=> string

        // Match 1.23, .23, 1.23e10, 1.24e10, 112e10
        <init> @match [+-]? (DEC? "." DEC EXP? | DEC EXP) / POST_CHAR  {
          int dummy = 0;
          state.push_floating_point(g_converter.StringToFloat(match, lex_state.cursor - match, &dummy));
          goto check_top;
        }

        <init> @match [+-]? HEX / POST_CHAR {
          state.push_integer(read_hex(match, lex_state.cursor));
          goto check_top;
        }
        <init> @match [+-]? BIN / POST_CHAR {
          state.push_integer(read_bin(match, lex_state.cursor));
          goto check_top;
        }
        <init> @match [+-]? DEC / POST_CHAR {
          state.push_integer(read_dec(match, lex_state.cursor));
          goto check_top;
        }

        // Identifiers and keywords. Digits cannot start identifiers, but that is not explicitly
        // handled - digits match the DEC rule above, so those are excluded based on rule ordering.
        <init> @before (ID? ":")? @match ID => after_id {
          state.push_symbol(
            lex_state.save_string(before, before == match ? 0U : match - before - 1),
            lex_state.save_string(match, lex_state.cursor - match),
            *before == ':',
            state.has_pre_ellipsis
          );
          state.has_pre_ellipsis = false;
          continue;
        }

        <string> QUOTE / POST_CHAR => init {
          auto str = lisp_str::allocate(*state.heap, lex_state.str_size, (j::lisp::mem::object_hightag_flag)0, 1U);
          state.push_string(str);
          char * data = str->data();
          for (string_part * cur = lex_state.str_first; cur; cur = cur->next) {
            data = cur->copy_to(data, cur->size);
          }
          lex_state.reset_string();
          goto check_top;
        }

        <string> @match [^"\\\x00]+ {
          lex_state.write_string(match, lex_state.cursor - match);
          continue;
        }

        <string> ESC @match ([trn0] | [xuU] HEXDIGIT{2,8}) {
          u32_t code_point = 0U;
          switch(*match) {
          case '0': break;
          case 't': code_point = '\t'; break;
          case 'r': code_point = '\r'; break;
          case 'n': code_point = '\n'; break;

          case 'x':
            code_point = (char)util::convert_hex_byte(match);
            if (match + 3U != lex_state.cursor) { goto error_out; }
            break;
          case 'u':
            code_point = (char)util::convert_hex_u16_t(match);
            if (match + 5U != lex_state.cursor) { goto error_out; }
            break;
          case 'U':
            code_point = (char)util::convert_hex_u32_t(match);
            break;
          default: J_UNREACHABLE();
          }
          lex_state.write_utf8_code_point(code_point);
          continue;
        }
      */
    [[maybe_unused]] check_top:
      if (reader && state.got_top_level()) {
        reader->on_top_level();
      }
    }

    [[maybe_unused]] error_out:
      state.throw_error();

    [[maybe_unused]] success_out:
      if (reader && state.got_top_level()) {
        reader->on_top_level();
      }
      return read_status::eof;
  }

  lisp_vec * read(env::environment * J_NOT_NULL env, packages::pkg * J_NOT_NULL pkg, sources::source * src) {
    src->open();
    try {
      strings::const_string_view str = src->get_source();
      j::mem::unique_ptr<char[]> buffer;
      const char * cstr = nullptr;
      u32_t sz = str.size();
      if (!str.size()) {
        cstr = "";
      } else if (str.back() == '\0') {
        cstr = str.data();
        --sz;
      } else if (!is_aligned(str.end(), 16U) && *str.end() == '\0') {
        cstr = str.data();
      } else {
        buffer = j::mem::unique_ptr<char[]>(::new char[sz + 1U]);
        ::j::memcpy(buffer.get(), str.data(), sz);
        buffer[sz] = '\0';
        cstr = buffer.get();
      }
      state s(env, pkg, cstr, sz, src->index);
      u8_t attempts = 2;
      read_status result;
      do {
        result = read_terminated(s, nullptr);
      } while (result == read_status::waiting && attempts--);
      auto fn_res = result == read_status::eof ? build_ast(s) : nullptr;
      src->close();
      return fn_res;
    } catch (...) {
      src->close();
      throw;
    }
  }

  read_status read_interactive(interactive_reader & reader) {
    return read_terminated(reader.state, &reader);
  }
}
