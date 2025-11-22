#version 330 core
out vec4 FragColor;

// OpenGL context bindings
in vec2 TexCoords;
// in vec2 TexCoords2; // GLTF: special cases?
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

// light positions; temporary?
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

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

// fresnel effect approximation
// increasing reflectivity as surface angle from view gets steeper
vec3 fresnelSchlick(float HdotV, vec3 baseReflectivity) {
  // base reflectivity is in range 0 to 1
  // returns range of base reflectivity to 1
  // increases as HdotV increases
  return baseReflectivity + (1.0 - baseReflectivity) * pow(1.0 - HdotV, 5.0);
}

// trowbridge-reitz GGX normal distribution
// how many microfacets are aligned such that reflections bounce off at an angle towards viewer
float distributionGGX(float NdotH, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
  denom = PI * denom * denom;
  return a2 / max(denom, 0.000001); // prevent divide by 0
}

// or Smith's method
// self shadowing function
float geometrySmith(float NdotV, float NdotL, float roughness) {
  float r = roughness + 1.0;
  float k  = (r * r) / 8.0;
  float ggx1 = NdotV / (NdotV * (1.0 - k) + k); // Schlick GGX
  float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
  return ggx1 * ggx2;
}

void main() {
  vec3 albedo = (numAlbedo > 0) ? texture(ALBEDO[0], TexCoords).rgb : vec3(0.95);
  float alpha = (numAlbedo > 0) ? texture(ALBEDO[0], TexCoords).a : 1.0;

  // GLTF format
  // B channel is metallic
  // G channel is roughness
  float metallic = 0.0; // default
  float roughness = 0.5; // default
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

  // calculate reflectance at normal incidence; if dia-electrics (i.e. plastic) use baseReflectivity
  // if 0.04 and if its a metal, use the albed color as baseReflectivity
  vec3 baseReflectivity = mix(vec3(0.04), albedo, metallic);

  // reflectance equation
  vec3 Lo = vec3(0.0);
  for (int i = 0; i < 3; ++i) {
    // calculate per-light radiance
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

    float D = distributionGGX(NdotH, roughness); // larger the more microfacets aligned to H
    float G = geometrySmith(NdotV, NdotL, roughness); // smaller the more microfacets shadowed by other microfacets
    vec3 F = fresnelSchlick(HdotV, baseReflectivity);

    vec3 specular = D * G * F;
    specular /= 4.0 * NdotV * NdotL;

    // energy conservation
    // diffuse and specular cannot be above 1.0, unless surface emits lights
    // relationship between diffuse component (kD) should equal to 1.0 - kS
    vec3 kD = vec3(1.0) - F; // F = kS

    // multiply kD by the inverse metalness such that only non-metals have diffuse lighting
    // or a linear blend if partially metal
    // as pure metals have no diffuse light
    kD *= 1.0 - metallic;

    // note that
    // - angle of light affects specular as well as diffuse
    // - albedo is mixed with diffuse, not specular
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
  }

  // ambient lighting
  // TODO: replace with IBL
//  vec3 ambient = vec3(0.03) * albedo;
  vec3 irradianceColor = texture(irradiance, N).rgb;
  vec3 diffuseIBL      = irradianceColor * albedo * (1.0 - metallic);
  vec3 ambient         = diffuseIBL;
  vec3 color = (ambient + emission + Lo) * ao;

  // exposure based on scene luminance
  float avgLuminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
  float exposure = 1.0 / (1.0 + avgLuminance * 2.0);
  color *= exposure;

  // HDR tone mapping
  color = toneMapACES(color);
    
  // sRGB gamma correction
  // should work for most HDRIs
  color = pow(color, vec3(1.0/2.2));

  FragColor = vec4(color, alpha);
//  FragColor = vec4(irradianceColor, alpha);

//  vec3 viewDir = normalize(FragPos - viewPos);
//  vec3 reflectDir = reflect(viewDir, normalize(Normal));
//  vec3 skyboxColor = texture(skybox, reflectDir).rgb;
//  FragColor = vec4(skyboxColor, 1.0);

//  FragColor = vec4(albedo, alpha); // just albedo texture
//  FragColor = vec4(texture(METALLIC_ROUGHNESS[0], TexCoords).rgb, alpha); // metallic roughness
}