#pragma once

#include "properties/property_type.hpp"
#include "strings/string.hpp"
#include "containers/vector.hpp"

namespace j::properties {
  enum class COMPONENT_TYPE : u8_t {
    EMPTY         = 0x00U,
    ARRAY_INDEX   = 0x01U,
    MAP_KEY       = 0x02U,
    PROPERTY_NAME = 0x03U,
  };

  namespace detail {
    enum class array_index_tag_t { v };
    enum class property_name_tag_t { v };
    enum class map_key_tag_t { v };
  }

  J_NO_DEBUG inline constexpr detail::array_index_tag_t array_index_tag{detail::array_index_tag_t::v};
  J_NO_DEBUG inline constexpr detail::property_name_tag_t property_name_tag{detail::property_name_tag_t::v};
  J_NO_DEBUG inline constexpr detail::map_key_tag_t map_key_tag{detail::map_key_tag_t::v};

  class path;

  class path_component {
    friend i64_t path_cmp(const path &, const path &) noexcept;
    uptr_t m_data;
    COMPONENT_TYPE m_type = COMPONENT_TYPE::EMPTY;

    void initialize_string(const strings::string & str);
    void initialize_string(strings::string && str) noexcept;

    [[noreturn]] void throw_not_map_key() const;
    [[noreturn]] void throw_not_property_name() const;
    [[noreturn]] void throw_not_array_index() const;

    J_INLINE_GETTER bool is_string() const noexcept {
      return static_cast<u8_t>(m_type) & 0x02U;
    }
  public:
    J_INLINE_GETTER strings::string & string_ref() const noexcept {
      return const_cast<strings::string &>(reinterpret_cast<const strings::string &>(m_data));
    }

    using nicely_copyable_tag_t = void;
    using zero_initializable_tag_t = void;
    path_component() noexcept = default;

    explicit path_component(const detail::property_name_tag_t &, const strings::string & rhs);

    explicit path_component(const detail::property_name_tag_t &, strings::string && rhs) noexcept;

    explicit path_component(const detail::map_key_tag_t &, const strings::string & rhs);

    explicit path_component(const detail::map_key_tag_t &, strings::string && rhs) noexcept;

    explicit path_component(const detail::array_index_tag_t &, sz_t sz) noexcept
      : m_data(sz),
        m_type(COMPONENT_TYPE::ARRAY_INDEX)
    {
    }

    path_component(const path_component & rhs);

    path_component(path_component && rhs) noexcept
      : m_data(rhs.m_data),
        m_type(rhs.m_type)
    {
      rhs.m_type = COMPONENT_TYPE::EMPTY;
    }

    path_component & operator=(const path_component & rhs);

    path_component & operator=(path_component && rhs) noexcept {
      if (this != &rhs) {
        reset();
        m_data = rhs.m_data;
        m_type = rhs.m_type;
        rhs.m_type = COMPONENT_TYPE::EMPTY;
      }
      return *this;
    }

    void reset() noexcept {
      if (is_string()) {
        string_ref().~string();
      }
      m_type = COMPONENT_TYPE::EMPTY;
    }

    ~path_component() {
      reset();
    }

    template<typename Name>
    void set_to_property_name(Name && name) {
      if (is_string()) {
        string_ref() = static_cast<Name &&>(name);
      } else {
        initialize_string(static_cast<Name &&>(name));
      }
      m_type = COMPONENT_TYPE::PROPERTY_NAME;
    }

    template<typename Name>
    void set_to_map_key(Name && name) {
      if (is_string()) {
        string_ref() = static_cast<Name &&>(name);
      } else {
        initialize_string(static_cast<Name &&>(name));
      }
      m_type = COMPONENT_TYPE::MAP_KEY;
    }

    void set_to_array_index(sz_t index) noexcept {
      reset();
      m_data = index;
      m_type = COMPONENT_TYPE::ARRAY_INDEX;
    }

    J_INLINE_GETTER bool empty() const noexcept {
      return m_type == COMPONENT_TYPE::EMPTY;
    }

    J_INLINE_GETTER COMPONENT_TYPE type() const noexcept {
      return m_type;
    }

    J_INLINE_GETTER bool is_root() const noexcept {
      return m_type == COMPONENT_TYPE::EMPTY;
    }

    J_INLINE_GETTER bool is_map_key() const noexcept {
      return m_type == COMPONENT_TYPE::MAP_KEY;
    }

    J_INLINE_GETTER bool is_property_name() const noexcept {
      return m_type == COMPONENT_TYPE::PROPERTY_NAME;
    }

    J_INLINE_GETTER bool is_array_index() const noexcept {
      return m_type == COMPONENT_TYPE::ARRAY_INDEX;
    }

    J_INLINE_GETTER const strings::string & as_property_name_noexcept() const noexcept {
      return string_ref();
    }

    const strings::string & as_property_name() const {
      if (J_UNLIKELY(m_type != COMPONENT_TYPE::PROPERTY_NAME)) {
        throw_not_property_name();
      }
      return string_ref();
    }

    J_INLINE_GETTER const strings::string & as_map_key_noexcept() const noexcept {
      return string_ref();
    }

    const strings::string & as_map_key() const {
      if (J_UNLIKELY(m_type != COMPONENT_TYPE::MAP_KEY)) {
        throw_not_map_key();
      }
      return string_ref();
    }

    J_INLINE_GETTER sz_t as_array_index_noexcept() const noexcept {
      return m_data;
    }

    sz_t as_array_index() const {
      if (J_UNLIKELY(m_type != COMPONENT_TYPE::ARRAY_INDEX)) {
        throw_not_array_index();
      }
      return m_data;
    }

    bool operator==(const path_component & rhs) const noexcept {
      if (m_type != rhs.m_type) {
        return false;
      }
      switch (m_type) {
      case COMPONENT_TYPE::MAP_KEY:
      case COMPONENT_TYPE::PROPERTY_NAME:
        return string_ref() == rhs.string_ref();
      case COMPONENT_TYPE::ARRAY_INDEX:
        return m_data == rhs.m_data;
      case COMPONENT_TYPE::EMPTY:
        return true;
      }
    }

    bool operator<(const path_component & rhs) const noexcept;

    bool operator>=(const path_component & rhs) const noexcept;

    bool operator<=(const path_component & rhs) const noexcept;

    bool operator>(const path_component & rhs) const noexcept;
  };
}

J_DECLARE_EXTERN_VECTOR(j::properties::path_component);

namespace j::properties {
  class path {
    vector<path_component> m_data;
  public:
    J_INLINE_GETTER bool empty() const noexcept {
      return m_data.empty();
    }

    J_INLINE_GETTER u32_t size() const noexcept {
      return m_data.size();
    }

    void clear() noexcept;

    bool operator==(const path & rhs) const noexcept {
      u32_t sz = m_data.size();
      if (sz != rhs.m_data.size()) {
        return false;
      }
      // Compare from back to front, since last elements tend to have more variance.
      while (sz--) {
        if (!m_data[sz].operator==(rhs.m_data[sz])) {
          return false;
        }
      }
      return true;
    }

    bool operator<(const path & rhs) const noexcept;

    bool operator<=(const path & rhs) const noexcept;

    bool operator>(const path & rhs) const noexcept;

    bool operator>=(const path & rhs) const noexcept;

    J_INLINE_GETTER path_component * begin() noexcept {
      return m_data.begin();
    }

    J_INLINE_GETTER path_component * end() noexcept {
      return m_data.end();
    }

    J_INLINE_GETTER const path_component * begin() const noexcept {
      return m_data.begin();
    }

    J_INLINE_GETTER const path_component * end() const noexcept {
      return m_data.end();
    }

    template<typename... Args>
    J_ALWAYS_INLINE path_component & emplace_back(Args && ... args) {
      return m_data.emplace_back(static_cast<Args &&>(args)...);
    }

    void reserve(u32_t size);

    J_INLINE_GETTER bool is_root() const noexcept {
      return m_data.empty();
    }

    COMPONENT_TYPE type() const noexcept {
      return m_data.empty() ? COMPONENT_TYPE::EMPTY : m_data.back().type();
    }

    bool is_array_index() const noexcept {
      return !m_data.empty() && m_data.back().is_array_index();
    }

    bool is_map_key() const noexcept {
      return !m_data.empty() && m_data.back().is_map_key();
    }

    bool is_property_name() const noexcept {
      return !m_data.empty() && m_data.back().is_property_name();
    }

    J_INLINE_GETTER path_component & operator[](u32_t sz) noexcept {
      return m_data[sz];
    }

    J_INLINE_GETTER const path_component & operator[](u32_t sz) const noexcept {
      return m_data[sz];
    }

    J_INLINE_GETTER path_component & at(u32_t sz) {
      return m_data.at(sz);
    }

    J_INLINE_GETTER const path_component & at(u32_t sz) const {
      return m_data.at(sz);
    }

    J_INLINE_GETTER path_component & front() noexcept {
      return m_data.front();
    }

    J_INLINE_GETTER const path_component & front() const noexcept {
      return m_data.front();
    }

    J_INLINE_GETTER path_component & back() noexcept {
      return m_data.back();
    }

    J_INLINE_GETTER const path_component & back() const noexcept {
      return m_data.back();
    }

    bool is_ancestor_of(const path & rhs) const noexcept;

    bool is_ancestor_of_or_same(const path & rhs) const noexcept;
  };
}
