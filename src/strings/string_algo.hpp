#pragma once

#include "strings/string_view.hpp"
#include "hzd/ctype.hpp"
#include "exceptions/assert_lite.hpp"
#include "hzd/type_traits.hpp"

namespace j::strings {
  template<typename Str, typename Fn>
  [[nodiscard]] i32_t find_first_matching(const Str & str, Fn && fn, i32_t start = 0) noexcept {
    const char * const data = str.data();
    const i32_t sz = str.size();
    for (; start < sz && !static_cast<Fn &&>(fn)(data[start]); ++start) { }
    return start == sz ? Str::npos : start;
  }

  template<typename Str, typename Fn>
  [[nodiscard]] i32_t find_last_matching(const Str & str, Fn && fn) noexcept {
    const char * const data = str.data();
    const i32_t sz = str.size();
    i32_t i = sz - 1;
    for (; i < sz && !static_cast<Fn &&>(fn)(data[i]); --i) { }
    return i;
  }

  template<typename Str, typename Fn>
  J_INLINE_GETTER i32_t find_first_not_matching(const Str & str, Fn && fn, i32_t start = 0) noexcept {
    const char * const data = str.data();
    const i32_t sz = str.size();
    for (; start < sz && static_cast<Fn &&>(fn)(data[start]); ++start) { }
    return start == sz ? Str::npos : start;
  }

  template<typename Str, typename Fn>
  J_INLINE_GETTER i32_t find_last_not_matching(const Str & str, Fn && fn) noexcept {
    const char * const data = str.data();
    const i32_t sz = str.size();
    i32_t i = sz - 1;
    for (; i < sz && static_cast<Fn &&>(fn)(data[i]); --i) { }
    return i;
  }

  template<typename Str>
  J_INLINE_GETTER i32_t find_first(const Str & str, const char c, i32_t start = 0) noexcept {
    const char * const begin = str.data();
    const char * const found = ::j::memchr(begin + start, c, str.size());
    return found ? found - begin : Str::npos;
  }

  template<typename Str>
  J_INLINE_GETTER i32_t find_last(const Str & str, const char c) noexcept
  { return find_last_matching(str, [c](char c2) { return c == c2; }); }

  template<typename Str>
  J_INLINE_GETTER i32_t find_first_not(const Str & str, const char c, i32_t start = 0) noexcept
  { return find_first_not_matching(str, [c](char c2) { return c == c2; }, start); }

  template<typename Str>
  J_INLINE_GETTER i32_t find_last_not(const Str & str, const char c) noexcept
  { return find_last_not_matching(str, [c](char c2) { return c == c2; }); }

  template<typename Str>
  J_INLINE_GETTER i32_t find_last_of(const Str & str, const char * const chars) noexcept
  { return find_last_matching(str, [chars](char c) { return (bool)::j::strchr(chars, c); }); }

  template<typename Str>
  J_INLINE_GETTER i32_t find_first_not_of(const Str & str, const char * const chars, i32_t start = 0) noexcept
  { return find_first_not_matching(str, [chars](char c) { return (bool)::j::strchr(chars, c); }, start); }

  template<typename Str>
  J_INLINE_GETTER i32_t find_last_not_of(const Str & str, const char * const chars) noexcept
  { return find_last_not_matching(str, [chars](char c) { return (bool)::j::strchr(chars, c); }); }

  template<typename Str>
  J_INLINE_GETTER i32_t find_first_unescaped(const Str & str, const char c, i32_t start = 0) noexcept {
    for (;;) {
      start = find_first(str, c, start);
      if (start == Str::npos || start == 0 || str[start - 1] != '\\') {
        return start;
      }
      ++start;
    }
  }

  template<typename Char>
  J_INLINE_GETTER basic_string_view<Char> ltrim(basic_string_view<Char> str, const char c) noexcept
  { return str.without_prefix(find_first_not(str, c)); }

  template<typename Char>
  J_INLINE_GETTER basic_string_view<Char> ltrim(basic_string_view<Char> str, const char * const chars) noexcept
  { return str.without_prefix(find_first_not_of(str, chars)); }

  template<typename Char>
  J_INLINE_GETTER basic_string_view<Char> ltrim(basic_string_view<Char> str) noexcept
  { return str.without_prefix(find_first_not_of(str, " \t\n\r")); }

  template<typename Char>
  J_INLINE_GETTER basic_string_view<Char> rtrim(basic_string_view<Char> str, const char * const chars) noexcept
  { return str.prefix(find_last_not_of(str, chars) + 1); }

  template<typename Char>
  J_INLINE_GETTER basic_string_view<Char> rtrim(basic_string_view<Char> str, const char c) noexcept
  { return str.prefix(find_last_not(str, c) + 1); }

  template<typename Char>
  J_INLINE_GETTER basic_string_view<Char> rtrim(basic_string_view<Char> str) noexcept
  { return str.prefix(find_last_not_of(str, " \t\n\r") + 1); }

  template<typename Char>
  J_INLINE_GETTER basic_string_view<Char> trim(basic_string_view<Char> str, const char * const chars) noexcept
  { return ltrim(rtrim(str, chars), chars); }

  template<typename Char>
  J_INLINE_GETTER basic_string_view<Char> trim(basic_string_view<Char> str, const char c) noexcept
  { return ltrim(rtrim(str, c), c); }

  template<typename Char>
  J_INLINE_GETTER basic_string_view<Char> trim(basic_string_view<Char> str) noexcept
  { return ltrim(rtrim(str)); }


  template<typename Str1, typename Str2>
  J_INLINE_GETTER bool starts_with(const Str1 & subject, const Str2 & prefix) noexcept {
    const i32_t sz = prefix.size();
    if (subject.size() < sz) {
      return false;
    }
    return ::j::memcmp(subject.data(), prefix.data(), sz) == 0;
  }

  template<typename Str1, typename Str2>
  J_INLINE_GETTER bool ends_with(const Str1 & subject, const Str2 & suffix) noexcept {
    const i32_t sz = suffix.size(), subject_sz = subject.size();
    if (subject_sz < sz) {
      return false;
    }
    return ::j::memcmp(subject.data() + subject_sz - sz, suffix.data(), sz) == 0;
  }

  template<typename Str>
  J_INLINE_GETTER bool starts_with(const Str & subject, char prefix) noexcept
  { return !subject.empty() && subject.front() == prefix; }

  template<typename Str>
  J_INLINE_GETTER bool ends_with(const Str & subject, char suffix) noexcept
  { return !subject.empty() && subject.back() == suffix; }

  template<typename C>
  J_ALWAYS_INLINE bool maybe_skip_prefix(basic_string_view<C> & subject, const_string_view prefix) noexcept {
    if (!starts_with(subject, prefix)) {
      return false;
    }
    subject.remove_prefix(prefix.size());
    return true;
  }

  template<typename C>
  J_ALWAYS_INLINE bool maybe_skip_suffix(basic_string_view<C> & subject, const_string_view suffix) noexcept {
    if (!ends_with(subject, suffix)) {
      return false;
    }
    subject.remove_suffix(suffix.size());
    return true;
  }

  template<typename C>
  J_ALWAYS_INLINE bool maybe_skip_prefix(basic_string_view<C> & subject, char prefix) noexcept {
    if (!starts_with(subject, prefix)) {
      return false;
    }
    subject.remove_prefix(1);
    return true;
  }

  template<typename C>
  J_ALWAYS_INLINE bool maybe_skip_suffix(basic_string_view<C> & subject, char suffix) noexcept {
    if (!ends_with(subject, suffix)) {
      return false;
    }
    subject.remove_suffix(1);
    return true;
  }

  template<typename C>
  void skip_prefix(basic_string_view<C> & subject, const_string_view prefix) {
    if (J_UNLIKELY(!maybe_skip_prefix(subject, prefix))) {
      J_THROW("Prefix not found.");
    }
  }

  template<typename C>
  void skip_prefix(basic_string_view<C> & subject, char prefix) {
    if (J_UNLIKELY(!maybe_skip_prefix(subject, prefix))) {
      J_THROW("Prefix not found.");
    }
  }

  template<typename C>
  void skip_suffix(basic_string_view<C> & subject, const_string_view suffix) {
    if (J_UNLIKELY(!maybe_skip_suffix(subject, suffix))) {
      J_THROW("Suffix not found.");
    }
  }

  template<typename C>
  void skip_suffix(basic_string_view<C> & subject, char suffix) {
    if (J_UNLIKELY(!maybe_skip_suffix(subject, suffix))) {
      J_THROW("Suffix not found.");
    }
  }



  template<typename C, typename Fn>
  [[nodiscard]] basic_string_view<C> take_while(basic_string_view<C> & subject, Fn && fn) noexcept {
    const i32_t pos = find_first_not_matching(subject, static_cast<Fn &&>(fn));
    const basic_string_view<C> result = subject.prefix(pos);
    return subject.remove_prefix(pos), result;
  }

  template<typename C, typename Fn>
  [[nodiscard]] basic_string_view<C> take_until(basic_string_view<C> & subject, Fn && fn) noexcept {
    const i32_t pos = find_first_matching(subject, static_cast<Fn &&>(fn));
    const basic_string_view<C> result = subject.prefix(pos);
    return subject.remove_prefix(pos), result;
  }

  template<typename C>
  [[nodiscard]] basic_string_view<C> take_until(basic_string_view<C> & subject, char c) noexcept {
    const i32_t pos = find_first(subject, c);
    const basic_string_view<C> result = subject.prefix(pos);
    return subject.remove_prefix(pos), result;
  }

  template<typename C>
  [[nodiscard]] char take_char(basic_string_view<C> & subject) noexcept {
    J_REQUIRE(!subject.empty(), "Empty subject.");
    const char result = subject.front();
    return subject.remove_prefix(1), result;
  }

  template<typename Int, typename C>
  [[nodiscard]] Int take_integer(basic_string_view<C> & subject) noexcept {
    bool is_negative = false;
    if constexpr (is_signed_v<Int>) {
      if (subject.maybe_skip_prefix('-')) {
        is_negative = true;
      }
    }
    J_REQUIRE(!subject.empty() && is_digit(subject.front()), "Not an integer.");
    Int result = 0;
    do {
      result *= 10;
      result += subject.front() - '0';
      subject.remove_prefix(1);
    } while (!subject.empty() && is_digit(subject.front()));
    return is_negative ? -result : result;
  }

  template<typename C>
  bool skip_after(basic_string_view<C> & subject, char c) noexcept {
    i32_t pos = find_first(subject, c);
    subject.remove_prefix(pos < 0 ? -1 : pos + 1);
    return pos >= 0;
  }

  template<typename C, typename Fn>
  void skip_suffix_until(basic_string_view<C> & subject, Fn && fn) noexcept {
    subject.remove_suffix(subject.size() - find_last_matching(subject, static_cast<Fn &&>(fn)));
  }

  template<typename C>
  void skip_suffix_until(basic_string_view<C> & subject, char c) noexcept {
    subject.remove_suffix(subject.size() - find_last(subject, c));
  }

  template<typename C>
  [[nodiscard]] basic_string_view<C> suffix_until(basic_string_view<C> subject, char c) noexcept {
    auto res = find_last(subject, c);
    return res == basic_string_view<C>::npos
      ? subject
      : subject.suffix(subject.size() - res - 1);
  }
}
