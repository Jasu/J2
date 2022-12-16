#include "attrs.hpp"
#include "strings/string_map.hpp"
#include "exceptions/assert.hpp"
#include "meta/attr_context.hpp"

J_DEFINE_EXTERN_STRING_MAP(j::meta::attr_value);

namespace j::meta {
  attrs::attrs(attrs && rhs) noexcept
    : data(static_cast<strings::string_map<attr_value> &&>(rhs.data)),
      bitmask(rhs.bitmask)
  {
    rhs.bitmask = 0U;
  }

  attrs & attrs::operator=(attrs && rhs) noexcept {
    if (this != &rhs) {
      data = static_cast<strings::string_map<attr_value> &&>(rhs.data);
      bitmask = rhs.bitmask;
      rhs.bitmask = 0U;
    }
    return *this;
  }

  attrs::attrs(const attrs & rhs) noexcept
    : data(rhs.data),
      bitmask(rhs.bitmask)
  {
  }

  attrs & attrs::operator=(const attrs & rhs) noexcept {
    if (this != &rhs) {
      data = rhs.data;
      bitmask = rhs.bitmask;
    }
    return *this;
  }

  attrs::~attrs() {
  }

  void attrs::clear() noexcept {
    data.clear();
    bitmask = 0U;
  }

  attr_value & attrs::set(const attr_context * J_NOT_NULL ctx,
                          strings::const_string_view key,
                          attr_value && value,
                          node_type applied_to) {
    u8_t idx = ctx->at(key, applied_to);
    bitmask |= 1 << idx;
    if (value) {
      return data[key] = static_cast<attr_value &&>(value);
    } else {
      J_REQUIRE(ctx->defs[idx].default_set_value, "Attribute {} has no default value.", key);
      return data[key] = ctx->defs[idx].default_set_value;
    }
  }

  void attrs::apply_default(attr_context * J_NOT_NULL ctx,
                     strings::const_string_view key,
                     const attr_value & value,
                     node_type applied_to) {
    u8_t idx = ctx->at(key, applied_to);
    if (bitmask & (1 << idx)) {
      return;
    }
    bitmask |= (1 << idx);
    J_REQUIRE(data.emplace(key, value).second, "Duplicate attribute when applying defaults: {}", key);
  }

  attr_value & attrs::set(u8_t index, strings::const_string_view key, attr_value && value) noexcept {
    bitmask |= 1 << index;
    return data[key] = static_cast<attr_value &&>(value);
  }

  [[nodiscard]] bool attrs::has(strings::const_string_view key) const noexcept {
    return data.contains(key);
  }

  [[nodiscard]] const attr_value & attrs::at(const attr_context & ctx, strings::const_string_view key) const noexcept {
    if (const attr_value * res = data.maybe_at(key)) {
      return *res;
    }
    auto idx = ctx.at(key);
    if (ctx.defs[idx].default_get_value) {
      return ctx.defs[idx].default_get_value;
    }
    J_THROW("Attribute {} not found.", key);
  }

  [[nodiscard]] attr_value * attrs::maybe_at(strings::const_string_view key) noexcept
  { return data.maybe_at(key); }

  [[nodiscard]] const attr_value * attrs::maybe_at(strings::const_string_view key) const noexcept
  { return data.maybe_at(key); }

  [[nodiscard]] strings::string * attrs::maybe_string_at(strings::const_string_view key) noexcept {
    auto p = maybe_at(key);
    return p && p->is_str() ? &p->string : nullptr;
  }
  [[nodiscard]] const strings::string * attrs::maybe_string_at(strings::const_string_view key) const noexcept {
    auto p = maybe_at(key);
    return p && p->is_str() ? &p->string : nullptr;
  }
}
