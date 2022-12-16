#include "lisp/mem/heap_cell.hpp"
#include "lisp/sources/debug_formatters.hpp"
#include "lisp/values/debug_formatters.hpp"
#include "strings/formatters/enum_formatter.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::lisp::mem {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;

    J_A(ND, NODESTROY) const f::enum_formatter<lowtag_type> g_type_formatter{{
      {lowtag_type::footer,        "╰┄footer ", s::styles::light_gray},
      {lowtag_type::free_chunk,    "free     ", s::styles::bright_green.with_bold()},
      {lowtag_type::sentinel,      "SENTINEL ", s::styles::bright_red},
      {lowtag_type::debug_info,    "╭┄debug  ", s::styles::bright_magenta},
      {lowtag_type::str,           "String   ", s::styles::bright_red},
      {lowtag_type::vec,           "Vec      ", s::styles::bright_red},
      {lowtag_type::act_record,    "Act rec  ", s::styles::bright_yellow},
    }};

    class J_TYPE_HIDDEN heap_ctrl_value_formatter final : public s::formatter_known_length<heap_ctrl_value, heap_footer, heap_free_chunk, heap_sentinel> {
    public:
      void do_format(
        const s::const_string_view &,
        heap_ctrl_value const & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        g_type_formatter.do_format("", value.type(), target, current_style);
        f::integer_formatter_v<u32_t>.do_format("04X", value.size_taken(), target, current_style);
        target.write(" ");
        f::integer_formatter_v<u64_t>.do_format("016X", *reinterpret_cast<const u64_t *>(&value), target, current_style);
        switch (value.type()) {
        case lowtag_type::footer:
          target.write(" -");
          f::integer_formatter_v<u32_t>.do_format("04X", value.as_footer().offset_qwords() * 8U, target, current_style);
          break;
        case lowtag_type::free_chunk:
          target.write(" @0x");
          f::integer_formatter_v<u64_t>.do_format("012X", (u64_t)&value, target, current_style);
          target.write(" Next free -> 0x");
          f::integer_formatter_v<u64_t>.do_format("012X",
                                         (u64_t)value.as_free_chunk().next_free(),
                                         target, current_style);
          break;
        case lowtag_type::sentinel: {
          bool is_begin = value.as_sentinel().is_begin();
          target.write_styled(is_begin ? s::styles::green : s::styles::red, is_begin ? " Begin" : " End");
        }
          break;
        case lowtag_type::vec:
        case lowtag_type::str:
        case lowtag_type::act_record:
          break;
        case lowtag_type::debug_info: {
          auto & dbg = value.as_debug_info();
          switch (dbg.debug_info_type()) {
          case debug_info_type::source_location: {
            auto & loc = dbg.source_location();
            target.write(" ");
            sources::g_source_location_formatter.do_format("", loc, target, current_style);
            break;
          }
          }
          break;
        }
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const heap_ctrl_value & value
      ) const noexcept override {
        u32_t result = g_type_formatter.do_get_length("", value.type())
          + f::integer_formatter_v<u32_t>.do_get_length("04X", value.size_taken())
          + 17;
        switch (value.type()) {
        case lowtag_type::footer:
          result += 2U + f::integer_formatter_v<u32_t>.do_get_length("04X", value.as_footer().offset_qwords() * 8U);
          break;
        case lowtag_type::free_chunk:
          result += 4U
            + f::integer_formatter_v<u64_t>.do_get_length("012X", (u64_t)&value)
            + 16U
            + f::integer_formatter_v<u64_t>.do_get_length("012X", (u64_t)value.as_free_chunk().next_free());
          break;
        case lowtag_type::sentinel:
          result += value.as_sentinel().is_begin() ? 6U : 4U;
          break;
        case lowtag_type::str:
        case lowtag_type::vec:
        case lowtag_type::act_record:
          break;
        case lowtag_type::debug_info: {
          auto & dbg = value.as_debug_info();
          switch (dbg.debug_info_type()) {
          case debug_info_type::source_location: {
            auto & loc = dbg.source_location();
            result += 1U + sources::g_source_location_formatter.do_get_length("", loc);
            break;
          }
          }
          break;
        }
        }
        return result;
      }
    };

    J_A(ND, NODESTROY) const heap_ctrl_value_formatter g_heap_ctrl_value_formatter;

    class J_TYPE_HIDDEN heap_cell_formatter final : public s::formatter_known_length<heap_cell> {
      void do_format(
        const s::const_string_view &,
        heap_cell const & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        if (value.is_ctrl()) {
          g_heap_ctrl_value_formatter.do_format("", value.as_ctrl(), target, current_style);
        } else {
          auto & o = value.as_object();
          g_type_formatter.do_format("", value.header.lowtag.type(), target, current_style);
          f::integer_formatter_v<u32_t>.do_format("04X", o.size_taken(), target, current_style);
          target.write(" ");
          g_lisp_object_formatter.do_format("", o, target, current_style);
          if (!o.header.object_hightag().is_gc_managed()) {
            target.write_styled(s::styles::bright_red, " Error: not GC-managed.");
          }
          if (o.header.object_hightag().has_debug_info()) {
            target.write_styled(s::styles::cyan, " Dbg info");
          }
          if (o.header.object_hightag().has_mark()) {
            target.write_styled(s::styles::yellow, " Mark");
          }
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const heap_cell & value
      ) const noexcept override {
        if (value.is_ctrl()) {
          return g_heap_ctrl_value_formatter.do_get_length("", value.as_ctrl());
        } else {
          auto & o = value.as_object();
          return g_type_formatter.do_get_length("", value.header.lowtag.type())

            + f::integer_formatter_v<u32_t>.do_get_length("04X", o.size_taken())
            + 1U
            + g_lisp_object_formatter.do_get_length("", o)
            + (!o.header.object_hightag().is_gc_managed() ? 23U : 0U)
            + (o.header.object_hightag().has_debug_info() ? 9U : 0U)
            + (o.header.object_hightag().has_mark() ? 5U : 0U);
        }
      }
    };

    J_A(ND, NODESTROY) const heap_cell_formatter g_heap_cell_formatter;
  }
}
