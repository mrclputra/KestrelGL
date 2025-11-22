#version 330 core
out vec4 FragColor;

// OpenGL context bindings
in vec2 TexCoords;
// in vec2 TexCoords2; // GLTF: special cases?
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

// TODO: switch to bindless textures later
// GLTF 2.0 convention
uniform sampler2D ALBEDO[8];
uniform sampler2D METALLIC_ROUGHNESS[8];
uniform sampler2D NORMAL[8];
uniform sampler2D OCCLUSION[8];
uniform sampler2D EMISSION[8];

// other texture types here (other formats; i.e specular model)
// TODO: handle edge cases later
// TODO: or manually turn specular models to PBR compatible
// specular
// diffuse (should probably reuse albedo)

// this setup is based on 
// https://www.khronos.org/files/gltf20-reference-guide.pdf
uniform int numAlbedo;
uniform int numMetallicRoughness;
uniform int numNormal;
uniform int numOcclusion;
uniform int numEmission;

uniform samplerCube skybox;
uniform samplerCube irradiance; // DIFFUSE IBL
//uniform samplerCube specular; // SPECULAR IBL // TOIMPLEMENT LATER
// TODO: BRDF LUT? need to lookup what it is

uniform vec3 viewPos; // camera position

const float PI = 3.14159265359;

// ACES approximation; same as skybox
// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 toneMapACES(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
  vec3 albedo = (numAlbedo > 0) ? texture(ALBEDO[0], TexCoords).rgb : vec3(0.95);
  float alpha = (numAlbedo > 0) ? texture(ALBEDO[0], TexCoords).a : 1.0;

  // GLTF format
  // B channel is metallic
  // G channel is roughness
  float metallic = 0.0;
  float roughness = 1.0;
  if (numMetallicRoughness > 0) {
    vec3 mr = texture(METALLIC_ROUGHNESS[0], TexCoords).rgb;
    metallic = mr.b;
    roughness = mr.g;
  }

  float ao = (numOcclusion > 0) ? texture(OCCLUSION[0], TexCoords).r : 1.0;
  vec3 emission = (numEmission > 0) ? texture(EMISSION[0], TexCoords).rgb : vec3(0.0);

  // compute normal
  // map tangent space
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
  vec3 R = reflect(-V, N); // reflection vector for specular

  // base reflectivity at normal incidence (F0)
  // dielectrics use 0.04, metals use albedo color
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  // IBL lighting with fresnel
  vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
  vec3 kD = 1.0 - kS;
  kD *= 1.0 - metallic; // metals have no diffuse

  // diffuse IBL
  vec3 irradianceColor = texture(irradiance, N).rgb;
  vec3 diffuseIBL = irradianceColor * albedo;

  // specular IBL
  // I'm just using skybox for now
  const float MAX_REFLECTION_LOD = 4.0;
  vec3 prefilteredColor = textureLod(skybox, R, roughness * MAX_REFLECTION_LOD).rgb;
  vec3 specularIBL = prefilteredColor * kS;

  // combine ambient occlusion
  vec3 ambient = (kD * diffuseIBL + specularIBL) * ao;
  
  // add emission
  vec3 color = ambient + emission;

  // exposure based on scene luminance
  float avgLuminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
  float exposure = 1.0 / (1.0 + avgLuminance * 2.0);
  color *= exposure;

  // tone mapping
  color = toneMapACES(color);
    
  // sRGB gamma correction
  // should work for most HDRIs
  color = pow(color, vec3(1.0/2.2));

  FragColor = vec4(color, alpha);

//  vec3 viewDir = normalize(FragPos - viewPos);
//  vec3 reflectDir = reflect(viewDir, normalize(Normal));
//  vec3 skyboxColor = texture(skybox, reflectDir).rgb;
//  FragColor = vec4(skyboxColor, 1.0);

//  FragColor = vec4(albedo, alpha); // just albedo texture
//  FragColor = vec4(metallicRoughness, alpha); // other textures
}