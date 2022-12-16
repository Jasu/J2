#pragma once

#include "mem/shared_ptr_fwd.hpp"

namespace j::strings {
  inline namespace formatting {
    class formatted_sink;
  }
}
namespace j::tags {
  struct tag_formatter;
}
namespace j::exceptions {
  struct exception;

  class exception_formatter final {
  public:
    exception_formatter(mem::shared_ptr<tags::tag_formatter> tag_formatter) noexcept;

    /// Write exception e to sink.
    ///
    /// Note that this cannot guarantee noexcept, since writing to the sink might throw.
    void format(strings::formatted_sink & sink, exception & e) const;
  private:
    mem::shared_ptr<tags::tag_formatter> m_tag_formatter;
  };
}
