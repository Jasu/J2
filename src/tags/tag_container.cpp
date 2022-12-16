#include "tags/tag_container.hpp"

J_DEFINE_EXTERN_NONCOPYABLE_VECTOR(j::tags::tag);

namespace j::tags {
  tag_container::tag_container() noexcept { }
  tag_container::tag_container(tag_container && rhs) noexcept
    : m_tags(static_cast<noncopyable_vector<tag> &&>(rhs.m_tags))
  { }

  tag_container & tag_container::operator=(tag_container && rhs) noexcept {
    m_tags = static_cast<noncopyable_vector<tag> &&>(rhs.m_tags);
    return *this;
  }

  [[nodiscard]] i32_t tag_container::size() noexcept {
    return m_tags.size();
  }

  [[nodiscard]] tag * tag_container::begin() noexcept {
    return m_tags.begin();
  }

  [[nodiscard]] tag * tag_container::end() noexcept {
    return m_tags.end();
  }

  [[nodiscard]] const tag * tag_container::begin() const noexcept {
    return m_tags.begin();
  }

  [[nodiscard]] const tag * tag_container::end() const noexcept {
    return m_tags.end();
  }

  J_A(RNN) tag * tag_container::add_tag(tag && t) noexcept {
    return &m_tags.emplace_back(static_cast<tag &&>(t));
  }

  J_A(RNN) tag * tag_container::add_tag_before(tag * at, tag && t) noexcept {
    return m_tags.emplace(at, static_cast<tag &&>(t));
  }

  tag * tag_container::maybe_get_tag(const tag_definition_base & definition) noexcept {
    for (auto & tag : m_tags) {
      if (tag.is(definition)) {
        return &tag;
      }
    }
    return nullptr;
  }
}
