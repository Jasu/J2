#include "meta/rules/nonterminal.hpp"
#include "meta/dump.hpp"
#include "meta/rules/pattern_tree.hpp"
#include "strings/string_map.hpp"
#include "strings/format.hpp"
#include "strings/styling/default_styles.hpp"

namespace j::meta::inline rules {
  namespace s = strings;
  namespace st = strings::styles;

  nonterminal::nonterminal(s::const_string_view name, doc_comment && comment) noexcept
    : node(node_nonterminal, name, static_cast<doc_comment &&>(comment))
  { }

  void dump(dump_context & ctx, const nonterminal & value) noexcept {
    comment_writer cw{ctx.to, hash_comment};
    cw.write_doc_comment(value.comment, {
      .title.prefix = s::format("NonTerminal #{} - ", value.index),
    });
    ctx.to.clear_section_break();
    ctx.to.begin_line();
    ctx.to.write("NonTerminal ", st::magenta);
    ctx.to.write(value.name, st::bright_magenta.with_bold());
    ctx.to.write("(", st::white.with_bold());
    {
      bool is_first = true;
      for (auto & field : value.fields) {
        cw.reset();
        if (!is_first) {
          ctx.to.write(", ");
        }
        is_first = false;
        ctx.to.write_formatted("{#bright_yellow,bold}{}{/}", field.name);
      }
    }
    ctx.to.queue_break();
  }
  [[nodiscard]] u32_t nonterminal::field_index(strings::const_string_view name) const {
    u32_t i = 0;
    for (auto & f : fields) {
      if (f.name == name) {
        return i;
      }
      ++i;
    }
    J_THROW("Field {} not found in {}", name, this->name);
  }

  [[nodiscard]] const nt_data_field & nonterminal::field_at(strings::const_string_view name) const {
    for (auto & f : fields) {
      if (f.name == name) {
        return f;
      }
    }
    J_THROW("Field {} not found in {}", name, this->name);
  }

  [[nodiscard]] const nt_data_field & nonterminal::field_at_offset(i32_t offset) const {
    for (auto & f : fields) {
      if (f.offset == offset) {
        return f;
      }
    }
    J_THROW("Field not found at offset {} for {}", offset, name);
  }

  [[nodiscard]] const nt_data_field & nonterminal::field_by_tracking_index(i32_t index) const {
    for (auto & f : fields) {
      if (f.tracking_index == index) {
        return f;
      }
    }
    J_THROW("Field not found for tracking index {} for {}", index, name);
  }

  [[nodiscard]] const nt_data_field * nonterminal::field_maybe_at(strings::const_string_view name) const noexcept {
    for (auto & f : fields) {
      if (f.name == name) {
        return &f;
      }
    }
    return nullptr;
  }

  [[nodiscard]] strings::const_string_view nonterminal::field_name(i32_t i) const {
    return fields.at(i).name;
  }

  void nonterminal::add_pat(pat_p && pat) noexcept {
    this->pat = add_alternate(static_cast<pat_p &&>(this->pat), static_cast<pat_p &&>(pat));
  }


  [[nodiscard]] attr_value nonterminal::make_accessor_map(strings::const_string_view struct_ptr) const noexcept {
    i32_t sz = fields.size();
    attr_map_t result(sz);
    for (i32_t i = 0; i < sz; ++i) {
      result.emplace(fields[i].name, struct_ptr + "->" + data_type->fields[i].name);
    }
    return attr_value(static_cast<attr_map_t &&>(result));
  }
}
