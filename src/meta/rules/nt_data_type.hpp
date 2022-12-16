#pragma once
#include "meta/value_types.hpp"
#include "meta/rules/common.hpp"

namespace j::meta::inline rules {
  struct term_expr;

  struct nt_data_field final {
    J_BOILERPLATE(nt_data_field, CTOR_NE_ND)

    strings::string name{};
    val_type * type = nullptr;
    i32_t offset = 0U;
    bool has_lifetime_tracking = false;
    i8_t tracking_index = -1;
    i8_t track_index = -1;
    term_expr * initializer = nullptr;
    term_expr * release_extra_arg = nullptr;

    nt_data_field(val_type * J_NOT_NULL type, strings::const_string_view name, term_expr * init, term_expr * release_extra_arg, i32_t tracking_index, i32_t track_index) noexcept;

    void combine(const nt_data_field & rhs, i32_t index) noexcept;
    void set_default_name(i32_t index) noexcept;
  };

  struct nt_data_type final {
    J_BOILERPLATE(nt_data_type, CTOR_NE_ND)
    strings::string name{};
    vector<nt_data_field> fields{};
    u32_t index = 0U;
    u32_t size = 0U;
    i8_t num_tracked_fields = 0;
    i8_t max_tracking_index = -1;

    explicit nt_data_type(strings::const_string_view name) noexcept;

    void add_field(val_type * J_NOT_NULL type, strings::const_string_view name, term_expr * init, term_expr * release_extra_arg, i32_t tracking_index = -1) noexcept;

    J_A(AI,NODISC) inline u32_t num_fields() const noexcept { return fields.size(); }
    [[nodiscard]] u32_t field_index(strings::const_string_view name) const;
    [[nodiscard]] const nt_data_field & field_at(strings::const_string_view name) const;
    [[nodiscard]] const nt_data_field * field_maybe_at(strings::const_string_view name) const noexcept;
    [[nodiscard]] strings::const_string_view field_name(u32_t index) const;

    J_A(AI,NODISC) inline bool operator!() const noexcept {
      return !(bool)fields;
    }
    J_A(AI,NODISC) inline explicit operator bool() const noexcept {
      return (bool)fields;
    }
    [[nodiscard]] bool layout_eq(const nt_data_type & rhs) const noexcept;

    void combine(const nt_data_type & rhs) noexcept;

  };

  struct nt_data_type_hash final  {
    [[nodiscard]] bool operator()(const nt_data_type_p & lhs, const nt_data_type & rhs) const noexcept;
    [[nodiscard]] bool operator()(const nt_data_type_p & lhs, const nt_data_type_p & rhs) const noexcept;
    [[nodiscard]] u32_t operator()(const nt_data_type & t) const noexcept;
    [[nodiscard]] u32_t operator()(const nt_data_type_p & s) const noexcept;
  };

  struct nt_data_type_const_key final {
    using type = nt_data_type;
    using arg_type = const nt_data_type &;
    using getter_type = nt_data_type;

    J_A(AI,NODISC) inline const nt_data_type & operator()(const nt_data_type_p & p) const noexcept {
      return *p;
    }
  };

  struct nt_data_types final {
    nt_data_types() noexcept;

    vector<nt_data_type_p> types;
    hash_set<nt_data_type_p, nt_data_type_hash, nt_data_type_hash, nt_data_type_const_key> hash;

    [[nodiscard]] nt_data_type_p type_maybe_at(strings::const_string_view name) noexcept;

    void set_data_type(nt_p J_NOT_NULL nt, const nt_data_type & type);
    [[nodiscard]] nt_data_type_p add(const nt_data_type & type) noexcept;

    [[nodiscard]] inline u32_t size() const noexcept { return types.size(); }
    [[nodiscard]] inline const nt_data_type_p & operator[](u32_t i) const noexcept { return types[i]; }
    [[nodiscard]] inline const nt_data_type_p * begin() const noexcept { return types.begin(); }
    [[nodiscard]] inline const nt_data_type_p * end() const noexcept { return types.end(); }
  };
}

J_DECLARE_EXTERN_HASH_SET(j::meta::nt_data_type_p, HASH(j::meta::nt_data_type_hash), KEYEQ(j::meta::nt_data_type_hash), CONST_KEY(j::meta::nt_data_type_const_key));
