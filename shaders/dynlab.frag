#version 330 

in vec3 vertex_normal;
out vec4 output_color;

uniform vec3 color;

void main() {

  vec4 diffuse_intensity = vec4(0.45f, 0.45f, 0.45f, 1.0f);
  vec4 ambient_intensity = vec4(0.25f, 0.25f, 0.25f, 1.0f);
  vec4 light_direction = vec4(-0.5f, -1.0f, 1.0f, 1.0f);
  vec4 diffuse_color = vec4(color, 1.0f);
  vec4 specular_color = vec4(0.3f, 0.3f, 0.3f, 1.0f);

  /* Compute cosine of angle of incidence */
  float cos_incidence = dot(vertex_normal, light_direction.xyz);
  cos_incidence = clamp(cos_incidence, 0, 1);

  /* Compute Blinn term */
  vec3 view_direction = vec3(0.0f, 0.0f, 1.0f);
  vec3 half_angle = normalize(light_direction.xyz + view_direction);
  float blinn_term = dot(vertex_normal, half_angle);
  blinn_term = clamp(blinn_term, 0.0f, 1.0f);
  blinn_term = pow(blinn_term, 2.0f); 
    
  /* Compute final color */
  output_color = ambient_intensity * diffuse_color + 
    diffuse_intensity * diffuse_color * cos_incidence +
    diffuse_intensity * specular_color * blinn_term;
}
