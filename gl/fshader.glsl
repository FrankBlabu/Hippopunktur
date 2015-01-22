/*
 * fshader.glsl - Fragment shader
 *
 * Frank Blankenburg, Jan. 2015
 */

uniform sampler2D texture;
varying vec4 v_texcoord;

void main (void)
{
    gl_FragColor = texture2D (texture, v_texcoord);
}

