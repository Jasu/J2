#pragma once

#include "ui/attributes.hpp"
#include "ui/layout/box.hpp"
#include "ui/layout/size.hpp"
#include "ui/styling/style.hpp"
#include "attributes/basic_operations.hpp"
#include "geometry/rect.hpp"
#include "geometry/vec2.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::rendering::vulkan::inline command_buffers { struct command_buffer_builder; }

namespace j::ui {
  inline namespace rendering { struct context; }
  namespace g = j::geometry;
  /// Base class for UI elements.
  class element {
  public:
    virtual size min_size() const noexcept = 0;
    virtual g::vec2u16 max_content_size() const noexcept = 0;
    virtual styling::style style() const noexcept = 0;
    virtual void render(context & ctx, const g::rect_u16 & region) const = 0;
    virtual ~element();
  };

  /// Element whose all attributes, including size, are static.
  class static_element : public element {
  public:
    J_BOILERPLATE(static_element, CTOR_CE)

    template<typename... Attrs>
    J_ALWAYS_INLINE constexpr static_element(Attrs && ... attrs) noexcept
      : m_box(static_cast<Attrs &&>(attrs)...),
        m_style(static_cast<Attrs &&>(attrs)...)
    { }

    size min_size() const noexcept override final {
      return m_box.min_size();
    }

    virtual g::vec2u16 max_content_size() const noexcept override final {
      return m_box.max_content_size;
    }

    styling::style style() const noexcept override final {
      return m_style;
    }

  protected:
    layout::box m_box;
    styling::style m_style;
  };
}
