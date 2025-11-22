// PBR model:
// albedo map (diffuse)
// normal map
// metallic map
// roughness
// ambient occlusion (AO)

#version 330 core
out vec4 FragColor;

struct Light {
  vec3 position;
  vec3 color;
};

// OpenGL Context bindings
// in this case, passed from the vertex shader
in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

// TODO: switch to bindless textures?
uniform sampler2D DIFFUSE[4];
uniform sampler2D SPECULAR[4];
uniform sampler2D NORMAL[4];
uniform sampler2D HEIGHT[4];
uniform sampler2D METALLIC_ROUGHNESS[4];
uniform sampler2D METALLIC[4];
uniform sampler2D ROUGHNESS[4];
uniform sampler2D AO[4];

uniform int numDiffuse;
uniform int numSpecular;
uniform int numNormal;
uniform int numHeight;
uniform int numMetallic;
uniform int numRoughness;
uniform int numAO;
uniform int numMetallicRoughness;

uniform samplerCube skybox;

//uniform Light lights[3];
//uniform int numLights;
uniform vec3 viewPos;

const float PI = 3.14159265359;

// Fresnel-Shlick approximation
//https://en.wikipedia.org/wiki/Schlick%27s_approximation
//vec3 fresnelSchlick(float cosTheta, vec3 F0) {
//  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
//}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
} // IBL

// GGX/Trowbridge-Reitz normal distribution
float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return a2 / denom;
}

// Smith's Schlick-GGX geometry function
float GeometrySchlickGGX(float NdotV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;
  
  return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);
  
  return ggx1 * ggx2;
}

void main() {
  // compute normal
  vec3 N = normalize(Normal);
  if (numNormal > 0) {
    vec3 T = normalize(Tangent);
    vec3 B = normalize(Bitangent);
    mat3 TBN = mat3(T, B, N);
    vec3 normalMap = texture(NORMAL[0], TexCoords).rgb;
    normalMap = normalize(normalMap * 2.0 - 1.0);
    N = normalize(TBN * normalMap);
  }

  vec3 V = normalize(viewPos - FragPos); // vector to camera
  vec3 R = reflect(-V, N); // reflection vector

  // sample material properties
  vec3 albedo = (numDiffuse > 0) ? texture(DIFFUSE[0], TexCoords).rgb : vec3(0.95);
  float alpha = (numDiffuse > 0) ? texture(DIFFUSE[0], TexCoords).a : 1.0;
//  float metallic = (numMetallic > 0) ? texture(METALLIC[0], TexCoords).r : 0.0;
//  float roughness = (numRoughness > 0) ? texture(ROUGHNESS[0], TexCoords).r : 0.5;
  float metallic, roughness;
  if (numMetallicRoughness > 0) {
    // glTF format: metallic in B channel, roughness in G channel
    vec3 metallicRoughness = texture(METALLIC_ROUGHNESS[0], TexCoords).rgb;
    metallic = metallicRoughness.b;
    roughness = metallicRoughness.g;
  } else {
    // separate textures
    metallic = (numMetallic > 0) ? texture(METALLIC[0], TexCoords).r : 0.0;
    roughness = (numRoughness > 0) ? texture(ROUGHNESS[0], TexCoords).r : 0.5;
  }
  float ao = (numAO > 0) ? texture(AO[0], TexCoords).r : 1.0;

  // base reflectivity at normal incidence (F0)
  // dielectrics should have about 0.04, metals use albedo
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  // IBL lighting
  // sample the skybox for ambient lighting
  // use normal for diffuse (irradiance) and reflection vector for specular
  vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
  vec3 kD = 1.0 - kS;
  kD *= 1.0 - metallic;

  // diffuse
  vec3 irradiance = texture(skybox, N).rgb;
  vec3 diffuseIBL = irradiance * albedo;

  // specular
  const float MAX_REFLECTION_LOD = 4.0;
  vec3 prefilteredColor = textureLod(skybox, R, roughness * MAX_REFLECTION_LOD).rgb;
  vec3 specularIBL = prefilteredColor * kS; // specular + fresnel

  vec3 ambient = (kD * diffuseIBL + specularIBL) * ao;

  // fallback directional light
  vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
  vec3 H = normalize(V + lightDir);
  float NdotL = max(dot(N, lightDir), 0.0);

  vec3 color = ambient;

//  // accumulate lighting contribution
//  vec3 Lo = vec3(0.0);
//
//  for (int i = 0; i < numLights; i++) {
//    vec3 L = normalize(lights[i].position - FragPos);
//    vec3 H = normalize(V + L);
//
//    float distance = length(lights[i].position - FragPos);
////    float attenuation = 1.0 / (distance * distance); // account for light distance
//    float attenuation = 1.2;
//    vec3 radiance = lights[i].color * attenuation;
//
//    // Cook-Torrence BRDF
//    float NDF = DistributionGGX(N, H, roughness);
//    float G = GeometrySmith(N, V, L, roughness);
//    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
//
//    vec3 numerator = NDF * G * F;
//    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
//    vec3 specular = numerator / denominator;
//    
//    // energy conservation law: 
//    // diffuse + specular can't exceed 1.0
//    vec3 kS = F;              // specular contribution
//    vec3 kD = vec3(1.0) - kS; // diffuse contribution
//    kD *= 1.0 - metallic;     // metals have no diffuse
//    
//    float NdotL = max(dot(N, L), 0.0);
//    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
//  }
//
//  // ambient lighting
//  vec3 ambient = vec3(0.03) * albedo * ao;
//  vec3 color = ambient + Lo;

  // HDR tonemap
  color = color / (color + vec3(1.0));

  FragColor = vec4(color, alpha); // combined
//  FragColor = vec4((numDiffuse > 0) ? texture(DIFFUSE[0], TexCoords).rgb : vec3(0.95), alpha); // albedo
//  FragColor = vec4((numNormal > 0) ? texture(NORMAL[0], TexCoords).rgb : vec3(N * 0.5 + 0.5), alpha); // normal
//  FragColor = vec4(vec3((numMetallic > 0) ? texture(METALLIC[0], TexCoords).r : 0.0), alpha);  // metallic
//  FragColor = vec4(vec3((numRoughness > 0) ? texture(ROUGHNESS[0], TexCoords).r : 0.0), alpha);  // roughness
//  FragColor = vec4(vec3((numAO > 0) ? texture(AO[0], TexCoords).r : 0.0), alpha);  // AO
//  FragColor = vec4((numMetallicRoughness > 0) ? texture(METALLIC_ROUGHNESS[0], TexCoords).rgb: vec3(0.0), alpha);
}