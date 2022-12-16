#include "lisp/sources/source.hpp"

#include "lisp/sources/include_dirs.hpp"
#include "lisp/values/lisp_str.hpp"

namespace j::lisp::sources {
  source_location::source_location(const source & src, u32_t line, u32_t column) noexcept
    : source_location(src.index, line, column)
  { }

  source::source(source_type type, u32_t index, u32_t include_dir_index, lisp_str * J_NOT_NULL name) noexcept
    : type(type),
      index(index),
      include_dir_index(include_dir_index),
      name(name)
  {
    J_ASSUME(type != source_type::none);
    J_ASSUME(type == source_type::file || !include_dir_index);
  }

  strings::const_string_view source::name_str() const noexcept {
    return J_LIKELY(name) ? name->value() : "";
  }

  source::~source() {
  }

  file_source::file_source(u16_t index,
                           struct include_dir * J_NOT_NULL dir,
                           lisp_str * J_NOT_NULL path)
    : source(source_type::file, index, dir->index, path),
      include_dir(dir),
      mapping(dir->dir.open_at(strings::string(path->value()), files::open_flags::read), files::map_full_range)
  {
  }

  void file_source::do_open() noexcept {
  }

  void file_source::do_close() noexcept {
  }

  strings::const_string_view file_source::get_source() {
    return {mapping.data(), mapping.size()};
  }

  string_source::string_source(
    bool is_dynamic,
    u16_t index,
    lisp_str * J_NOT_NULL name,
    strings::const_string_view src) noexcept
    : source(is_dynamic ? source_type::dynamic_string : source_type::static_string, index, 0U, name),
      src(src)
  { }

  void string_source::do_open() {
  }

  void string_source::do_close() noexcept {
  }

  strings::const_string_view string_source::get_source() {
    return src;
  }
}
