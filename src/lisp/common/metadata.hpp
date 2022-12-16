#pragma once

#include "containers/span.hpp"
#include "strings/string_view.hpp"
#include "lisp/sources/source_location.hpp"
#include "lisp/common/operand_name.hpp"
#include "lisp/common/id.hpp"

namespace j::lisp::inline common {
  enum class metadata_type : u8_t {
    none,

    /// Source location that generated the instruction.
    source_location,
    /// Information about the instruction operand.
    operand,
    /// Information about the instruction result.
    result,
    /// Text comment relating to the instruction.
    comment,
  };

  J_INLINE_GETTER constexpr bool has_text(metadata_type type) noexcept {
    return type >= metadata_type::operand;
  }

  J_INLINE_GETTER constexpr bool is_value_metadata(metadata_type type) noexcept {
    return type == metadata_type::result || type == metadata_type::operand;
  }

  struct metadata_key_none;
  struct metadata_key_result;
  struct metadata_key_comment;
  struct metadata_key_loc;
  struct metadata_key_op;

  struct metadata_key {
    J_BOILERPLATE(metadata_key, CTOR_CE, EQ_CE)

    metadata_type type = metadata_type::none;
    u8_t operand_index = 0U;

    J_ALWAYS_INLINE constexpr metadata_key(metadata_type type, u8_t index = 0U) noexcept
      : type(type),
        operand_index(index)
    { }

    J_A(AI,ND,NODISC) inline constexpr explicit operator bool() const noexcept {
      return type != metadata_type::none;
    }

    J_A(AI,ND,NODISC) inline constexpr bool operator!() const noexcept {
      return type == metadata_type::none;
    }

    J_INLINE_GETTER static constexpr metadata_key_op op(u8_t index) noexcept;

    static const metadata_key_none none;
    static const metadata_key_result result;
    static const metadata_key_comment comment;
    static const metadata_key_loc loc;
  };

  struct metadata_key_none final : metadata_key {
    J_A(AI,ND) inline constexpr metadata_key_none() noexcept : metadata_key{metadata_type::none} { }
    using metadata_key::operator==;
  };

  struct metadata_key_result final : metadata_key {
    J_A(AI,ND) inline constexpr metadata_key_result() noexcept : metadata_key{metadata_type::result} { }
    using metadata_key::operator==;
  };

  struct metadata_key_comment final : metadata_key {
    J_A(AI,ND) inline constexpr metadata_key_comment() noexcept : metadata_key{metadata_type::comment} { }
    using metadata_key::operator==;
  };

  struct metadata_key_loc final : metadata_key {
    J_A(AI,ND) inline constexpr metadata_key_loc() noexcept : metadata_key{metadata_type::source_location} { }
    using metadata_key::operator==;
  };

  struct metadata_key_op final : metadata_key {
    J_A(AI,ND) inline constexpr metadata_key_op(u8_t index) noexcept : metadata_key{metadata_type::operand, index} { }
    using metadata_key::operator==;
  };

  J_A(ND) inline constexpr metadata_key_result  metadata_key::result;
  J_A(ND) inline constexpr metadata_key_comment metadata_key::comment;
  J_A(ND) inline constexpr metadata_key_loc     metadata_key::loc;
  J_A(ND) inline constexpr metadata_key_none    metadata_key::none;

  template<typename T>
  concept MetadataKey = DerivedFrom<metadata_key, remove_cref_t<T>>;

  J_INLINE_GETTER constexpr metadata_key_op metadata_key::op(u8_t index) noexcept {
    return {index};
  }

  /// Hint for formatting known values of an operand or a result.
  enum class format_hint : u8_t {
    none,
    signed_dec,
    hex,
    bin,
  };

  struct value_metadata final {
    struct id id = lisp::common::id{};
    i8_t index = 0;
    operand_name_format name_format = operand_name_format::default_format;
    format_hint format = format_hint::none;

    [[nodiscard]] strings::const_string_view name() const noexcept;
    [[nodiscard]] operand_name operand_name() const noexcept;
  };

  struct comment_metadata final {
    [[nodiscard]] strings::const_string_view comment() const noexcept;
  };

  struct metadata final {
    metadata_key key;
    u16_t text_size:15 = 0U;
    bool is_last:1 = false;
    union {
      sources::source_location source_location;
      value_metadata value_metadata;
      comment_metadata comment_metadata;
    };
    const metadata * next() const noexcept;

    [[nodiscard]] strings::const_string_view text() const noexcept;

    J_INLINE_GETTER u32_t size() const noexcept {
      return align_up(sizeof(metadata) + text_size, 4U);
    }
  };

  struct metadata_iterator final {
    const metadata * it = nullptr;
    J_INLINE_GETTER bool operator==(const metadata_iterator &) const noexcept = default;

    J_INLINE_GETTER_NONNULL const metadata * operator->() const noexcept {
      return it;
    }

    J_INLINE_GETTER const metadata & operator*() const noexcept {
      return *it;
    }

    metadata_iterator & operator++() noexcept {
      it = it->is_last ? nullptr : it->next();
      return * this;
    }

    metadata_iterator operator++(int) noexcept {
      metadata_iterator result{it};
      operator++();
      return result;
    }
  };

  struct metadata_view final {
    J_INLINE_GETTER metadata_iterator begin() const noexcept {
      return {first};
    }

    J_INLINE_GETTER metadata_iterator end() const noexcept {
      return {nullptr};
    }

    [[nodiscard]] sources::source_location loc() const noexcept;
    [[nodiscard]] strings::const_string_view comment() const noexcept;

    [[nodiscard]] const metadata * find(metadata_key key) const noexcept;

    [[nodiscard]] const value_metadata * result() const noexcept;
    [[nodiscard]] id result_id() const noexcept;
    [[nodiscard]] strings::const_string_view result_name() const noexcept;

    [[nodiscard]] const value_metadata * operand(u8_t index) const noexcept;
    [[nodiscard]] id operand_id(u8_t index) const noexcept;
    [[nodiscard]] strings::const_string_view operand_name(u8_t index) const noexcept;

    const metadata * first = nullptr;
  };
}
