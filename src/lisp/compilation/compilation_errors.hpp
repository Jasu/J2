#pragma once

#include "lisp/sources/source_location.hpp"
#include "lisp/common/id.hpp"
#include "exceptions/exceptions.hpp"

namespace j::lisp::compilation {

  enum class compilation_error_type : u8_t {
    none,

    symbol_not_found,
    variable_not_found,
    function_not_found,
    function_recursive,
    variable_recursive,
    not_function,
    not_writable,

    arg_count_mismatch,
    too_few_args,
    too_many_args,

    arg_count_mismatch_range,
  };

  enum id_resolution_error : u8_t {
    symbol_not_found,
    variable_not_found,
    function_not_found,
    function_recursive,
    variable_recursive,
    not_function,
    not_writable,
  };

  enum arg_count_error : u8_t {
    arg_count_mismatch,
    too_few_args,
    too_many_args,
  };

  enum arg_range_error : u8_t {
    arg_count_mismatch_range,
  };


  struct compilation_error final {
    J_BOILERPLATE(compilation_error, CTOR_NE_ND)

    compilation_error(arg_count_error error, sources::source_location location, id form, u8_t expected, u8_t given) noexcept;

    compilation_error(arg_range_error error, sources::source_location location, id form, u8_t min, u8_t max, u8_t given) noexcept;

    compilation_error(id_resolution_error error, sources::source_location location, id form, id symbol_id) noexcept;

    compilation_error_type type = compilation_error_type::none;
    id current_symbol;
    sources::source_location location;

    union {
      struct {} no_error = {};
      id symbol_id;
      struct {
        id form;
        u8_t given;
        u8_t expected;
      } arg_count;

      struct {
        id form;
        u8_t given;
        u8_t min;
        u8_t max;
      } arg_range;
    };

    template<typename Fn> decltype(auto) visit(Fn && callback) const {
      switch (type) {
      case compilation_error_type::none:
        return static_cast<Fn &&>(callback)(type, location);
      case compilation_error_type::variable_not_found:
      case compilation_error_type::function_not_found:
      case compilation_error_type::symbol_not_found:
      case compilation_error_type::function_recursive:
      case compilation_error_type::variable_recursive:
      case compilation_error_type::not_function:
      case compilation_error_type::not_writable:
        return static_cast<Fn &&>(callback)(type, location, symbol_id);
      case compilation_error_type::arg_count_mismatch:
      case compilation_error_type::too_few_args:
      case compilation_error_type::too_many_args:
        return static_cast<Fn &&>(callback)(type, location, arg_count.form, arg_count.expected, arg_count.given);
      case compilation_error_type::arg_count_mismatch_range:
        return static_cast<Fn &&>(callback)(type, location, arg_range.form, arg_range.min, arg_range.max, arg_range.given);
      }
    }
  };

  [[noreturn]] void throw_compilation_error(sources::source_location loc, id form, id_resolution_error error, id symbol_id);
  [[noreturn]] void throw_compilation_error(sources::source_location loc, id form, arg_count_error error, u8_t got, u8_t expected);
  [[noreturn]] void throw_compilation_error(sources::source_location loc, id form, arg_range_error error, u8_t got, u8_t min, u8_t max);

  extern const tags::tag_definition<compilation_error> err_compilation_error;

  struct compilation_error_exception : public virtual j::exceptions::exception { };
}
