#include "exceptions/exceptions.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace j::fonts::rasterizing::freetype {
  extern const tags::tag_definition<FT_Error> ft_error;
  [[noreturn]] void fail_function(const char *function_name, FT_Error error);

  inline void call_checked(const char * const function_name, FT_Error result) {
    if (J_UNLIKELY(result != FT_Err_Ok)) {
      fail_function(function_name, result);
    }
  }
}

#define J_FT_CALL_CHECKED(FN, ...) \
  ::j::fonts::rasterizing::freetype::call_checked(#FN, FN(__VA_ARGS__))
