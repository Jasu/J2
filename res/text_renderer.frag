#version 450

layout (binding = 0) uniform sampler2D glyph_atlas;

layout (location = 0) in vec2 coord;

layout (location = 0) out vec4 color;

layout(push_constant) uniform u {
  layout(offset=16) vec4 text_color;
} u_text_color;

void main() {
  color = u_text_color.text_color * texture(glyph_atlas, coord);
}
