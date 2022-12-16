#include "mem/debug/memory_mapping_info_formatter.hpp"
#include "mem/memmap.hpp"
#include "strings/formatters/pointers.hpp"
#include "strings/formatters/integers.hpp"
#include "files/paths/path.hpp"
#include "strings/formatters/strings.hpp"
#include "processes/process_info.hpp"
#include "strings/styling/default_styles.hpp"
#include "colors/default_colors.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/string_algo.hpp"
#include "mem/memory_region.hpp"
#include "mem/debug/styles.hpp"

namespace j::mem::debug {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;
    namespace c = colors::colors;
    namespace st = s::styles;

    class memory_mapping_info_formatter final : public s::formatter_known_length<memory_mapping_info> {
    public:
      void do_format(
        const s::const_string_view &,
        const memory_mapping_info & v,
        s::styled_sink & out,
        s::style cs
      ) const override {

        s::style reg_style = styles::reg_anonymous;
        if (v.path()) {
          if (v.path()[0] == '[') {
            if (v.path() == "[heap]") {
              reg_style = styles::reg_heap;
            } else if (v.path() == "[stack]") {
              reg_style = styles::reg_stack;
            } else if (v.path() == "[vvar]") {
              reg_style = styles::reg_vvar;
            } else if (v.path() == "[vdso]") {
              reg_style = styles::reg_vdso;
            } else if (v.path() == "[vsyscall]") {
              reg_style = styles::reg_vsyscall;
            }
          } else if (strings::starts_with(v.path(), strings::const_string_view("/usr/lib"))
                     || strings::starts_with(v.path(), strings::const_string_view("/lib"))) {
            reg_style = styles::reg_system_lib;
          } else if (strings::starts_with(v.path(), strings::const_string_view("/usr/share"))) {
            reg_style = styles::reg_system_data;
          } else if (strings::starts_with(v.path(), strings::const_string_view("/var/cache/"))) {
            reg_style = styles::reg_system_cache;
          } else if (strings::starts_with(v.path(), strings::const_string_view("anon_inode:"))) {
            reg_style = styles::reg_anon_inode;
          } else if (strings::starts_with(v.path(), strings::const_string_view("/memfd:"))) {
            reg_style = styles::reg_memfd;
          } else if (v.path() == processes::get_process_executable(processes::get_self())) {
            reg_style = styles::reg_self;
          }
        }

        out.set_style(reg_style);
        f::g_void_star_formatter.do_format("", v.begin(), out, reg_style);
        out.set_style(cs);

        out.write(" ");
        out.write_styled(
          v.is_readable() ? styles::flag_readable : styles::flag_unreadable,
          v.is_readable() ? "R" : "-");

        out.write_styled(
          v.is_writable()
            ? (v.is_executable() ? styles::flag_wx : styles::flag_writable)
            : styles::flag_unwritable,
          v.is_writable() ? "W" : "-");

        out.write_styled(
          v.is_executable()
            ? (v.is_writable() ? styles::flag_wx : styles::flag_executable)
            : styles::flag_unexecutable,
          v.is_executable() ? "X" : "-");

        out.write_styled(
          v.is_private() ? styles::flag_private : styles::flag_shared,
          v.is_private() ? "P" : "S");

        out.write(" [");
        const sz_t num_pages = byte_offset(v.begin(), v.end()) / J_PAGE_SIZE;
        f::integer_formatter_v<u32_t>.do_format("8", num_pages, out, cs);
        out.write(num_pages > 1 ? " pages]" : " page] ");

        if (v.path()) {
          out.write(" ");
          out.set_style(reg_style);
          f::g_string_formatter.do_format("", v.path(), out, reg_style);
          out.set_style(cs);
        }

        if (!v.offset() && !v.inode()) {
          return;
        }

        out.write(" (");
        if (v.offset()) {
          out.write("offset ");
          f::integer_formatter_v<u64_t>.do_format("", v.offset(), out, cs);
        }
        if (v.inode()) {
          if (v.offset()) {
            out.write(", ");
          }
          out.write("inode ");
          f::integer_formatter_v<u64_t>.do_format("", v.inode(), out, cs);
        }
        out.write(")");
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view &, const memory_mapping_info & v) const noexcept override {
        u32_t result = 0U;

        result += f::g_void_star_formatter.do_get_length("", v.begin());

        result += 5U;
        if (!v.offset() && !v.inode()) {
          return result;
        }

        result += 2U;
        const sz_t num_pages = byte_offset(v.begin(), v.end()) / J_PAGE_SIZE;
        result += f::integer_formatter_v<u32_t>.do_get_length("8", num_pages);
        result += num_pages > 1 ? 7U : 6U;

        if (v.path()) {
          result += 1U + f::g_string_formatter.do_get_length("", v.path());
        }

        result += 2U;
        if (v.offset()) {
          result += 7U;
          result += f::integer_formatter_v<u64_t>.do_get_length("", v.offset());
        }
        if (v.inode()) {
          if (v.offset()) {
            result += 2U;
          }
          result += 6U;
          result += f::integer_formatter_v<u64_t>.do_get_length("", v.inode());
        }
        result += 1U;
        return result;
      }
    };

    J_A(ND, NODESTROY) const memory_mapping_info_formatter g_mapping_formatter;
  }
  const strings::formatter_known_length<memory_mapping_info> & g_memory_mapping_info_formatter = g_mapping_formatter;
}
