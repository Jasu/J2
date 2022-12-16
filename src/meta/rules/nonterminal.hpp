#pragma once

#include "meta/node.hpp"
#include "containers/vector.hpp"
#include "meta/rules/nt_data_type.hpp"
#include "meta/rules/term_expr.hpp"

namespace j::meta::inline rules {
  struct nonterminal final : node {
    nonterminal(strings::const_string_view name, doc_comment && comment) noexcept;

    nt_data_type_p data_type{};
    vector<nt_data_field> fields{};
    pat_p pat{};

    void add_pat(pat_p && pat) noexcept;
    [[nodiscard]] attr_value make_accessor_map(strings::const_string_view struct_ptr) const noexcept;

    J_A(AI,NODISC) inline u32_t num_fields() const noexcept { return fields.size(); }
    [[nodiscard]] u32_t field_index(strings::const_string_view name) const;
    [[nodiscard]] strings::const_string_view field_name(i32_t i) const;
    [[nodiscard]] const nt_data_field & field_at(strings::const_string_view name) const;
    [[nodiscard]] const nt_data_field & field_at_offset(i32_t offset) const;
    [[nodiscard]] const nt_data_field & field_by_tracking_index(i32_t offset) const;
    [[nodiscard]] const nt_data_field * field_maybe_at(strings::const_string_view name) const noexcept;

    J_A(AI,NODISC,ND) inline bool has_lifetime_tracking() const noexcept {
      return data_type && data_type->num_tracked_fields;
    }

    J_A(AI,NODISC,ND) inline i32_t num_tracked_fields() const noexcept {
      return data_type ? data_type->num_tracked_fields : 0;
    }
    J_A(AI,NODISC,ND) inline i32_t max_tracking_index() const noexcept {
      return data_type ? data_type->max_tracking_index : 0;
    }
  };

  struct nonterminal_hash final {
    J_A(AI,ND,NODISC) inline u32_t operator()(nt_p J_NOT_NULL t) const noexcept {
      return crc32(17, t->index);
    }
  };
}
