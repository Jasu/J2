#version 450

// Uniforms
layout(binding = 0) uniform renderer_state_t {
  vec2 viewport_size_px_inv;
  vec2 viewport_size_px;
} renderer_state;

// Push constants
layout(push_constant) uniform u {
  layout(offset=0) vec4 position;
} u_position;

// Outputs
layout(location = 0) out vec2 pos;

vec2 positions[4] = vec2[](vec2(0.0f, 0.0f),
                           vec2(1.0f, 0.0f),
                           vec2(0.0f, 1.0f),
                           vec2(1.0f, 1.0f));

void main() {
  const vec2 position = positions[gl_VertexIndex];
  gl_Position = vec4(renderer_state.viewport_size_px_inv * u_position.position.xy +
                     renderer_state.viewport_size_px_inv * position * u_position.position.zw,
                     0.5f, 1.0f);
  pos = position;
}
