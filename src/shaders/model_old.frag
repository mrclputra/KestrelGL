// PHONG MODEL
// NOT USED, NOT IBL

#version 330 core
out vec4 FragColor;

struct Light {
    vec3 position;
    vec3 color;
};

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

// GLSL doesnt care if these are unbound if they are not used
uniform sampler2D DIFFUSE[8];
uniform sampler2D NORMAL[8];
uniform sampler2D SPECULAR[8];
uniform sampler2D HEIGHT[8];

uniform int numDiffuse;
uniform int numNormal;
uniform int numSpecular;
uniform int numHeight;

uniform Light lights[3];
uniform int numLights;
uniform vec3 viewPos;

void main() {
  // compute normal
  vec3 N = normalize(Normal);
  vec3 T = normalize(Tangent);
  vec3 B = normalize(Bitangent);
  vec3 normalMap = vec3(0.0);

  if (numNormal > 0) {
    mat3 TBN = mat3(T, B, N);
    normalMap = texture(NORMAL[0], TexCoords).rgb;
    normalMap = normalize(normalMap * 2.0 - 1.0);
    N = TBN * normalMap;
  }

  vec3 V = normalize(viewPos - FragPos); // view direction
  vec3 baseColor = (numDiffuse > 0) ? texture(DIFFUSE[0], TexCoords).rgb : vec3(0.95);
  float alpha = (numDiffuse > 0) ? texture(DIFFUSE[0], TexCoords).a : 1.0;
//  if (alpha < 0.01) discard;

  // accumulate components over all lights
  vec3 ambient = vec3(0.0);
  vec3 diffuse = vec3(0.0);
  vec3 specular = vec3(0.0);
  vec3 finalColor = vec3(0.0);

  for (int i = 0; i < numLights; i++) {
    vec3 L = normalize(lights[i].position - FragPos);

    // ambient
    ambient += 0.02 * lights[i].color;

    // diffuse
    float diff = max(dot(N, L), 0.0);
    diffuse += 0.62 * diff * lights[i].color;

    // specular
    vec3 R = reflect(-L, N);
    vec3 specMap = (numSpecular > 0) ? texture(SPECULAR[0], TexCoords).rgb : vec3(0.2);
    float gloss = (numSpecular > 0) ? texture(SPECULAR[0], TexCoords).a : 0.5;
    float spec = pow(max(dot(V, R), 0.0), gloss * 128.0);
    specular += specMap * spec * lights[i].color * 2.0;
  }

  finalColor = (ambient + diffuse + specular) * baseColor;

  FragColor = vec4(finalColor, alpha);      // full phong
//  FragColor = vec4(diffuse, 1.0);         // diffuse only
//  FragColor = vec4(specular, 1.0);        // specular only
//  FragColor = vec4(N * 0.5 + 0.5, 1.0);   // normal
//  FragColor = vec4(ambient, 1.0);         // ambient only

//  vec3 specMap = (numSpecular > 0) ? texture(SPECULAR[0], TexCoords).rgb : vec3(0.2);
//  FragColor = vec4(specMap, 1.0);        // specular map only
}