#include "strings/formatting/formatter_unknown_length.hpp"
#include "type_id/type_id.hpp"
#include "hzd/demangle.hpp"

namespace j::strings::formatters {
  namespace {
    styled_string demangle(const char * mangled_name) {
      const char * demangled_name = j::demangle(mangled_name);
      styled_string styled{demangled_name, {}};
      j::demangle_free(demangled_name);
      return styled;
    }

    template<typename T>
    class type_formatter final : public formatter_unknown_length<T> {
    public:
      styled_string do_format(
        const const_string_view &,
        const T & value
      ) const override {
        return demangle(value.name());
      }
    };

    J_A(ND, NODESTROY) const type_formatter<type_id::type_id> g_type_id_formatter;
    J_A(ND, NODESTROY) const type_formatter<std::type_info> g_type_info_formatter;
  }
}
