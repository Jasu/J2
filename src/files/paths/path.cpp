#include "files/paths/path.hpp"
#include "exceptions/assert_lite.hpp"
#include "strings/string_algo.hpp"
#include "strings/parsing/parser.hpp"

namespace j::files::inline paths {
  bool path::is_directory() const noexcept {
    if (!m_path) {
      return false;
    }
    u32_t last = m_path.size() - 1U;
    if (m_path[last] == '/') {
      return true;
    } else if (m_path[last] != '.') {
      return false;
    }
    if (!last) {
      return true; // "."
    }
    --last;
    if (m_path[last] == '/') {
      return true; // "anything/."
    } else if (m_path[last] != '.') { // Cannot be .. or asdf/..
      return false;
    }
    return !last || m_path[last - 1] == '/';
  }

  strings::const_string_view path::extension() const noexcept {
    const u32_t sz = m_path.size();
    // than three characters (+ 1 for NUL) - "a.a" has an extension, but ".a" is a
    // hidden file.
    if (sz < 3 || m_path[sz - 1] == '/' || m_path[sz - 1] == '.') {
      return {};
    }
    for (u32_t i = sz - 2U; i != 0U; --i) {
      if (m_path[i] == '/') {
        break;
      }

      if (m_path[i] == '.') {
        if (m_path[i - 1] == '/') {
          break;
        }
        return strings::const_string_view(m_path.data() + i + 1U, m_path.end());
      }
    }
    return {};
  }

  [[nodiscard]] bool path::has_extension() const noexcept {
    const u32_t sz = m_path.size();
    // than three characters (+ 1 for NUL) - "a.a" has an extension, but ".a" is a
    // hidden file.
    if (sz < 3 || m_path[sz - 1] == '/' || m_path[sz - 1] == '.') {
      return false;
    }

    for (u32_t i = sz - 2U; i != 0U; --i) {
      if (m_path[i] == '/') {
        return false;
      } else if (m_path[i] == '.') {
        return m_path[i - 1] != '/';
      }
    }
    return false;
  }

  [[nodiscard]] path path::without_extension() const noexcept {
    if (auto e = extension()) {
      return path{m_path.data(), e.begin() - 1U};
    }
    return *this;
  }

  strings::const_string_view path::basename() const noexcept {
    i32_t sz = m_path.size();
    const char * data = m_path.data();
    /// Skip trailing slashes, exluding the first slash ("///" should return "/")
    while (sz > 1 && data[sz] == '/') { --sz; }

    /// For paths containing a single character, return the only character.
    /// "/" should return "/" and "" should return "".
    if (J_UNLIKELY(sz <= 1)) {
      return strings::const_string_view{data, sz};
    }

    i32_t i = sz;
    for (; i; --i) {
      if (data[i - 1] == '/') {
        break;
      }
    }

    return strings::const_string_view{data + i, sz - i};
  }

  path path::parent() const {
    J_ASSERT(m_path.size() >= 1, "Path cannot be empty..");
    // Remove trailing slashes
    strings::const_string_view s(m_path);
    i32_t num_ellipses = 0;
    do {
      maybe_skip_suffix(s, '/');
      maybe_skip_suffix(s, "/.");
      if (maybe_skip_suffix(s, "/..")) {
        ++num_ellipses;
        continue;
      }
      skip_suffix_until(s, '/');
      if (s) {
        --num_ellipses;
      }
      J_ASSERT_NOT_NULL(s);
    } while (num_ellipses > 0);
    J_ASSERT_NOT_NULL(s);
    return path(s);
  }

  path path::operator/(const path & tail) const noexcept {
    J_ASSERT(tail && tail.is_relative(), "Appended path must be relative.");
    u32_t size = m_path.size() + tail.m_path.size();
    bool had_separator = m_path && m_path[m_path.size() - 1U] == '/';
    if (!had_separator) {
      ++size;
    }
    strings::string s(size);
    auto ptr = s.data();
    ::j::memcpy(ptr, m_path.data(), m_path.size());
    ptr += m_path.size();
    if (!had_separator) {
      *ptr++ = '/';
    }
    ::j::memcpy(ptr, tail.m_path.data(), tail.m_path.size() + 1U);
    return path(static_cast<strings::string &&>(s));
  }

  path path::operator+(const strings::const_string_view & tail) const noexcept {
    return {m_path + tail};
  }

  path & path::operator+=(const strings::const_string_view & tail) noexcept {
    m_path += tail;
    return *this;
  }
  namespace {
    struct J_TYPE_HIDDEN path_parser final : strings::parsing::parser<path> {
      [[nodiscard]] path do_parse(const strings::const_string_view & value) const override {
        return path(strings::string(value));
      }
    };
    const path_parser path_parser_v;
  }
}
