
uniform sampler2D texture;

varying mediump vec4 fragment_color;
varying mediump vec2 fragment_texture;
varying mediump vec3 fragment_normal;
varying mediump vec3 fragment_light_direction;
varying mediump vec3 fragment_viewer_direction;

//
// Configuration
//
const vec4 ambient_color = vec4 (1.0, 1.0, 1.0, 1.0); // Ka
const vec4 diffuse_color = vec4 (0.327680, 0.327680, 0.327680, 1.0); // Kd
const vec4 specular_color = vec4 (0.0, 0.0, 0.0, 1.0); // Ks
const float ambient_reflection = 1.0;
const float diffuse_reflection = 1.0;
const float specular_reflection = 1.0;
const float shininess = 86.0; // Ns

void main(void)
{
  vec3 n = normalize (fragment_normal);
  vec3 light_direction = normalize (fragment_light_direction);
  vec3 viewer_direction = normalize (fragment_viewer_direction);
  vec4 ambient_illumination = ambient_reflection * ambient_color;
  vec4 diffuse_illumination = diffuse_reflection * max (0.0, dot (light_direction, n)) * diffuse_color;
  vec4 specular_illumination = specular_reflection * pow (max (0.0, dot (-reflect (light_direction, n), viewer_direction)), shininess) * specular_color;

  //gl_FragColor = fragment_color * (ambient_illumination + diffuse_illumination) + specular_illumination;
  gl_FragColor = texture2D (texture, fragment_texture) * (ambient_illumination + diffuse_illumination) + specular_illumination;
  //gl_FragColor = out_color;
  //gl_FragColor = texture2D (texture, out_texture);
}
