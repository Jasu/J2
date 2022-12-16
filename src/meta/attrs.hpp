#pragma once

#include "strings/string_map_fwd.hpp"
#include "meta/attr_value.hpp"
#include "meta/common.hpp"

J_DECLARE_EXTERN_STRING_MAP(j::meta::attr_value);

namespace j::meta {
  struct attr_context;

  struct attrs final {
    J_A(AI,ND) inline attrs() noexcept = default;

    attrs(attrs && rhs) noexcept;
    attrs & operator=(attrs && rhs) noexcept;
    attrs(const attrs & rhs) noexcept;
    attrs & operator=(const attrs & rhs) noexcept;
    ~attrs();

    void clear() noexcept;

    attr_value & set(const attr_context * J_NOT_NULL ctx,
             strings::const_string_view key,
             attr_value && value,
             node_type applied_to = node_any);

    void apply_default(attr_context * J_NOT_NULL ctx,
                       strings::const_string_view key,
                       const attr_value & value,
                       node_type applied_to = node_any);

    attr_value & set(u8_t index, strings::const_string_view key, attr_value && value) noexcept;

    [[nodiscard]] bool has(strings::const_string_view key) const noexcept;
    [[nodiscard]] const attr_value & at(const attr_context & ctx, strings::const_string_view key) const noexcept;

    [[nodiscard]] attr_value * maybe_at(strings::const_string_view key) noexcept;
    [[nodiscard]] const attr_value * maybe_at(strings::const_string_view key) const noexcept;
    [[nodiscard]] strings::string * maybe_string_at(strings::const_string_view key) noexcept;
    [[nodiscard]] const strings::string * maybe_string_at(strings::const_string_view key) const noexcept;

    J_A(AI,NODISC) inline u32_t size() const noexcept { return data.size(); }
    J_A(AI,NODISC) inline bool empty() const noexcept { return data.empty(); }
    J_A(AI,NODISC) inline explicit operator bool() const noexcept { return !data.empty(); }
    J_A(AI,NODISC) inline bool operator!() const noexcept { return data.empty(); }

    [[nodiscard]] strings::string_map_iterator<attr_value> begin() noexcept { return data.begin(); }
    [[nodiscard]] strings::const_string_map_iterator<attr_value> begin() const noexcept { return data.begin(); }
    [[nodiscard]] strings::string_map_iterator<attr_value> end() noexcept { return data.end(); }
    [[nodiscard]] strings::const_string_map_iterator<attr_value> end() const noexcept { return data.end(); }

    strings::string_map<attr_value> data;
    u64_t bitmask = 0U;
  };
}
