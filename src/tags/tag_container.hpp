#pragma once


#include "tags/tag.hpp"
#include "containers/vector.hpp"
#include "hzd/type_traits.hpp"

J_DECLARE_EXTERN_NONCOPYABLE_VECTOR(j::tags::tag);

namespace j::tags {
  class tag_container {
  public:
    tag_container() noexcept;
    tag_container(tag_container && rhs) noexcept;
    tag_container & operator=(tag_container && rhs) noexcept;

    /// Used to identify classes deriving from tag_container without calls to
    /// is_base_of etc.
    using tag_container_tag_t = void;

    [[nodiscard]] tag * maybe_get_tag(const tag_definition_base & definition) noexcept;

    [[nodiscard]] const tag * maybe_get_tag(const tag_definition_base & definition) const noexcept {
      return const_cast<tag_container*>(this)->maybe_get_tag(definition);
    }

    template<typename ValueType>
    [[nodiscard]] const ValueType * maybe_get_tag_value(const tag_definition<ValueType> & definition) const noexcept {
      const tag * t = maybe_get_tag(definition);
      return t ? t->value().maybe_get<ValueType>() : nullptr;
    }

    J_A(RNN) tag * add_tag(tag && t) noexcept;
    J_A(RNN) tag * add_tag_before(tag * at, tag && t) noexcept;

    [[nodiscard]] i32_t size() noexcept;
    [[nodiscard]] tag * begin() noexcept;

    [[nodiscard]] tag * end() noexcept;

    [[nodiscard]] const tag * begin() const noexcept;

    [[nodiscard]] const tag * end() const noexcept;

  private:
    noncopyable_vector<tag> m_tags;
  };

  template<typename Container, typename = typename j::remove_ref_t<Container>::tag_container_tag_t>
  Container && operator<<(Container && c, tag && t) noexcept {
    c.add_tag(static_cast<tag &&>(t));
    return static_cast<Container &&>(c);
  }
}
