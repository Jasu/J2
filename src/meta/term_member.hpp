#pragma once

#include "meta/node.hpp"
#include "meta/basic_node_set.hpp"

namespace j::meta {
  struct val_type;
  struct term_member final : node {
    J_A(AI,ND) inline term_member() noexcept : node(node_term_member) { }

    J_BOILERPLATE(term_member, COPY_ND)

    term_member(strings::const_string_view name, doc_comment && comment, val_region region,
                i8_t ctor_index,
                u8_t region_index,
                val_type * J_NOT_NULL type) noexcept;

    val_type * type = nullptr;
    term_member * dynamic_offset_member = nullptr;

    val_region region = region_none;
    i8_t ctor_index = -1;
    u8_t region_index = 0U;
    bool has_dynamic_size = false;
    u8_t static_offset = 0U;
    basic_node_set allowed_children{};


    [[nodiscard]] attr_value get_dynamic_offset_member() const noexcept;

    J_A(AI,NODISC,ND) bool has_dynamic_offset() const noexcept {
      return dynamic_offset_member;
    }
    [[nodiscard]] u32_t size() const noexcept;
    [[nodiscard]] u32_t align() const noexcept;
    [[nodiscard]] bool is_scalar() const noexcept;

    [[nodiscard]] attr_value ctor_stmt() const noexcept;
    [[nodiscard]] attr_value size_expr() const noexcept;
    [[nodiscard]] attr_value ctor_size_expr() const noexcept;
    [[nodiscard]] attr_value getter_expr() const noexcept;
    [[nodiscard]] attr_value const_getter_expr() const noexcept;

    [[nodiscard]] strings::const_string_view type_name() const noexcept;
    [[nodiscard]] strings::const_string_view ctor_type_name() const noexcept;
    [[nodiscard]] strings::const_string_view getter_type_name() const noexcept;
    [[nodiscard]] strings::const_string_view const_getter_type_name() const noexcept;

    J_A(AI,NODISC) constexpr inline bool is_input() const noexcept {
      return region == region_input;
    }
    J_A(AI,NODISC) constexpr inline bool is_const() const noexcept {
      return region == region_const;
    }
    J_A(AI,NODISC) constexpr inline bool is_data() const noexcept {
      return region == region_data;
    }
  };
}
