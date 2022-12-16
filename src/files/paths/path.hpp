#pragma once

#include "strings/string.hpp"

namespace j::files::inline paths {
  /// Null-terminated string representing an absolute or relative path.
  ///
  /// This class only handles the lexical aspect of paths - what the path
  /// actually points to is not considered by any member function.
  class path final {
  public:
    J_BOILERPLATE(path, CTOR_NE_ND)

    /// Create a path from string, ensuring that it is null-terminated.
    template<typename... Args>
    J_A(AI,ND) inline constexpr path(Args && ... args) noexcept(is_nothrow_constructible_v<strings::string, Args && ...>)
      : m_path(static_cast<Args &&>(args)...)
    {
    }

    /// Return whether the path starts with a slash.
    J_INLINE_GETTER bool is_absolute() const noexcept {
      return m_path && m_path[0] == '/';
    }

    /// Return whether the path does not start with a slash.
    J_INLINE_GETTER bool is_relative() const noexcept {
      return m_path && m_path[0] != '/';
    }

    /// Return whether the path ends with a slash, "." or "..".
    ///
    /// \note The check is purely lexical - whether the path actually points
    /// to a directory is not considered.
    /// \note A negative result means that the path may either refer to a file
    /// or a directory.
    [[nodiscard]] bool is_directory() const noexcept;

    /// Return whether the path is an empty string (i.e. invalid).
    [[nodiscard]] inline bool empty() const noexcept {
      return m_path.empty();
    }

    [[nodiscard]] inline explicit operator bool() const noexcept {
      return !empty();
    }

    [[nodiscard]] inline bool operator!() const noexcept {
      return empty();
    }

    /// Get the null-terminated string of the path.
    J_INLINE_GETTER const strings::string & as_string() const noexcept {
      return m_path;
    }

    J_INLINE_GETTER explicit operator const strings::string &() const noexcept {
      return m_path;
    }

    /// Get the null-terminated string of the path.
    [[nodiscard]] inline const char * as_c_string() const noexcept { return m_path.data(); }

    /// Get the file extension, if any.
    ///
    /// If there is no extension, returns an empty const_string_view.
    ///
    /// Extension is defined as characters following the last dot of the last
    /// path component, unless the last dot is the first character of the
    /// component. (i.e. "asdf.tar.gz" -> "gz", "/.htaccess" -> "").
    [[nodiscard]] strings::const_string_view extension() const noexcept;

    [[nodiscard]] path without_extension() const noexcept;

    [[nodiscard]] bool has_extension() const noexcept;

    /// Get the basename, including the extension (if any).
    ///
    /// Basename behavior is as follows:
    /// "hello.txt" => "hello.txt"
    /// "foo/hello.txt" => "hello.txt"
    /// "/foo/bar/" => "bar"
    /// "/foo/." => "."
    /// "/foo/.." => ".."
    /// "/" => "/"
    /// "//" => "/"
    /// "/../" => ".."
    /// "/./" => "."
    /// "" => ""
    strings::const_string_view basename() const noexcept;

    /// Get the parent directory of the path.
    path parent() const;

    path operator+(const strings::const_string_view & tail) const noexcept;
    path & operator+=(const strings::const_string_view & tail) noexcept;

    path operator/(const path & tail) const noexcept;

    /// Return whether paths are the same string.
    ///
    /// Paths "." and "./" will be considered as different - this performs
    /// a pure string comparison.
    J_INLINE_GETTER bool operator==(const path &) const noexcept = default;
  private:
    friend struct path_comparer;
    friend struct path_hash;

    strings::string m_path;
  };
}
