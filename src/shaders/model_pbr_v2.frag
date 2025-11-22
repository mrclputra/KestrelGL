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
uniform samplerCube specular; // SPECULAR IBL
// TODO: BRDF LUT? need to lookup what it is

uniform vec3 viewPos; // camera position

const float PI = 3.14159265359;

void main() {
  vec3 albedo = (numAlbedo > 0) ? texture(ALBEDO[0], TexCoords).rgb : vec3(0.95);
  float alpha = (numAlbedo > 0) ? texture(ALBEDO[0], TexCoords).a : 1.0;

  vec3 metallicRoughness = (numMetallicRoughness > 0) ? texture(METALLIC_ROUGHNESS[0], TexCoords).rgb : vec3(0.0);
  vec3 normal = (numNormal > 0) ? texture(NORMAL[0], TexCoords).rgb : vec3(0.0);
  vec3 occlusion = (numOcclusion > 0) ? texture(OCCLUSION[0], TexCoords).rgb : vec3(0.0);
  vec3 emission = (numEmission > 0) ? texture(EMISSION[0], TexCoords).rgb : vec3(0.0);

  // TODO: implement PBR

  FragColor = vec4(albedo, alpha); // just albedo texture
//  FragColor = vec4(metallicRoughness, alpha); // just albedo texture
}