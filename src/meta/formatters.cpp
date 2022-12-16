#include "strings/formatters/debug_enum_formatter.hpp"
#include "meta/attr_value.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/styling/default_styles.hpp"

namespace j::meta {
  namespace {
    namespace s = strings;
    namespace st = s::styles;
    namespace f = s::formatters;
    static const f::debug_enum_entry attr_type_entries[] = {
      [attr_type_none]        = { "None",      st::error},
      [attr_int]              = { "Int",       st::cyan},
      [attr_bool]             = { "Bool",      st::cyan},
      [attr_enum]             = { "Enum",      st::cyan},

      [attr_term]             = { "Term",      st::bright_green.with_bold()},
      [attr_term_member]      = { "Member",   st::bright_green.with_bold()},
      [attr_value_type]       = { "ValType",   st::bright_green.with_bold()},
      [attr_attr_def]         = { "AttrDef",   st::bright_green.with_bold()},
      [attr_enum_def]         = { "EnumDef",   st::bright_green.with_bold()},
      [attr_struct_def]       = { "StructDef",   st::bright_green.with_bold()},
      [attr_gen_file_set]      = { "GenFileSet",   st::bright_red.with_bold()},
      [attr_nonterminal] = { "NonTerminal",      st::bright_green.with_bold()},
      [attr_fn]               = { "Fn",        st::bright_red.with_bold()},

      [attr_term_set]         = { "TermSet",    st::bright_cyan.with_bold()},
      [attr_val_type_set]     = { "TypeSet",st::bright_cyan.with_bold()},

      [attr_rule]             = { "Rule",      st::bright_green.with_bold()},


      [attr_id]               = { "Id",    st::bright_magenta},
      [attr_str]              = { "String",    st::bold},
      [attr_tpl]              = { "Template",  st::bright_blue.with_bold()},
      [attr_tuple]            = { "Tuple",     st::bright_magenta.with_bold()},
      [attr_map]            = { "Map",     st::bright_cyan.with_bold()},
      [attr_struct]           = { "Struct",     st::bright_magenta.with_bold()},
    };

    static const f::debug_enum_entry node_type_entries[] = {
      [node_none]             = { "None",    st::error},
      [node_term]             = { "Term",    st::bright_green.with_bold()},
      [node_term_member]      = { "Member",  st::bright_cyan},
      [node_value_type]       = { "Type",    st::bright_blue},

      [node_attr_def]         = { "AttrDef", st::bright_magenta.with_bold()},
      [node_enum]             = { "Enum",    st::red},
      [node_nonterminal]      = { "NonTerminal",    st::bright_yellow.with_bold()},
      [node_gen_file_set]     = { "FileSet", st::green},
      [node_fn]               = { "Fn",      st::bright_red.with_bold()},
      [node_any]              = { "Any",     st::error},
    };
    J_A(ND,NODESTROY) const f::debug_enum_formatter<node_type> node_type_fmt(node_type_entries);
    J_A(ND,NODESTROY) const f::debug_enum_formatter<attr_type> attr_type_fmt(attr_type_entries);

    class J_TYPE_HIDDEN attr_type_mask_formatter final : public s::formatter_known_length<attr_type_mask> {
    public:
      void do_format(
        const s::const_string_view &,
        const attr_type_mask & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        u64_t mask = (u64_t)value;
        if (!mask) {
          target.write_styled(st::bright_red, "Empty");
          return;
        }
        bool did_write = false;
        while (mask) {
          if (did_write) {
            target.write_styled(st::yellow, "|");
          }
          attr_type_fmt.do_format("", (attr_type)(bits::ctz(mask) + 1), target, current_style);
          mask = mask & (mask - 1);
          did_write = true;
        }
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view &, const attr_type_mask & value) const noexcept override {
        u64_t mask = (u64_t)value;
        if (!mask) {
          return 5;
        }
        bool did_write = false;
        u32_t result = 0;
        while (mask) {
          if (did_write) {
            ++result;
          }
          result += attr_type_fmt.do_get_length("", (attr_type)(bits::ctz(mask) + 1));
          mask = mask & (mask - 1);
          did_write = true;
        }
        return result;
      }
    };
    J_A(ND,NODESTROY) const attr_type_mask_formatter attr_type_mask_fmt;
  }
}
