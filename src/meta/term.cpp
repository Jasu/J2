#include "term.hpp"
#include "meta/value_types.hpp"

namespace j::meta {
  term::term(strings::const_string_view name, doc_comment && comment) noexcept
    : node{node_term, name, static_cast<doc_comment &&>(comment)}
  { }

  void term::apply_defaults(const term & from) noexcept {
    comment.apply_defaults(from.comment);
    num_inputs = from.num_inputs;
    num_consts = from.num_consts;
    num_data = from.num_data;
    attrs = from.attrs;
    for (u8_t i = 0U; i < num_inputs + num_consts + num_data; ++i) {
      ctor[i] = ::new term_member(*from.ctor[i]);
    }
  }

  [[nodiscard]] bool term::has_member(strings::const_string_view name) const noexcept {
    for (auto t : all()) {
      if (t->name == name) {
        return true;
      }
    }
    return false;
  }
  J_A(RNN,NODISC) term_member * term::get_member(strings::const_string_view name) noexcept {
    for (auto t : all()) {
      if (t->name == name) {
        return t;
      }
    }
    J_THROW("Term {} has no member {}", this->name, name);
  }

  J_A(RNN,NODISC) term_member * term::add_member(strings::const_string_view name,
                                            doc_comment && comment,
                                            val_region region,
                                            val_type * J_NOT_NULL type)
  {
    u8_t * region_index;
    i8_t ctor_index = -1;
    switch (region) {
    case region_input:
      J_ASSERT(!num_data, "Tried to add input with data present.");
      region_index = &num_inputs;
      ctor_index = num_inputs + num_consts;
      break;
    case region_const:
      J_ASSERT(!num_data, "Tried to add const with data present.");
      region_index = &num_consts;
      ctor_index = num_inputs + num_consts;
      break;
    case region_data:
      region_index = &num_data;
      break;
    default:
      J_THROW("Unsupported member region.");
    }

    term_member * result = ctor[num_inputs + num_consts + num_data] = ::new term_member(
      name,
      static_cast<doc_comment &&>(comment),
      region,
      ctor_index,
      *region_index,
      type);

    ++*region_index;

    return result;
  }

  void term::initialize(const module * J_NOT_NULL mod) {
    const u8_t sz = num_consts + num_inputs + num_data;
    term_member **wr_in = storage;
    term_member **wr_const = wr_in + num_inputs;
    term_member **wr_data = wr_const + num_consts;
    for (u8_t i = 0; i < sz; ++i) {
      term_member * cur_member = ctor[i];
      switch (cur_member->region) {
      case region_input:
        *wr_in++    = cur_member;
        has_dynamic_input |= cur_member->has_dynamic_size;
        static_input_size += cur_member->size();
        if (!cur_member->allowed_children) {
          cur_member->allowed_children = basic_node_set::all(mod, node_term);
        }
        break;
      case region_const:
        has_dynamic_const |= cur_member->has_dynamic_size;
        static_const_size += cur_member->size();
        *wr_const++ = cur_member;
        J_ASSERT(!cur_member->allowed_children);
        break;
      case region_data:
        *wr_data++  = cur_member;
        has_dynamic_data |= cur_member->has_dynamic_size;
        static_data_size += cur_member->size();
        J_ASSERT(!cur_member->allowed_children);
        break;
      default:
        J_THROW("Unsupported member region.");
      }
    }

    u32_t static_offset = 0U;
    term_member * dyn_member = nullptr;
    bool require_last = false;
    for (auto s : stored()) {
      J_ASSERT(!require_last);
      J_ASSERT(s->align(), "No align in {}", s->type->name);
      static_offset = align_up(static_offset, s->align());
      s->static_offset = static_offset;
      if (dyn_member) {
        s->dynamic_offset_member = dyn_member;
      }
      if (s->has_dynamic_size) {
        if (dyn_member) {
          require_last = true;
        }
        dyn_member = s;
      } else {
        static_offset += s->size();
      }
    }
    J_ASSERT(static_offset < 255);
    static_size = static_offset;
    has_dynamic_size = has_dynamic_input || has_dynamic_const || has_dynamic_data;
  }

  bool term::layout_equals(const term & other) const noexcept {
    if (static_size != other.static_size
        || has_dynamic_input != other.num_inputs || has_dynamic_const != other.has_dynamic_const
        || has_dynamic_data != other.has_dynamic_data
        || num_inputs != other.num_inputs || num_data != other.num_data || num_consts != other.num_consts) {
      return false;
    }
    for (u8_t i = num_inputs, max = i + num_consts + num_data; i != max; ++i) {
      if (storage[i]->type != other.storage[i]->type) {
        return false;
      }
    }
    return true;
  }
}
