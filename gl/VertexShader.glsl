attribute highp vec4 in_vertex;
//attribute mediump vec3 in_normal;
attribute mediump vec3 in_color;
uniform mediump mat4 in_matrix;
varying mediump vec4 color;

void main (void)
{
  //vec4 a = in_matrix * vec4 (0.0, 0.8, -1.0, 1.0);
  //vec3 toLight = normalize (vec3 (a.x, a.y, a.z));
  //float angle = max (dot (in_normal, toLight), 0.0);
  //vec3 col = vec3 (0.8, 0.8, 0.8);
  //color = vec4 (col * 0.2 + col * 0.8 * angle, 1.0);
  //color = clamp (color, 0.0, 1.0);
  color = vec4 (in_color.x, in_color.y, in_color.z, 1.0);
  gl_Position = in_matrix * in_vertex;
}
