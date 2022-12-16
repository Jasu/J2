#include "lisp/compilation/compilation_errors.hpp"
#include "lisp/compilation/compilation_error_formatter.hpp"

namespace j::lisp::compilation {
  const tags::tag_definition<compilation_error> err_compilation_error{"Compilation error"};

  compilation_error::compilation_error(arg_count_error error, sources::source_location location, id form, u8_t expected, u8_t given) noexcept
  : type((compilation_error_type)error),
    current_symbol(form),
    location(location),
    arg_count{form, given, expected}
  { }

  compilation_error::compilation_error(arg_range_error error, sources::source_location location, id form, u8_t min, u8_t max, u8_t given) noexcept
  : type((compilation_error_type)error),
    current_symbol(form),
    location(location),
    arg_range{form, given, min, max}
  { }

  compilation_error::compilation_error(id_resolution_error error, sources::source_location location, id form, id symbol_id) noexcept
  : type((compilation_error_type)error),
    current_symbol(form),
    location(location),
    symbol_id(symbol_id)
  { }

  [[noreturn]] void throw_compilation_error(sources::source_location loc, id form, id_resolution_error error, id symbol) {
    J_THROW(compilation_error_exception() << err_compilation_error(compilation_error(error, loc, form, symbol)));
  }

  [[noreturn]] void throw_compilation_error(sources::source_location loc, id form, arg_count_error error,  u8_t got, u8_t expected) {
    J_THROW(compilation_error_exception() << err_compilation_error(compilation_error(error, loc, form, got, expected)));
  }

  [[noreturn]] void throw_compilation_error(sources::source_location loc, id form, arg_range_error error, u8_t got, u8_t min, u8_t max) {
    J_THROW(compilation_error_exception() << err_compilation_error(compilation_error(error, loc, form, got, min, max)));
  }
}
