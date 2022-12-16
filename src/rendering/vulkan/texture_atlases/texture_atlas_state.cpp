#include "rendering/vulkan/texture_atlases/texture_atlas_state.hpp"

#include "rendering/vulkan/texture_atlases/texture_atlas.hpp"
#include "rendering/vulkan/command_buffers/copy_from_buffer_to_image_command.hpp"
#include "rendering/vulkan/state/state_variable.hpp"
#include "rendering/images/image_buffer_info.hpp"
#include "rendering/images/image_source_manager.hpp"
#include "exceptions/assert_lite.hpp"
#include "strings/format.hpp"

namespace j::rendering::vulkan {
  namespace i = j::rendering::images;
  namespace s = j::rendering::vulkan::state;
  namespace ta = j::rendering::texture_atlas;
  inline namespace texture_atlases {
    namespace {
      const s::postcondition_definition loads_image_def{s::noninvalidating_flag};

      class J_TYPE_HIDDEN requires_image_precondition final : public s::precondition_definition {
      public:
        constexpr requires_image_precondition() noexcept
          : s::precondition_definition(s::modal_flag)
        { }

        bool get_initial_state(const s::state_variable & state, uptr_t image_key_uptr) const override {
          J_ASSERT_NOT_NULL(image_key_uptr, state.data);
          auto atlas = reinterpret_cast<ta::texture_atlas*>(state.data);
          return atlas->contains(image_key_uptr, true);
        }

        s::precondition_change get_change(const s::postcondition_instance & postcondition,
                                          uptr_t image_key_uptr) const override
        {
          return postcondition.data != image_key_uptr
            ? s::precondition_change::unchanged
            : s::precondition_change::satisfied;
        }
      };

      const requires_image_precondition requires_image_def;
    }

    s::postcondition_initializer texture_atlas_state_variable::loads_image(
      texture_atlas_ref atlas, i::image_source_key image
    ) const noexcept {
      J_ASSERT_NOT_NULL(atlas, image);
      return {
        atlas.wrapper,
        0x400,
        &loads_image_def,
        this,
        (uptr_t)image
      };
    }

    s::precondition_initializer texture_atlas_state_variable::requires_image(
      texture_atlas_ref atlas, i::image_source_key image
    ) const noexcept {
      J_ASSERT_NOT_NULL(atlas, image);
      return {
        atlas.wrapper,
        0x400,
        &requires_image_def,
        this,
        (uptr_t)image
      };
    }

    strings::string texture_atlas_state_variable::describe_precondition(
      resources::resource_wrapper *, uptr_t, const s::precondition_instance & c
    ) const {
      return strings::format("Atlas contains {}",
        i::image_source_manager::instance.describe(i::image_source_key(c.data)));
    }

    strings::string texture_atlas_state_variable::describe_postcondition(
      resources::resource_wrapper *, uptr_t, const s::postcondition_instance & c
    ) const {
      return strings::format("Load {} to atlas",
        i::image_source_manager::instance.describe(i::image_source_key(c.data)));
    }

    void texture_atlas_state_variable::initialize_state(
      resources::resource_wrapper * wrapper,
      uptr_t,
      uptr_t & state) const noexcept
    {
      J_ASSERT_NOT_NULL(wrapper);
      J_ASSERT(wrapper->status == resources::wrapper_status::initialized, "Uninitialized wrapper");
      auto atlas = &reinterpret_cast<texture_atlas*>(wrapper->data)->atlas;
      atlas->tick_lru();
      state = reinterpret_cast<uptr_t>(atlas);
    }

    bool texture_atlas_state_variable::do_can_transition(const s::transition_context &) const noexcept
    { return true; }

    s::condition_group * texture_atlas_state_variable::do_transition(s::transition_context & context) const {
      J_ASSERT_NOT_NULL(context.wrapper);
      auto & atlas = reinterpret_cast<texture_atlas*>(context.wrapper->data)->atlas;
      auto imgref = reinterpret_cast<texture_atlas*>(context.wrapper->data)->image;
      i::image_source_key k(context.to.data);

      auto info = i::image_source_manager::instance.get_info(k);
      ta::texture_record * res;
      while (true) {
        res = atlas.insert(context.to.data, {info.width(), info.height()}, {info.bearing_x(), info.bearing_y()});
        if (J_LIKELY(res)) {
          break;
        }
        atlas.erase();
      }
      return ::new command_buffers::copy_from_buffer_to_image_command(
        context.context,
        static_cast<images::image_ref &&>(imgref),
        res->rect,
        k,
        loads_image(texture_atlas_ref(context.wrapper), k));
    }

    const texture_atlas_state_variable texture_atlas_contents{};
  }
}
