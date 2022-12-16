#pragma once

#include "type_id/type_id.hpp"
#include "mem/any.hpp"

#include <typeinfo>

namespace j::mem {
  class any;
}
namespace j::strings::inline parsing {
  class parser_base {
  protected:
    parser_base(const std::type_info & type) noexcept;
  public:
    [[nodiscard]] virtual mem::any parse(const const_string_view & value) const = 0;
    virtual ~parser_base();

    parser_base(const parser_base &) = delete;
    parser_base & operator=(const parser_base &) = delete;
  private:
    const type_id::type_id m_type;
  };


  template<typename T>
  class parser : public parser_base {
  public:
    J_ALWAYS_INLINE parser() noexcept : parser_base(typeid(T)) { }
    [[nodiscard]] J_ALWAYS_INLINE mem::any parse(const const_string_view & value) const override final {
      return mem::any{do_parse(value)};
    }

    [[nodiscard]] virtual T do_parse(const const_string_view & value) const = 0;
  };
}
