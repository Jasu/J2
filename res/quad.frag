#version 450

layout (binding = 0) uniform sampler2D sm;

layout (location = 0) in flat vec3 frag_color;
layout (location = 1) in vec2 coord;

layout (location = 0) out vec4 color;

void main() {
  color = vec4(frag_color, 1.0) + texture(sm, coord);
}
