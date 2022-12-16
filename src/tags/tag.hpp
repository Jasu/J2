#pragma once

#include "mem/any.hpp"
#include "strings/string_view.hpp"

namespace j::tags {
  enum class tag_name_format : u8_t {
    unformatted,
    formatted,
    with_index,
  };
  struct tag_definition_base {
    const char * name;
    const char * format_string;
    tag_name_format name_format;
  };

  struct tag final {
    explicit tag(const tag_definition_base * definition, mem::any && value) noexcept;

    tag(const tag &) = delete;
    tag & operator=(const tag &) = delete;
    tag(tag &&) noexcept;
    tag & operator=(tag &&) noexcept;

    ~tag();

    J_A(AI,NODISC) inline const tag_definition_base & definition() const noexcept {
      return *m_definition;
    }

    J_A(AI,NODISC) inline bool is(const tag_definition_base & def) const noexcept {
      return m_definition == &def;
    }

    J_A(AI,NODISC) inline const mem::any & value() const noexcept { return m_value;  }
    J_A(AI,NODISC) inline mem::any & value() noexcept { return m_value; }
  private:
    const tag_definition_base * m_definition;
    mem::any m_value;
  };

  template<typename ValueType>
  class tag_definition final : public tag_definition_base {
  public:
    J_A(AI) explicit inline tag_definition(const char * J_NOT_NULL name, const char * J_NOT_NULL fmt = "{}", tag_name_format name_format = tag_name_format::unformatted) noexcept
      : tag_definition_base{name, fmt, name_format}
    { }

    tag operator()(const ValueType & value) const;
    tag operator()(ValueType && value) const;
  };

  template<typename ValueType>
  tag tag_definition<ValueType>::operator()(const ValueType & value) const {
    return tag(this, mem::any(value));
  }

  template<typename ValueType>
  tag tag_definition<ValueType>::operator()(ValueType && value) const {
    return tag(this, mem::any(static_cast<ValueType &&>(value)));
  }

  template<Scalar ValueType>
  class tag_definition<ValueType> final : public tag_definition_base {
  public:
    consteval explicit tag_definition(const char * J_NOT_NULL name, const char * J_NOT_NULL fmt = "{}", tag_name_format name_format = tag_name_format::unformatted)
      : tag_definition_base{name, fmt, name_format}
    { }

    tag operator()(ValueType value) const {
      return tag(this, mem::any(value));
    }
  };

  template<>
  class tag_definition<strings::string> final : public tag_definition_base {
  public:
    consteval explicit inline tag_definition(const char * J_NOT_NULL name, const char * J_NOT_NULL fmt = "{}", tag_name_format name_format = tag_name_format::unformatted)
      : tag_definition_base{name, fmt, name_format}
    { }

    tag operator()(const strings::string & value) const;
    tag operator()(strings::string && value) const;
    tag operator()(const char * value) const;
    tag operator()(const strings::const_string_view & value) const;
  };

  template<>
  class tag_definition<mem::any> final : public tag_definition_base {
  public:
    consteval tag_definition(const char * J_NOT_NULL name, const char * J_NOT_NULL fmt = "{}", tag_name_format name_format = tag_name_format::unformatted) noexcept
      : tag_definition_base{name, fmt, name_format}
    { }

    template<typename T>
    J_A(AI) inline tag operator()(T && value) const {
      if constexpr (is_same_v<T, mem::any>) {
        return tag(this, static_cast<mem::any &&>(value));
      } else {
        return tag(this, mem::any(static_cast<T &&>(value)));
      }
    }
  };
}

extern template class j::tags::tag_definition<j::strings::string>;
