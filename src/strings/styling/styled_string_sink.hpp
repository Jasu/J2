#pragma once

#include "strings/styling/styled_sink.hpp"
#include "strings/styling/styled_string.hpp"

namespace j::strings {
  inline namespace styling {
    class styled_string_sink final : public styled_sink {
    public:
      explicit styled_string_sink(i32_t size);

      styled_string_sink(styled_string_sink && rhs) = delete;
      styled_string_sink & operator=(styled_string_sink && rhs) = delete;
      styled_string_sink(const styled_string_sink & rhs) = delete;
      styled_string_sink & operator=(const styled_string_sink & rhs) = delete;

      ~styled_string_sink();

      u32_t get_column() noexcept override;
      void reset_column() noexcept override;
      void pad_to_column(u32_t i) override;

      u32_t write(const char * from, u32_t num_bytes) noexcept override;
      void flush() noexcept override;
      char * get_write_buffer(u32_t sz) override;
      void set_style(const style & style) noexcept override;
      style get_style() const override;
      u32_t write_styled(replace_style_tag, const style & style, const char * from, u32_t num_bytes) override;

      J_INLINE_GETTER const styled_string & string() const noexcept
      { return m_string; }
      J_INLINE_GETTER styled_string & string() noexcept
      { return m_string; }
    private:
      styled_string m_string;
      char * m_write_ptr;
      u32_t m_column = 0U;
    };
  }
}
