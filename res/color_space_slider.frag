#version 450

layout (binding = 1) uniform sampler1D colormap;
layout (binding = 2) uniform sampler1D colormap2;

layout (location = 0) in vec2 pos;

layout (location = 0) out vec4 color;

layout(push_constant) uniform u {
  layout(offset=16) float selected_pos;
} u_selected_pos;

void main() {
  color = mix(vec4(1.0f),
              mix(texture(colormap, pos.x),
                  texture(colormap2, pos.x),
                  smoothstep(0.499f, 0.501f, pos.y)),
              smoothstep(0.002f, 0.003f, abs(u_selected_pos.selected_pos - pos.x)));
}
