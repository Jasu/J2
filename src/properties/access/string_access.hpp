#pragma once

#include "properties/access/typed_access.hpp"

namespace j::strings {
  class string;
  template<typename T> class basic_string_view;
  using const_string_view = basic_string_view<const char>;
}
namespace j::properties::access {
  class string_access_definition final : public typed_access_definition {
    friend class string_access;

    using get_size_t = sz_t (*)(const void*) noexcept;
    using as_j_string_t = strings::string (*)(const void*);
    using as_j_string_view_t = strings::const_string_view (*)(const void*) noexcept;
    using as_char_ptr_t = const char * (*)(const void*) noexcept;

    get_size_t m_get_size = nullptr;
    as_j_string_t m_as_j_string = nullptr;
    as_j_string_view_t m_as_j_string_view = nullptr;
    as_char_ptr_t m_as_char_ptr = nullptr;
  public:
    string_access_definition() noexcept;

    ~string_access_definition();

    void initialize_string_access(
      get_size_t get_size,
      as_j_string_t as_j_string,
      as_j_string_view_t as_j_string_view,
      as_char_ptr_t as_char_ptr
    ) noexcept;
  };

  class string_access final : public typed_access {
  public:
    using typed_access::typed_access;

    sz_t size() const noexcept {
      return static_cast<const string_access_definition &>(value_definition()).m_get_size(as_void_star());
    }

    strings::string as_j_string() const;

    strings::const_string_view as_j_string_view() const noexcept;

    const char * data() const noexcept {
      return static_cast<const string_access_definition &>(value_definition()).m_as_char_ptr(as_void_star());
    }
  };
}
