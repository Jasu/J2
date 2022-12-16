#version 450

// Specialization constants
layout (constant_id = 0) const float atlas_size_px = 512.f;

// Instance attributes
layout(location = 0) in vec4 texture_rect;
layout(location = 1) in ivec2 screen_pos;

// Uniforms
layout(binding = 1) uniform renderer_state_t {
  vec2 viewport_size_px_inv;
  vec2 viewport_size_px;
} renderer_state;

// Push constants
layout(push_constant) uniform u {
  layout(offset=0) vec2 position;
} u_position;

// Outputs
layout(location = 0) out vec2 glyph_atlas_coord;

const vec2 positions[4] = {{0.0f, 0.0f},
                           {1.0f, 0.0f},
                           {0.0f, 1.0f},
                           {1.0f, 1.0f}};

void main() {
  const vec2 position = positions[gl_VertexIndex];
  gl_Position = vec4(
          renderer_state.viewport_size_px_inv
            * (screen_pos * 0.25f
               + position * texture_rect.zw * atlas_size_px
               + u_position.position),
          1.0, 1.0);
  glyph_atlas_coord = texture_rect.xy + position * texture_rect.zw;
}
