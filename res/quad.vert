#version 450

layout(location = 0) in ivec2 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 inst_color;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 coord;

layout(binding = 0) uniform ubo {
  float width_factor;
  float height_factor;
};

void main() {
  gl_Position = vec4(pos.x * width_factor, pos.y * height_factor, 0.5, 1.0);
  fragColor = inst_color * color;
  coord = vec2(pos.x * width_factor * 30, pos.y * height_factor * 30);
}
