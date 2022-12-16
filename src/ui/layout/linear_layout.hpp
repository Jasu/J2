#pragma once

#include "ui/layout/flags.hpp"
#include "ui/element.hpp"
#include "ui/rendering/element_frame.hpp"
#include "containers/vector.hpp"
#include "mem/shared_ptr_fwd.hpp"

J_DECLARE_EXTERN_NONCOPYABLE_VECTOR(j::mem::shared_ptr<j::ui::element>);

namespace j::ui::inline layout {
  class linear_layout : public element {
  public:
    J_BOILERPLATE(linear_layout, CTOR_CE, COPY_DEL, MOVE_DEL)

    template<typename... Attrs>
    explicit constexpr linear_layout(Attrs && ... attrs) noexcept
      : m_box(static_cast<Attrs &&>(attrs)...),
        m_style(static_cast<Attrs &&>(attrs)...)
      {
        namespace a = ui::attributes;
        namespace ja = j::attributes;
        if constexpr (ja::has<Attrs...>(a::orientation)) {
          if (a::orientation.get(static_cast<Attrs &&>(attrs)...) == orientation::columns) {
            m_flags |= layout_flags_t::orientation_columns;
          }
        }
        if constexpr (ja::has<Attrs...>(a::vertical_gravity)) {
          switch (a::vertical_gravity.get(static_cast<Attrs &&>(attrs)...)) {
          case gravity::start:
            m_flags |= layout_flags_t::vertical_gravity_start;
            break;
          case gravity::end:
            m_flags |= layout_flags_t::vertical_gravity_end;
            break;
          case gravity::center:
            break;
          }
        } else {
          m_flags |= layout_flags_t::vertical_gravity_start;
        }

        if constexpr (ja::has<Attrs...>(a::horizontal_gravity)) {
          switch (a::horizontal_gravity.get(static_cast<Attrs &&>(attrs)...)) {
          case gravity::start:
            m_flags |= layout_flags_t::horizontal_gravity_start;
            break;
          case gravity::end:
            m_flags |= layout_flags_t::horizontal_gravity_end;
            break;
          case gravity::center:
            break;
          }
        } else {
          m_flags |= layout_flags_t::horizontal_gravity_start;
        }

        if constexpr (ja::has<Attrs...>(a::collapse_margins)) {
          auto c = a::collapse_margins.get(static_cast<Attrs &&>(attrs)...);
          if (c.has(g::side::top)) {
            m_flags.set(layout_flags_t::collapse_margin_top);
          }
          if (c.has(g::side::bottom)) {
            m_flags.set(layout_flags_t::collapse_margin_bottom);
          }
          if (c.has(g::side::left)) {
            m_flags.set(layout_flags_t::collapse_margin_left);
          }
          if (c.has(g::side::right)) {
            m_flags.set(layout_flags_t::collapse_margin_right);
          }
        }
      }

    void push_back(mem::shared_ptr<element> e);

    size min_size() const noexcept override;
    g::vec2u16 max_content_size() const noexcept override;

    constexpr gravity horizontal_gravity() const noexcept {
      if (m_flags.has(layout_flags_t::horizontal_gravity_start)) {
        return gravity::left;
      } else if (m_flags.has(layout_flags_t::horizontal_gravity_end)) {
        return gravity::right;
      } else {
        return gravity::center;
      }
    }

    constexpr gravity vertical_gravity() const noexcept {
      if (m_flags.has(layout_flags_t::vertical_gravity_start)) {
        return gravity::top;
      } else if (m_flags.has(layout_flags_t::vertical_gravity_end)) {
        return gravity::bottom;
      } else {
        return gravity::center;
      }
    }

    constexpr gravity main_gravity() const noexcept {
      return m_flags.has(layout_flags_t::orientation_columns)
        ? horizontal_gravity() : vertical_gravity();
    }

    constexpr gravity cross_gravity() const noexcept {
      return m_flags.has(layout_flags_t::orientation_columns)
        ? vertical_gravity() : horizontal_gravity();
    }

    constexpr enum orientation orientation() const noexcept {
      return m_flags.has(layout_flags_t::orientation_columns)
        ? orientation::columns : orientation::rows;
    }

    styling::style style() const noexcept override {
      return m_style;
    }

    g::axis main_axis() const noexcept {
      return m_flags.has(layout_flags_t::orientation_columns)
        ? g::axis::horizontal : g::axis::vertical;
    }

    g::axis cross_axis() const noexcept {
      return m_flags.has(layout_flags_t::orientation_columns)
        ? g::axis::vertical : g::axis::horizontal;
    }

    void render(context & ctx, const g::rect_u16 & region) const override;
  private:
    noncopyable_vector<mem::shared_ptr<element>> m_elements;
    box m_box;
    styling::style m_style;
    layout_flags m_flags;
    element_frame m_frame;

    bool collapse_main_before() const noexcept;
    bool collapse_main_after() const noexcept;
    bool collapse_cross_before() const noexcept;
    bool collapse_cross_after() const noexcept;
  };
}
