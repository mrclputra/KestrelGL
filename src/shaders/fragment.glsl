#version 330 core
out vec4 FragColor;

struct Light {
  vec3 position;
  vec3 color;
};

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

// remember convention:
uniform sampler2D texture_diffuse1;
uniform Light lights[3];
uniform int numLights;
uniform vec3 viewPos; // camera location, for Phong model
uniform bool hasDiffuseMap; 

void main() {
  vec3 result = vec3(0.0);
  vec3 norm = normalize(Normal);
  
  vec3 baseColor = hasDiffuseMap ? texture(texture_diffuse1, TexCoords).rgb : vec3(0.95);
  
  // lighting
  for (int i = 0 ; i < numLights; i++) {
    // ambient
    vec3 ambient = 0.07 * lights[i].color;

    // diffuse
    vec3 lightDir = normalize(lights[i].position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = 0.42 * diff * lights[i].color;
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = 0.17 * spec * lights[i].color;

    // attentuation
    float distance = length(lights[i].position - FragPos);
    float attenuation = 1.0 / (1.0 + 0.07 * distance + 0.032 * (distance * distance));

//    ambient *= attenuation;
//    diffuse *= attenuation;
//    specular *= attenuation;

    result += (ambient + diffuse + specular) * baseColor;
  }
  
//  FragColor = vec4(baseColor, 1.0);
  FragColor = vec4(result, 1.0);
}