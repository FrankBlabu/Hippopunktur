varying mediump vec4 out_color;
varying mediump vec2 out_texture;
uniform sampler2D texture;

void main(void)
{
  //gl_FragColor = out_color;
  gl_FragColor = texture2D (texture, out_texture);
}
