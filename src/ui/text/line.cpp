#include "ui/text/line.hpp"

#include "containers/vector.hpp"
#include "containers/trivial_array.hpp"

J_DEFINE_EXTERN_VECTOR(j::mem::shared_ptr<j::fonts::shaping::shaped_string>);
J_DEFINE_EXTERN_VECTOR(j::ui::text::visual_line);
J_DEFINE_EXTERN_TRIVIAL_ARRAY_COPYABLE(j::ui::text::positioned_string);
