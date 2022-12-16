#pragma once

#include "meta/node.hpp"
#include "containers/vector.hpp"

J_DECLARE_EXTERN_STRING_MAP(u8_t);

namespace j::meta {
  struct enum_def;
  struct enum_entry final {
    strings::string name;
    u64_t value;
    enum_def * def;
  };

  struct enum_def final : node {
    noncopyable_vector<enum_entry> entries;
    bool is_flag_enum = false;
    enum_def(bool is_flag_enum, strings::const_string_view name, doc_comment && comment = {});
    enum_def(bool is_flag_enum, strings::const_string_view name, const strings::const_string_view * J_NOT_NULL first, i32_t size) noexcept;

    [[nodiscard]] u64_t at(strings::const_string_view name) const;
    [[nodiscard]] bool has(strings::const_string_view name) const noexcept;
    [[nodiscard]] attr_value value(strings::const_string_view name) const noexcept;
    [[nodiscard]] strings::string maybe_get_enumerant_name(u64_t value, bool qualified = false) const noexcept;
    [[nodiscard]] strings::string get_name(u64_t value, bool qualified = false) const;
    [[nodiscard]] strings::const_string_view get_unqualified_name(u64_t value) const;
    enum_def & add(strings::const_string_view name);
    enum_def & add(strings::const_string_view name, u64_t value);
    [[nodiscard]] u32_t size() const noexcept;
  };

  template<i32_t I>
  J_A(AI,ND,NODISC,RNN) inline enum_def * make_enum(strings::const_string_view name, strings::const_string_view (&& enumerants)[I]) noexcept {
    return ::new enum_def(false, name, enumerants, I);
  }

  template<i32_t I>
  J_A(AI,ND,NODISC,RNN) inline enum_def * make_flags(strings::const_string_view name, strings::const_string_view (&& enumerants)[I]) noexcept {
    return ::new enum_def(true, name, enumerants, I);
  }

  struct struct_def final : node {
    struct_def(strings::const_string_view name, doc_comment && comment);
    template<u32_t I>
    J_A(ND) struct_def(const char * name, const char * const (&& fields)[I])
      : node(node_struct_def, strings::const_string_view(name)),
        fields(I)
    {
      for (u32_t i = 0U; i < I; ++i) {
        this->fields.push_back(fields[i]);
      }
    }

    noncopyable_vector<strings::string> fields;
    [[nodiscard]] u32_t index_of(strings::const_string_view name) const;
    [[nodiscard]] bool has(strings::const_string_view name) const noexcept;
    [[nodiscard]] u32_t size() const noexcept;
    void push_back(strings::const_string_view name);
  };

  struct attr_type_def final {
    attr_type type = attr_type_none;
    J_A(AI) inline constexpr attr_type_def() noexcept: enum_def(nullptr) { }

    J_A(AI) inline explicit attr_type_def(attr_type type) noexcept
      : type(type),
        enum_def(nullptr)
    {
      if (type == attr_tuple) {
        init_tuple();
      }
    }

    J_A(AI) inline constexpr explicit attr_type_def(const struct enum_def * J_NOT_NULL def) noexcept
      : type(attr_enum),
        enum_def(def)
    { }

    J_A(AI) inline bool operator!() const noexcept {
      return type == attr_type_none;
    }

    J_A(AI) inline explicit operator bool() const noexcept {
      return type != attr_type_none;
    }

    attr_type_def(attr_type_def && rhs) noexcept;

    attr_type_def & operator=(attr_type_def && rhs) noexcept;

    [[nodiscard]] bool operator==(const attr_type_def & rhs) const noexcept;

    attr_type_def(const attr_type_def & rhs) noexcept;

    attr_type_def & operator=(const attr_type_def & rhs) noexcept;

    ~attr_type_def();

    [[nodiscard]] inline bool matches(const attr_value & val) const noexcept {
      return val.type == type && (type != attr_enum || val.enum_val.def == enum_def);
    }

    [[nodiscard]] static inline attr_type_def type_of(const attr_value & val) noexcept {
      if (val.type == attr_enum) {
        return attr_type_def(val.enum_val.def);
      }
      return attr_type_def(val.type);
    }

    void init_tuple() noexcept;

    void clear() noexcept;

    union {
      const enum_def * enum_def;
      vector<attr_type_def> tuple;
    };
  };

  template<typename T>
  concept AttrKey = is_same_v<T, strings::const_string_view> || is_same_v<T, const char *>;

  struct attr_def final : node {
    J_BOILERPLATE(attr_def, MOVE_NE_ND)

    J_A(AI,ND) inline attr_def() noexcept : node(node_attr_def) { }

    attr_def(u8_t index,
             node_type subjects,
             strings::const_string_view name,
             doc_comment && comment);

    node_type subjects = node_none;
    u8_t index = 0U;
    attr_value default_get_value{};
    attr_value default_set_value{};
    attr_type_def type{};
  };

  struct attr_context final {
    J_BOILERPLATE(attr_context, COPY_DEL)
    attr_context() noexcept;
    ~attr_context();

    u8_t define_attr(strings::const_string_view key, node_type subject);
    u8_t define_attr(strings::const_string_view key, node_type subject, doc_comment && comment);

    u8_t at(strings::const_string_view key, node_type subject = node_any) const;

    u64_t to_bitmask(strings::const_string_view key) const;
    u64_t to_bitmask(const char * const * keys, u32_t size) const;
    u64_t to_bitmask(const strings::const_string_view * keys, u32_t size) const;

    void validate_entry(strings::const_string_view key, node_type subject) const;

    void validate_entry(u8_t idx, node_type subject) const;

    strings::string_map<u8_t> attr_defs;
    u8_t cur_index = 0;
    attr_def defs[32];
  };
}
