#include <mem/memory_region.hpp>
#include <mem/memmap.hpp>
#include <hzd/mem.hpp>
#include <util/hex.hpp>
#include <containers/trivial_array.hpp>

J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::mem::memory_mapping_info);

/*!types:re2c*/
/*!maxnmatch:re2c*/
/*!rules:re2c

  re2c:define:YYCURSOR = pos.cursor;
  re2c:define:YYMARKER = pos.marker;
  re2c:define:YYCTYPE  = char;

  // Allow rules to have @var_name to store the position in the variable.
  re2c:flags:tags = 1;

  // The buffer to be parsed is the whole input + 0-terminator.
  // (0-terminator is implied by the lack of re2c:eof)
  re2c:yyfill:enable = 0;
 */

namespace j::mem {
  namespace {
    template<u64_t Mul>
    [[maybe_unused]] inline u64_t read_num(const char * J_NOT_NULL start,
                                           const char * J_NOT_NULL end,
                                           u64_t (*get_num)(char) noexcept) noexcept {
      J_ASSUME(start < end);
      u64_t result = 0;
      do {
        result *= Mul;
        result += get_num(*start++);
      } while (start < end);
      return result;
    }

    [[maybe_unused]] inline u64_t read_hex(const char * J_NOT_NULL start,
                                           const char * J_NOT_NULL end) noexcept
    {
      return read_num<16>(start, end, [](char c) noexcept -> u64_t {
        return c <= '9' ? c - '0' : c - 'a' + 10;
      });
    }

    [[maybe_unused]] inline u64_t read_dec(const char * J_NOT_NULL start,
                                           const char * J_NOT_NULL end) noexcept
    {
      return read_num<10>(start, end, [](char c) noexcept -> u64_t {
        return c - '0';
      });
    }
  }


struct J_TYPE_HIDDEN reader_pos final {
  J_ALWAYS_INLINE constexpr explicit reader_pos(const char * J_NOT_NULL buf) noexcept
    : cursor(buf),
      marker(buf)
  { }

    const char *cursor, *marker;
  };

  struct J_TYPE_HIDDEN parsed_info final {
    template<typename... Args>
      explicit parsed_info(Args && ... args) noexcept
      : info(static_cast<Args &&>(args)...)
    { }

    memory_mapping_info info;
    parsed_info * next = nullptr;
  };


  trivial_array<memory_mapping_info> parse_mem_map(const char * J_NOT_NULL c) {
    parsed_info * parsed_head = nullptr;
    [[maybe_unused]] parsed_info * parsed_tail = nullptr;
    u32_t num_parsed = 0U;

    reader_pos pos(c);
    [[maybe_unused]] const char
                       *begin_begin = nullptr,
                       *begin_end = nullptr,
                       *end_begin = nullptr,
                       *end_end = nullptr,
                       *perms = nullptr,
                       *offset_begin = nullptr,
                       *offset_end = nullptr,
                       *dev_major = nullptr,
                       *dev_minor = nullptr,
                       *inode_begin = nullptr,
                       *inode_end = nullptr,
                       *path_begin = nullptr,
                       *path_end = nullptr;
    [[maybe_unused]] int dummy = 0;
    /*!stags:re2c format = '[[maybe_unused]] const char *@@ = nullptr;'; */
    for (;;) {
      /*!use:re2c
        HEXDIGIT             = [0-9a-f];
        DECDIGIT             = [0-9];

        HEX      = HEXDIGIT+;
        PERMS    = [r-][w-][x-][ps];
        HEX2     = HEXDIGIT{2};
        DEC      = DECDIGIT+;
        WS       = [ \t]+;
        ANY      = [^\x00\n];
        NL       = [\n];

        @begin_begin HEX @begin_end "-" @end_begin HEX @end_end WS
        @perms PERMS WS
        @offset_begin HEX @offset_end WS
        @dev_major HEX2 ":" @dev_minor HEX2 WS
        @inode_begin DEC @inode_end [ \t]*
        @path_begin ANY* @path_end NL
        {
          parsed_info * const info = J_ALLOCA_NEW(parsed_info)(
            (const void*)read_hex(begin_begin, begin_end),
            (const void*)read_hex(end_begin, end_end),
            read_hex(offset_begin, offset_end),
            read_dec(inode_begin, inode_end),
            perms[0] == 'r',
            perms[1] == 'w',
            perms[2] == 'x',
            perms[3] == 'p',
            util::convert_hex_byte(dev_major),
            util::convert_hex_byte(dev_minor),
            strings::string(path_begin, path_end));
          if (parsed_tail) {
            parsed_tail->next = info;
          } else {
            parsed_head = info;
          }
          ++num_parsed;
          parsed_tail = info;
          continue;
        }

        // EOF rule
        [\x00] { goto exit; }
      */
    }
  [[maybe_unused]] exit:
    trivial_array<memory_mapping_info> result(uninitialized, num_parsed);
    for (; parsed_head; parsed_head = parsed_head->next) {
      result.initialize_element(static_cast<memory_mapping_info &&>(parsed_head->info));
    }
    return result;
  }


}
