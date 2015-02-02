attribute highp vec4 in_vertex;
attribute mediump vec3 in_normal;
attribute mediump vec3 in_color;
attribute mediump vec2 in_texture;

uniform mediump mat4 in_mvp_matrix;
uniform mediump mat4 in_mv_matrix;
uniform mediump mat3 in_n_matrix;
uniform mediump vec3 in_light_position;

uniform mediump vec3 in_ambient_color;
uniform mediump vec3 in_diffuse_color;
uniform mediump vec3 in_specular_color;
uniform mediump float in_specular_exponent;

varying mediump vec4 fragment_color;
varying mediump vec2 fragment_texture;
varying mediump vec3 fragment_normal;
varying mediump vec3 fragment_light_direction;
varying mediump vec3 fragment_viewer_direction;
varying mediump vec4 fragment_ambient_color;
varying mediump vec4 fragment_diffuse_color;
varying mediump vec4 fragment_specular_color;
varying mediump float fragment_specular_exponent;


void main (void)
{
  vec4 eye_vertex = in_mv_matrix * in_vertex;
  eye_vertex /= eye_vertex.w;
  fragment_normal = in_n_matrix * in_normal;
  fragment_light_direction = in_light_position - eye_vertex.xyz;
  fragment_viewer_direction = -eye_vertex.xyz;
  fragment_texture = in_texture;

  fragment_ambient_color = vec4 (in_ambient_color.xyz, 1);
  fragment_diffuse_color = vec4 (in_diffuse_color.xyz, 1);
  fragment_specular_color = vec4 (in_specular_color.xyz, 1);
  fragment_specular_exponent = in_specular_exponent;

  //fragment_color = vec4 (in_color.x, in_color.y, in_color.z, 1.0);
  fragment_color = vec4 (1.0, 1.0, 1.0, 1.0);
  gl_Position = in_mvp_matrix * in_vertex;
}
