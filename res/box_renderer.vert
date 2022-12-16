#version 450

// Instance attributes
layout(location = 0) in ivec4 rect;
layout(location = 1) in vec4 rgba;
layout(location = 2) in vec4 border_left_rgba;
layout(location = 3) in vec4 border_right_rgba;
layout(location = 4) in vec4 border_top_rgba;
layout(location = 5) in vec4 border_bottom_rgba;
// Left top right bottom
layout(location = 6) in uvec4 border_size;
layout(location = 7) in uvec4 corner_radius;

// Uniforms
layout(binding = 0) uniform renderer_state_t {
  vec2 viewport_size_px_inv;
  vec2 viewport_size_px;
} renderer_state;

// Outputs
layout(location = 0) out vec2 coord_out;
layout(location = 1) flat out vec4 rgba_out;
layout(location = 2) flat out mat4 border_colors_rgba_out;
// layout(location = 2) out vec4 border_left_rgba_out;
// layout(location = 3) out vec4 border_right_rgba_out;
// layout(location = 4) out vec4 border_top_rgba_out;
// layout(location = 5) out vec4 border_bottom_rgba_out;
// Left top right bottom
layout(location = 6) flat out vec4 border_size_out;
layout(location = 7) flat out vec4 corner_radius_out;
layout(location = 8) flat out vec2 half_size_px_out;

vec2 positions[4] = vec2[](vec2(0.0f, 0.0f),
                           vec2(1.0f, 0.0f),
                           vec2(0.0f, 1.0f),
                           vec2(1.0f, 1.0f));

void main() {
  // Size of the rectangle in pixels
  vec2 size_px = rect.zw;

  // Position in range [0.0f, 1.0f]
  vec2 position = positions[gl_VertexIndex];

  // Position in range [-0.5f, 0.5f]
  vec2 center_position = position - vec2(0.5f, 0.5f);

  gl_Position = vec4(renderer_state.viewport_size_px_inv * (rect.xy + position * size_px),
                     1.0f, 1.0f);

  // Pass colors through to the fragment shader
  rgba_out = rgba;
  border_colors_rgba_out = mat4(
          border_left_rgba,
          border_top_rgba,
          border_right_rgba,
          border_bottom_rgba);

  // border_left_rgba_out = border_left_rgba;
  // border_right_rgba_out = border_right_rgba;
  // border_top_rgba_out = border_top_rgba;
  // border_bottom_rgba_out = border_bottom_rgba;

  // Output position in range [-Size/2, +Size/2]
  coord_out = center_position * size_px;

  corner_radius_out = vec4(corner_radius);
  half_size_px_out = 0.5f * size_px;
  border_size_out = vec4(border_size);
}
