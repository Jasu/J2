#pragma once

#include "tags/tag.hpp"
#include "containers/vector.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::strings {
  class stringifier;
  class string;
  inline namespace styling {
    struct styled_string;
  }
}

namespace j::tags {
  struct tag;

  struct tag_formatter_interface {
  public:
    virtual ~tag_formatter_interface();

    virtual bool can_format(const tag & tag) const noexcept = 0;

    virtual strings::string format(const tag & tag) const = 0;
    virtual strings::styled_string format_styled(const tag & tag) const = 0;
  };

  /// Convert tag to a string with j::stringifier.
  struct stringifier_tag_formatter final : public tag_formatter_interface {
  public:
    bool can_format(const tag & tag) const noexcept override;

    strings::string format(const tag & tag) const override;
    strings::styled_string format_styled(const tag & tag) const override;
  };

  struct tag_formatter final {
  public:
    explicit tag_formatter(vector<mem::shared_ptr<tag_formatter_interface>> && formatters) noexcept;

    bool can_format(const tag & tag) const noexcept;

    strings::string format(const tag & tag) const;
    strings::styled_string format_styled(const tag & tag) const;
  private:
    vector<mem::shared_ptr<tag_formatter_interface>> m_formatters;
  };

}
