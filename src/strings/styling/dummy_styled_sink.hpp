#pragma once

#include "strings/styling/style.hpp"
#include "strings/styling/styled_sink.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::strings {
  inline namespace styling {
    class dummy_styled_sink final : public styled_sink {
    public:
      dummy_styled_sink() noexcept;
      explicit dummy_styled_sink(mem::shared_ptr<streams::sink> sink);

      dummy_styled_sink(const dummy_styled_sink & rhs) noexcept = delete;
      dummy_styled_sink(dummy_styled_sink && rhs) noexcept;
      dummy_styled_sink & operator=(const dummy_styled_sink & rhs) noexcept = delete;
      dummy_styled_sink & operator=(dummy_styled_sink && rhs) noexcept;

      ~dummy_styled_sink();

      u32_t write(const char * J_NOT_NULL from, u32_t num_bytes) noexcept override;
      char * get_write_buffer(u32_t num_bytes) override J_RETURNS_NONNULL;
      void flush() override;
      bool is_tty() const override;
      void set_style(const style & style) noexcept override;
      style get_style() const noexcept override;
      u32_t write_styled(styling::replace_style_tag,
                         const strings::style & style,
                         const char * J_NOT_NULL from, u32_t num_bytes) override;

      u32_t get_column() noexcept override;
      void reset_column() noexcept override;
      void pad_to_column(u32_t i) override;
    private:
      char * m_buffer = nullptr;
      mem::shared_ptr<streams::sink> m_sink;
      u32_t m_write_pos = 0U;
      u32_t m_column = 0U;
      style m_current_style;
    };
  }
}
