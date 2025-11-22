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

void main() {
  vec3 albedo = (numAlbedo > 0) ? texture(ALBEDO[0], TexCoords).rgb : vec3(0.95);
  float alpha = (numAlbedo > 0) ? texture(ALBEDO[0], TexCoords).a : 1.0;

  vec3 metallicRoughness = (numMetallicRoughness > 0) ? texture(METALLIC_ROUGHNESS[0], TexCoords).rgb : vec3(0.0);
  vec3 normal = (numNormal > 0) ? texture(NORMAL[0], TexCoords).rgb : vec3(0.0);
  vec3 occlusion = (numOcclusion > 0) ? texture(OCCLUSION[0], TexCoords).rgb : vec3(0.0);
  vec3 emission = (numEmission > 0) ? texture(EMISSION[0], TexCoords).rgb : vec3(0.0);

  // TODO: implement PBR

//  FragColor = vec4(albedo, alpha); // just albedo texture
//  FragColor = vec4(metallicRoughness, alpha); // other textures

  vec3 irradianceColor = texture(irradiance, normalize(Normal)).rgb;
  vec3 color = albedo * irradianceColor;

  float avgLuminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
  float exposure = 1.0 / (1.0 + avgLuminance * 2.0);
  color *= exposure;

  // tone mapping
  color = toneMapACES(color);
    
  // sRGB gamma correction
  // should work for most HDRIs
  color = pow(color, vec3(1.0/2.2));

  FragColor = vec4(irradianceColor, 1.0);

//  vec3 viewDir = normalize(FragPos - viewPos);
//  vec3 reflectDir = reflect(viewDir, normalize(Normal));
//
//  vec3 skyboxColor = texture(skybox, reflectDir).rgb;
//  FragColor = vec4(skyboxColor, 1.0);
}