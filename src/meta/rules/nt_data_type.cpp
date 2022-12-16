#include "nt_data_type.hpp"
#include "containers/hash_map.hpp"
#include "meta/rules/nonterminal.hpp"
#include "strings/format.hpp"

namespace s = j::strings;

J_DEFINE_EXTERN_HASH_SET(j::meta::nt_data_type_p, HASH(j::meta::nt_data_type_hash), KEYEQ(j::meta::nt_data_type_hash), CONST_KEY(j::meta::nt_data_type_const_key));

namespace j::meta::inline rules {
  nt_data_field::nt_data_field(val_type * J_NOT_NULL type, strings::const_string_view name, term_expr * init, term_expr * release_extra_arg, i32_t tracking_index, i32_t track_index) noexcept
    : name(name),
      type(type),
      has_lifetime_tracking(type->has_lifetime_tracking()),
      tracking_index(tracking_index),
      track_index(track_index),
      initializer(init),
      release_extra_arg(release_extra_arg)
  {
    J_ASSERT((track_index == -1) == (tracking_index == -1));
  }

  void nt_data_field::combine(const nt_data_field & rhs, i32_t index) noexcept {
    J_ASSERT(type == rhs.type);
    J_ASSERT(tracking_index == rhs.tracking_index);
    if (name != rhs.name) {
      set_default_name(index);
    }
  }

  void nt_data_field::set_default_name(i32_t index) noexcept {
    name = s::format(type->default_name_expr ? type->default_name_expr : "f{}", index);
  }

  nt_data_type::nt_data_type(strings::const_string_view name) noexcept
    : name(name)
  { }

  void nt_data_type::combine(const nt_data_type & rhs) noexcept {
    J_ASSERT(fields.size() == rhs.fields.size());
    auto other = rhs.fields.begin(), end = rhs.fields.end();
    u32_t i = 0U;
    for (auto & field : fields) {
      J_ASSERT(other != end);
      field.combine(*other++, i++);
    }
  }

  void nt_data_type::add_field(val_type * J_NOT_NULL type, strings::const_string_view name, term_expr * init, term_expr * release_extra_arg, i32_t tracking_index) noexcept {
    fields.emplace_back(type, name, init, release_extra_arg, tracking_index, tracking_index >= 0 ? num_tracked_fields : -1);
    if (tracking_index >= 0) {
      max_tracking_index = max(max_tracking_index, tracking_index);
      ++num_tracked_fields;
    }
  }

[[nodiscard]] bool nt_data_type::layout_eq(const nt_data_type & rhs) const noexcept {
  if (fields.size() != rhs.fields.size()) {
    return false;
  }
  auto it = rhs.fields.begin();
  for (auto & field : fields) {
    if (it->type != field.type || it->tracking_index != field.tracking_index) {
      return false;
    }
    ++it;
  }
  return true;
}

[[nodiscard]] const nt_data_field & nt_data_type::field_at(strings::const_string_view name) const {
  for (u32_t i = 0U, e = fields.size(); i != e; ++i) {
    if (fields[i].name == name) {
      return fields[i];
    }
  }
  J_THROW("Field {} not found in {}.", name, this->name);
}

[[nodiscard]] const nt_data_field * nt_data_type::field_maybe_at(strings::const_string_view name) const noexcept {
  for (u32_t i = 0U, e = fields.size(); i != e; ++i) {
    if (fields[i].name == name) {
      return &fields[i];
    }
  }
  return nullptr;
}

s::const_string_view nt_data_type::field_name(u32_t index) const {
  return fields.at(index).name;
}

u32_t nt_data_type::field_index(strings::const_string_view name) const {
  for (u32_t i = 0U, e = fields.size(); i != e; ++i) {
    if (fields[i].name == name) {
      return i;
    }
  }
  J_THROW("Field {} not found in {}.", name, this->name);
}

[[nodiscard]] bool nt_data_type_hash::operator()(const nt_data_type_p & lhs, const nt_data_type & rhs) const noexcept {
  return lhs->layout_eq(rhs);
}

[[nodiscard]] bool nt_data_type_hash::operator()(const nt_data_type_p & lhs, const nt_data_type_p & rhs) const noexcept {
  return lhs->layout_eq(*rhs);
}

[[nodiscard]] u32_t nt_data_type_hash::operator()(const nt_data_type & t) const noexcept {
  u32_t result = 17U;
  for (auto & f : t.fields) {
    result = crc32(result, f.type->index);
  }
  return result;
}

[[nodiscard]] u32_t nt_data_type_hash::operator()(const nt_data_type_p & s) const noexcept {
  return operator()(*s);
}

void nt_data_types::set_data_type(nt_p J_NOT_NULL nt, const nt_data_type & type) {
    if (!type) {
      return;
    }
    J_ASSERT(!nt->data_type);
    nt->fields = type.fields;
    nt->data_type = add(type);
    auto * to = nt->fields.begin();
    for (auto & from : nt->data_type->fields) {
      to++->offset = from.offset;
    }
  }

  namespace {
    const char * const chars = "abcdefghijklmnopqrstuvwxyz";
  }

  [[nodiscard]] nt_data_type_p nt_data_types::add(const nt_data_type & type) noexcept {
    if (!type) {
      return {};
    }
    if (auto * t = hash.maybe_at(type)) {
      J_ASSERT((*t)->name != type.name);
      i32_t num_fields = (*t)->num_fields();
      s::string name = s::format("nt_captures_{}", num_fields);
      i32_t i = 0;
      while (type_maybe_at(name)) {
        name = s::format("nt_captures_{}{}", num_fields, chars[i++]);
      }
      (*t)->name = name;
      (*t)->combine(type);
      return *t;
    }
    auto result = mem::make_shared<nt_data_type>(type);
    result->index = types.size();
    hash.emplace(result);
    types.emplace_back(result);
    i32_t cur_offset = 0U;
    for (auto & f : result->fields) {
      i32_t sz = f.type->size;
      J_ASSERT(sz);
      i32_t align = f.type->align ? f.type->align : f.type->size;
      J_ASSERT(align);
      cur_offset = align_up(cur_offset, align);
      f.offset = cur_offset;
      cur_offset += sz;
    }
    result->size = align_up(cur_offset, 8);
    return static_cast<nt_data_type_p &&>(result);
  }
    [[nodiscard]] nt_data_type_p nt_data_types::type_maybe_at(strings::const_string_view name) noexcept {
      for (const nt_data_type_p & p : types) {
        if (p->name == name) {
          return p;
        }
      }
      return {};
    }

  nt_data_types::nt_data_types() noexcept {}
}
