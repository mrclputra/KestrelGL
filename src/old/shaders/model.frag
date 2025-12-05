#version 330 core
out vec4 FragColor;

// vertex shader outputs
in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

// scene lighting
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

// PBR textures (GLTF 2.0 convention)
// https://www.khronos.org/files/gltf20-reference-guide.pdf
uniform sampler2D ALBEDO[8];
uniform sampler2D METALLIC_ROUGHNESS[8];
uniform sampler2D NORMAL[8];
uniform sampler2D OCCLUSION[8];
uniform sampler2D EMISSION[8];

uniform int numAlbedo;
uniform int numMetallicRoughness;
uniform int numNormal;
uniform int numOcclusion;
uniform int numEmission;

// IBL
uniform samplerCube skybox; // not needed after specularIBL
uniform samplerCube irradiance;

uniform vec3 viewPos;

const float PI = 3.14159265359;

// ACES tone mapping
// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 toneMapACES(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// fresnel effect approximation
// reflectivity increases as view angle becomes more grazing
vec3 fresnelSchlick(float HdotV, vec3 baseReflectivity) {
  return baseReflectivity + (1.0 - baseReflectivity) * pow(1.0 - HdotV, 5.0);
}

// Trowbridge-Reitz GGX normal distribution
// describes distribution of microfacet orientations
float distributionGGX(float NdotH, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
  denom = PI * denom * denom;
  return a2 / max(denom, 0.000001);
}

// Smith's method with Schlick-GGX
// accounts for geometry obstruction and shadowing
float geometrySmith(float NdotV, float NdotL, float roughness) {
  float r = roughness + 1.0;
  float k = (r * r) / 8.0;
  float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
  float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
  return ggx1 * ggx2;
}

void main() {
  // sample material properties
  vec3 albedo = (numAlbedo > 0) ? texture(ALBEDO[0], TexCoords).rgb : vec3(.7);
  float alpha = (numAlbedo > 0) ? texture(ALBEDO[0], TexCoords).a : 1.0;

  // GLTF metallic-roughness: B=metallic, G=roughness
  float metallic = 0.0;
  float roughness = 0.5;
  if (numMetallicRoughness > 0) {
    vec3 mr = texture(METALLIC_ROUGHNESS[0], TexCoords).rgb;
    metallic = mr.b;
    roughness = mr.g;
  }

  float ao = (numOcclusion > 0) ? texture(OCCLUSION[0], TexCoords).r : 1.0;
  vec3 emission = (numEmission > 0) ? texture(EMISSION[0], TexCoords).rgb : vec3(0.0);

  // calculate normal (with optional normal mapping)
  vec3 N = normalize(Normal);
  if (numNormal > 0) {
    vec3 T = normalize(Tangent);
    vec3 B = normalize(Bitangent);
    mat3 TBN = mat3(T, B, N);
    vec3 normalMap = texture(NORMAL[0], TexCoords).rgb;
    normalMap = normalize(normalMap * 2.0 - 1.0);
    N = normalize(TBN * normalMap);
  }

  vec3 V = normalize(viewPos - FragPos);
  vec3 R = reflect(-V, N);

  // base reflectivity at normal incidence
  // dielectrics: ~0.04, metals: use albedo color
  vec3 baseReflectivity = mix(vec3(0.04), albedo, metallic);

  // DIRECT LIGHTING
  vec3 Lo = vec3(0.0);
  for (int i = 0; i < 1; ++i) {
    vec3 L = normalize(lightPositions[i] - FragPos);
    vec3 H = normalize(V + L);
    float distance = length(lightPositions[i] - FragPos);
    float attenuation = 1.0;
    vec3 radiance = lightColors[i] * attenuation;

    // Cook-Torrance BRDF
    float NdotV = max(dot(N, V), 0.0000001);
    float NdotL = max(dot(N, L), 0.0000001);
    float HdotV = max(dot(H, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    float D = distributionGGX(NdotH, roughness);
    float G = geometrySmith(NdotV, NdotL, roughness);
    vec3 F = fresnelSchlick(HdotV, baseReflectivity);

    vec3 specular = D * G * F / (4.0 * NdotV * NdotL);

    // energy conservation: kD + kS = 1
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic); // metals have no diffuse

    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
  }

  // IBL
  vec3 irradianceColor = texture(irradiance, N).rgb;
  vec3 kD = (1.0 - baseReflectivity) * (1.0 - metallic);
  vec3 diffuseIBL = kD * irradianceColor * albedo;
  vec3 ambient = diffuseIBL * ao;

  // FINAL COLOR
  vec3 color = ambient + Lo + emission;

//  color *= exposure; 

  // HDR tone mapping
  color = toneMapACES(color);

  // gamma correction 
  // for sRGB display
  color = pow(color, vec3(1.0/2.2));

  FragColor = vec4(color, alpha);
//  FragColor = vec4(diffuseIBL, 1.0);
//  FragColor = vec4(irradianceColor, 1.0);

//  FragColor = vec4(albedo, 1.0);
//  FragColor = vec4(vec3(roughness), 1.0);
//  FragColor = vec4(N * 0.5 + 0.5, 1.0);  // remap -1..1 to 0..
  
  // cool stuff
//  FragColor = vec4(normalize(viewPos - FragPos) * 0.5 + 0.5, 1.0); // space-view vector
}