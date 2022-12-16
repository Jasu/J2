#include "args.hpp"
#include "meta/rules/term_expr.hpp"
#include "meta/rules/nonterminal.hpp"
#include "meta/rules/nt_data_type.hpp"
#include "meta/rules/pattern_tree.hpp"

namespace j::meta::inline rules {
  J_A(NODESTROY) const pat_captures pat_captures::empty{};

  void pat_captures::set(i8_t index, const pat_capture & cap, bool overwrite) noexcept {
    J_ASSERT(cap.type != arg_type::none);
    if (index == -1) {
      J_ASSERT(overwrite || !this_capture);
      this_capture = cap;
    } else {
      if (index >= (i8_t)captures.size()) {
        captures.resize(j::max(4, index + 1));
      }
      J_ASSERT(overwrite || !captures[index]);
      captures[index] = cap;
      ++num_used;
    }
  }


  [[nodiscard]] bool pat_captures::operator==(const pat_captures & rhs) const noexcept {
    i32_t used = num_used;
    if (used != rhs.num_used || this_capture != rhs.this_capture) {
      return false;
    }
    const pat_capture * it = rhs.captures.begin();
    for (auto & cap : captures) {
      if (!used--) {
        return true;
      }
      if (cap != *it++) {
        return false;
      }
    }
    return !used;
  }

  [[nodiscard]] pat_capture & pat_captures::at(i32_t i) {
    pat_capture & result = i == -1 ? this_capture : captures.at(i);
    if (!result) {
      exceptions::throw_out_of_range(0, i);
    }
    return result;
  }

  reduction_arg::reduction_arg(i8_t stack_offset, arg_type type, val_type * value_type, u8_t field_offset, nt_p debug_nt) noexcept
    : type(type),
      stack_offset(stack_offset),
      field_offset(field_offset),
      value_type(value_type),
      debug_nt(debug_nt)
  { J_ASSERT(type != arg_type::reference); }

  [[nodiscard]] i32_t reduction_arg::stack_depth() const noexcept {
    J_ASSERT(type < arg_type::reference);
    return type == arg_type::select_term_ahead ? stack_offset : stack_offset + 1;
  }

  void reduction_arg::resolve(const pat_capture & cap) {
    J_ASSERT(cap);
    stack_offset = cap.stack_offset;
    field_offset = cap.field_offset;
    J_REQUIRE(type == arg_type::reference, "Arg was already resolved.");
    type = cap.type;
    J_ASSERT(type != arg_type::none);
    if (type == arg_type::capture_this_reduction_nt || type == arg_type::capture_reduction_copy_nt || type == arg_type::capture_reduction_copy) {
      nt = cap.nt;
      J_ASSERT(nt && field_offset == 0);
    } else {
      value_type = cap.value_type;
    }
    debug_nt = cap.nt;
  }

  reduction_value::reduction_value(const reduction_value & rhs) noexcept
    : expr(copy(rhs.expr)),
      rewrite_action(rhs.rewrite_action),
      rewrite_offset(rhs.rewrite_offset)
  { }

  reduction_value::reduction_value(const reduction_value & rhs, copy_cb_t callback, void * userdata) noexcept
    : expr(copy(rhs.expr, callback, userdata)),
      rewrite_action(rhs.rewrite_action),
      rewrite_offset(rhs.rewrite_offset)
  { }


  reduction_value & reduction_value::operator=(const reduction_value & rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      expr = copy(rhs.expr);
      rewrite_action = rhs.rewrite_action;
      rewrite_offset = rhs.rewrite_offset;
    }
    return *this;
  }

  [[nodiscard]] bool reduction_value::operator==(const reduction_value & rhs) const noexcept {
    return rewrite_action == rhs.rewrite_action && rewrite_offset == rhs.rewrite_offset
      && *expr == *rhs.expr;
  }

  [[nodiscard]] u32_t reduction_value_hash::operator()(const reduction_value & r) const noexcept {
    return r.expr->hash + r.rewrite_offset + (i32_t)r.rewrite_action * 100;
  }

  [[nodiscard]] u32_t reduction_arg_hash::operator()(const reduction_arg & r) const noexcept {
    u32_t result = (u32_t)r.type + 1;
    switch (r.type) {
    case arg_type::none: break;
    case arg_type::reference: result = crc32(result, r.capture_index); break;
    case arg_type::capture_term:
    case arg_type::select_term_ahead:
      result = crc32(result, r.stack_offset << 8);
      break;
    case arg_type::capture_term_field:
    case arg_type::capture_reduction_field:
      result = crc32(result + r.field_offset, (r.stack_offset << 8) ^ (r.value_type ? r.value_type->index : 1));
      break;
    case arg_type::capture_this_reduction_nt:
    case arg_type::capture_reduction_copy_nt:
      result = crc32(result + r.stack_offset, r.nt ? r.nt->index : 1);
      break;
    case arg_type::capture_reduction_copy_sz:
      result = crc32(result + r.stack_offset, r.size);
      break;
    case arg_type::capture_reduction_copy:
      result = crc32(result, r.stack_offset);
      if (r.data_type) {
        result = crc32(result, r.data_type->name.begin(), r.data_type->name.size());
      }
      break;
    }
    return result;
  }

}
