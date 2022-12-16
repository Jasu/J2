#include "hzd/demangle.hpp"
#include "exceptions/exceptions.hpp"
#include "hzd/mem.hpp"

#include <cxxabi.h>

namespace j {
  const char * demangle(const char * mangled_name) {
    namespace e = j::exceptions;
    J_ASSERT_NOT_NULL(mangled_name);
    int status = 1;
    if (const char * demangled_name = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status)) {
      return demangled_name;
    }

    switch (status) {
    case -1:
      throw e::bad_alloc_exception() << e::message("Memory allocation failed in demangle.");
    case -2:
      throw e::invalid_argument_exception() << e::message("Name is not a valid mangled name.");
    case -3:
      throw e::logic_error_exception() << e::message("BUG: demangle status -3");
    default:
      throw e::logic_error_exception() << e::message("Unknown status in demangle.");
    }
  }

  void demangle_free(const char * mangled_name) {
    J_ASSERT_NOT_NULL(mangled_name);
    ::j::free(const_cast<char*>(mangled_name));
  }
}
