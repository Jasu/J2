#version 450

layout (binding = 1) uniform sampler1D colormap;

layout (location = 0) in vec2 pos;

layout (location = 0) out vec4 color;

layout(push_constant) uniform u {
  layout(offset=16) vec4 color;
} u_color;

void main() {
  color = mix(u_color.color,
              texture(colormap, pos.x),
              smoothstep(0.4, 0.401, max(abs(pos.x - 0.5f), abs(pos.y - 0.5f))));
}
