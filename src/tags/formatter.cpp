#include "tags/formatter.hpp"

#include "tags/tag.hpp"
#include "strings/styling/styled_string.hpp"
#include "strings/format.hpp"
#include "strings/formatting/formatter_registry.hpp"
#include "services/service_definition.hpp"
#include "services/interface_definition.hpp"
#include "containers/vector.hpp"
#include "mem/shared_ptr.hpp"

namespace j::tags {
  namespace s = strings;
  tag_formatter_interface::~tag_formatter_interface() {
  }

  bool stringifier_tag_formatter::can_format(const tag & tag) const noexcept {
    return s::do_maybe_get_formatter(tag.value().type());
  }

  s::string stringifier_tag_formatter::format(const tag & tag) const {
    const s::formatting::format_value fv(tag.value().as_void_star(), tag.value().type());
    return s::detail::format(tag.definition().format_string, 1U, &fv);
  }

  s::styled_string stringifier_tag_formatter::format_styled(const tag & tag) const {
    const s::formatting::format_value fv(tag.value().as_void_star(), tag.value().type());
    return s::detail::format_styled(tag.definition().format_string, 1U, &fv);
  }

  tag_formatter::tag_formatter(vector<mem::shared_ptr<tag_formatter_interface>> && formatters) noexcept
    : m_formatters(static_cast<vector<mem::shared_ptr<tag_formatter_interface>> &&>(formatters))
  {
  }

  bool tag_formatter::can_format(const tag & tag) const noexcept {
    for (auto & f : m_formatters) {
      if (f->can_format(tag)) {
        return true;
      }
    }
    return false;
  }

  s::string tag_formatter::format(const tag & tag) const {
    for (auto & f : m_formatters) {
      if (f->can_format(tag)) {
        return f->format(tag);
      }
    }
    return "(no formatter)";
  }

  s::styled_string tag_formatter::format_styled(const tag & tag) const {
    for (auto & f : m_formatters) {
      if (f->can_format(tag)) {
        return f->format_styled(tag);
      }
    }
    return s::styled_string{"(no formatter)", {}};
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) const s::interface_definition<tag_formatter_interface> tag_formatter_interface_definition(
      "tags.formatter_interface",
      "Tag formatter interface"
    );

    J_A(ND, NODESTROY) const s::service_definition<tag_formatter> tag_formatter_definition(
      "tags.formatter",
      "Tag formatter",
      s::create = s::constructor<vector<mem::shared_ptr<tag_formatter_interface>> &&>()
    );

    J_A(ND, NODESTROY) const s::service_definition<stringifier_tag_formatter> stringifier_formatter_definition(
      "tags.formatter.stringifier",
      "Stringifier tag formatter",
      s::implements = s::interface<tag_formatter_interface>
    );
  }
}
