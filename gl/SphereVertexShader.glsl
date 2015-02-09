attribute highp vec4 in_vertex;
attribute mediump vec3 in_normal;

uniform mediump mat4 in_mvp;
uniform mediump vec3 in_color;

varying mediump vec4 fragment_color;


void main (void)
{
  fragment_color = vec4 (in_color.x, in_color.y, in_color.z, 1.0);
  gl_Position = in_mvp * in_vertex;
}
