#include "attr_context.hpp"
#include "exceptions/assert.hpp"
#include "strings/string_map.hpp"

J_DEFINE_EXTERN_STRING_MAP(u8_t);

namespace j::meta {
  namespace s = strings;
  enum_def::enum_def(bool is_flag_enum, strings::const_string_view name, doc_comment && comment)
    : node(node_enum, name, static_cast<doc_comment &&>(comment)),
      is_flag_enum(is_flag_enum)
  {
  }

  enum_def::enum_def(bool is_flag_enum, strings::const_string_view name, const strings::const_string_view * J_NOT_NULL first, i32_t size) noexcept
    : node(node_enum, name),
      is_flag_enum(is_flag_enum)
  {
    while (size--) {
      add(*first++);
    }
  }

  [[nodiscard]] u64_t enum_def::at(s::const_string_view name) const {
    for (auto & e : entries) {
      if (e.name == name) {
        return e.value;
      }
    }
    J_THROW("Enum {} does not contain value \"{}\".", this->name, name);
  }

  [[nodiscard]] attr_value enum_def::value(strings::const_string_view name) const noexcept {
    return attr_value(this, at(name));
  }


  [[nodiscard]] bool enum_def::has(s::const_string_view name) const noexcept {
    for (auto & e : entries) {
      if (e.name == name) {
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] u32_t enum_def::size() const noexcept {
    return entries.size();
  }

  [[nodiscard]] strings::string enum_def::maybe_get_enumerant_name(u64_t value, bool qualified) const noexcept {
    for (auto & e : entries) {
      if (e.value == value) {
        return qualified ? name + e.name : e.name;
      }
    }
    return {};
  }

  [[nodiscard]] strings::const_string_view enum_def::get_unqualified_name(u64_t value) const {
    for (auto & e : entries) {
      if (e.value == value) {
        return e.name;
      }
    }
    J_THROW("No enumerant found for {} in {}.", value, this->name);
  }

  [[nodiscard]] strings::string enum_def::get_name(u64_t value, bool qualified) const {
    auto n = maybe_get_enumerant_name(value, qualified);
    if (n) {
      return n;
    }

    J_REQUIRE(is_flag_enum, "No name for {} in {}", value, name);
    if (!value) {
      return qualified ? name + ".None" : "None";
    }
    strings::string result;
    do {
      u64_t cur = value ^ (value & value - 1);
      n = maybe_get_enumerant_name(cur, qualified);
      J_REQUIRE(n, "Flag {} not present in {}.", cur, name);
      if (result) {
        result.push_back('|');
      }
      result += n;
      value ^= cur;
    } while (value);
    return result;
  }

  enum_def & enum_def::add(s::const_string_view name, u64_t value) {
    J_ASSERT(name && name.front() >= 'A' && name.front() <= 'Z');
    for (auto & e : entries) {
      if (e.name == name) {
        J_THROW("Duplicate enumerator \"{}\" in {}.", name, this->name);
      }
    }
    entries.push_back(enum_entry{name, value, this});
    return *this;
  }

  enum_def & enum_def::add(s::const_string_view name) {
    return add(name, is_flag_enum ? 1UL << entries.size() : (u64_t)entries.size());
  }

  struct_def::struct_def(strings::const_string_view name, doc_comment && comment)
    : node(node_struct_def, name, static_cast<doc_comment &&>(comment))
  { }


  [[nodiscard]] u32_t struct_def::index_of(strings::const_string_view name) const {
    for (u32_t i = 0U, sz = fields.size(); i != sz; ++i) {
      if (fields[i] == name) {
        return i;
      }
    }
    J_THROW("Field \"{}\" does not exist in struct \"{}\".", name, this->name);
  }
  [[nodiscard]] bool struct_def::has(strings::const_string_view name) const noexcept {
    for (u32_t i = 0U, sz = fields.size(); i != sz; ++i) {
      if (fields[i] == name) {
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] u32_t struct_def::size() const noexcept {
    return fields.size();
  }
  void struct_def::push_back(strings::const_string_view name) {
    fields.push_back(name);
  }

  attr_type_def::attr_type_def(attr_type_def && rhs) noexcept
      : type(rhs.type),
        enum_def(nullptr)
    {
      if (rhs.type == attr_enum) {
        enum_def = rhs.enum_def;
      } else if (rhs.type == attr_tuple) {
        ::new (&tuple) vector<attr_type_def>(static_cast<vector<attr_type_def> &&>(rhs.tuple));
      }
      rhs.type = attr_type_none;
    }

  attr_type_def & attr_type_def::operator=(attr_type_def && rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      clear();
      type = rhs.type;
      if (rhs.type == attr_enum) {
        enum_def = rhs.enum_def;
      } else if (rhs.type == attr_tuple) {
        ::new (&tuple) vector<attr_type_def>(static_cast<vector<attr_type_def> &&>(rhs.tuple));
      }
      rhs.type = attr_type_none;
    }
    return *this;
  }


  attr_type_def::attr_type_def(const attr_type_def & rhs) noexcept
    : type(rhs.type)
  {
    if (rhs.type == attr_enum) {
      enum_def = rhs.enum_def;
    } else if (rhs.type == attr_tuple) {
      ::new (&tuple) vector<attr_type_def>(rhs.tuple);
    }
  }

  attr_type_def & attr_type_def::operator=(const attr_type_def & rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      clear();
      type = rhs.type;
      if (rhs.type == attr_enum) {
        enum_def = rhs.enum_def;
      } else if (rhs.type == attr_tuple) {
        ::new (&tuple) vector<attr_type_def>(rhs.tuple);
      }
    }
    return *this;
  }

  void attr_type_def::init_tuple() noexcept {
    ::new (&tuple) noncopyable_vector<attr_type_def>();
  }

  void attr_type_def::clear() noexcept {
    if (type == attr_tuple) {
      tuple.~vector();
    }
    type = attr_type_none;
  }

  [[nodiscard]] bool attr_type_def::operator==(const attr_type_def & rhs) const noexcept {
    if (type != rhs.type) {
      return false;
    }
    if (type == attr_enum) {
      return enum_def == rhs.enum_def;
    }
    if (type == attr_tuple) {
      if (tuple.size() != rhs.tuple.size()) {
        return false;
      }
      const attr_type_def * t2 = rhs.tuple.begin();
      for (auto & t : tuple) {
        if (t != *t2) {
          return false;
        }
        ++t2;
      }
    }
    return true;
  }

  attr_type_def::~attr_type_def() {
    clear();
  }

  attr_def::attr_def(u8_t index, enum node_type subjects, s::const_string_view name, doc_comment && comment)
    : node(node_attr_def, name, static_cast<doc_comment &&>(comment)),
      subjects(subjects),
      index(index)
  {
  }

  attr_context::attr_context() noexcept
    : attr_defs(16)
  {
    define_attr("AfterConstruct", node_term);
  }

  attr_context::~attr_context() {
  }

  u8_t attr_context::at(strings::const_string_view key, node_type subject) const {
    J_ASSERT(key && subject != node_none);
    auto idx = attr_defs.at(key);
    validate_entry(idx, subject);
    return idx;
  }

  u64_t attr_context::to_bitmask(strings::const_string_view key) const {
    J_ASSERT(key);
    return 1UL << attr_defs.at(key);
  }
  u64_t attr_context::to_bitmask(const char * const * keys, u32_t size) const {
    u64_t bm = 0U;
    while (size--) {
      bm |= 1UL << attr_defs.at(*keys++);
    }
    return bm;
  }
  u64_t attr_context::to_bitmask(const strings::const_string_view * keys, u32_t size) const {
    u64_t bm = 0U;
    while (size--) {
      bm |= 1UL << attr_defs.at(*keys++);
    }
    return bm;
  }

  u8_t attr_context::define_attr(strings::const_string_view key, node_type subject) {
    return define_attr(key, subject, {});
  }

  u8_t attr_context::define_attr(strings::const_string_view key, node_type subject, doc_comment && comment) {
    J_ASSERT(key && subject != node_none);
    defs[cur_index] = attr_def(cur_index, subject, key, static_cast<doc_comment &&>(comment));
    const auto p = attr_defs.emplace(key, cur_index++);
    J_ASSERT(p.second, "Duplicate attribute definition \"{}\".", key);
    return p.first->second;
  }

  void attr_context::validate_entry(u8_t idx, node_type subject) const {
    J_ASSERT(idx < 32U);
    auto & def = defs[idx];
    if (!(def.subjects & subject)) {
      J_THROW("Attribute {} is only supported on {}.",
              def.name, def.subjects == node_term ? "terms" : "operands");
    }
  }

  void attr_context::validate_entry(strings::const_string_view key,
                                    node_type subject) const
  {
    J_ASSERT(key && subject != node_none);
    auto idx = attr_defs.at(key);
    validate_entry(idx, subject);
  }
}
