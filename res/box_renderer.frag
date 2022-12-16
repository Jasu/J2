#version 450

layout (location = 0) in vec2 coord;
layout (location = 1) flat in vec4 rgba;
// Left top right bottom
layout (location = 2) flat in mat4 border_colors_rgba;
layout (location = 6) flat in vec4 border_size;
layout (location = 7) flat in vec4 corner_radius;
layout (location = 8) flat in vec2 half_size_px;

layout (location = 0) out vec4 rgba_out;

void main() {
  // coord has range [-Size/2, +Size/2]. Thus, `abs`ing it gives a V-like curve:
  //
  //  \        / +
  //   \      /  |
  //    \    /   | Height is half the rectangle in pixels (for each coordinate separately)
  //     \  /    |
  //  ____\/___  +
  //
  // Subtracting the distance from the middle to a corner yields a curve where positive values
  // represent Manhattan distances to the "inner" rectangle (spanned by the midpoints of the
  // arcs making the rounded corners):
  //
  //  \        / +
  //   \      /  | Height is corner radius.
  //  --\----/-  +
  //     \  /
  //      \/

  const vec2 negative_distance = abs(coord) - half_size_px;

  // Decide whether we are rendering closer to the top or bottom than to the left or right edge.
  // This is used to determine which border color gets drawn, to add a miter when the border
  // color changes.
  // Hovever, since the borders may have different sizes, the miters do not always have neat
  // 45 degree edges. In fact, all of them may have different angles - this makes it impossible
  // to have a uniform variable for the the miter angle. The logic below pretends it is possible
  // hovever - it draws 45 degree angles, but the `nearest_border_sizes` variable is used to correct for that
  // by "stretching the canvas" according to the actual angle.
  const vec2 nearest_border_sizes = mix(border_size.xy, border_size.zw, step(0.0f, coord));
  const float vhmix = smoothstep(-2.f, 2.f, dot(negative_distance, nearest_border_sizes.yx * vec2(-1.0f, 1.0f)));
  const vec2 vertical_horizontal_mix = vec2(1.0f - vhmix, vhmix);


  // First, select the border of the edge that's closest to the current fragment,
  // to allow borders of different widths.
  // Left, Top, Right, Bottom
  const vec2 a = step(0.0f, coord) * vertical_horizontal_mix;
  const vec4 side_vector = vec4(vertical_horizontal_mix - a, a);
  const vec4 border_rgba = border_colors_rgba * side_vector;

  // Mix the border with the background.
  rgba_out = mix(rgba, border_rgba, step(0.0f, dot(negative_distance + nearest_border_sizes, vertical_horizontal_mix)));

  // Round the outer corners.
  const float xs = step(0.0f, coord.x);
  const float ys = step(0.0f, coord.y);
  const vec4 corner_vector = vec4((1.0f - xs) * (1.0f - ys), xs * (1.0f - ys), (1.0f - xs) * ys, xs * ys);
  const float nearest_corner_radius = dot(corner_radius, corner_vector);
  const vec2 edge_distance = max(negative_distance + nearest_corner_radius, 0.0f);
  rgba_out.a *= smoothstep(-9.f, -0.7f, nearest_corner_radius * nearest_corner_radius - dot(edge_distance, edge_distance));
}
