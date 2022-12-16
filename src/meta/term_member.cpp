#include "term_member.hpp"
#include "meta/value_types.hpp"

namespace j::meta {
  term_member::term_member(strings::const_string_view name, doc_comment && comment, val_region region,
                           i8_t ctor_index,
                           u8_t region_index,
                           val_type * J_NOT_NULL type) noexcept
  : node(node_term_member, name, static_cast<doc_comment &&>(comment)),
    type(type),
    region(region),
    ctor_index(ctor_index),
    region_index(region_index),
    has_dynamic_size(type->has_dynamic_size())
  { }

  [[nodiscard]] attr_value term_member::get_dynamic_offset_member() const noexcept {
    return dynamic_offset_member ? attr_value(dynamic_offset_member) : attr_value();
  }
  [[nodiscard]] u32_t term_member::size() const noexcept {
    return type->size;
  }

  [[nodiscard]] u32_t term_member::align() const noexcept {
    return type->align;
  }
  [[nodiscard]] bool term_member::is_scalar() const noexcept {
    return type->is_scalar;
  }
  [[nodiscard]] strings::const_string_view term_member::type_name() const noexcept {
    return type->type_name;
  }
  [[nodiscard]] strings::const_string_view term_member::ctor_type_name() const noexcept {
    return type->ctor_type_name;
  }
  [[nodiscard]] strings::const_string_view term_member::getter_type_name() const noexcept {
    return type->getter_type_name;
  }
  [[nodiscard]] strings::const_string_view term_member::const_getter_type_name() const noexcept {
    return type->const_getter_type_name;
  }
  [[nodiscard]] attr_value term_member::ctor_stmt() const noexcept {
    return type->ctor_stmt;
  }
  [[nodiscard]] attr_value term_member::size_expr() const noexcept {
    return type->size_expr;
  }
  [[nodiscard]] attr_value term_member::ctor_size_expr() const noexcept {
    return type->ctor_size_expr;
  }
  [[nodiscard]] attr_value term_member::getter_expr() const noexcept {
    return type->getter_expr;
  }
  [[nodiscard]] attr_value term_member::const_getter_expr() const noexcept {
    return type->const_getter_expr;
  }
}
