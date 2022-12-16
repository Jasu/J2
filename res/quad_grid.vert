#version 450

layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 color;
layout(push_constant) uniform u {
  float x;
  float y;
} push_consts;

layout(location = 0) out flat vec3 frag_color;

layout(location = 1) out vec2 coord;

void main() {
  gl_Position = vec4(push_consts.x * min(pos.x, pos.y), max(pos.x, pos.y), 0, push_consts.y);
  coord = vec2(gl_Position.x * 50, gl_Position.y * 50);
  frag_color = color;
}
