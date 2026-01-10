#version 460 core
out vec4 FragColor;

// lights
struct DirectionalLight { vec3 direction; vec3 color; };

uniform DirectionalLight dirLights[8];
uniform int numDirLights;

// shadows
//uniform sampler2DArray shadowMaps;
//uniform mat4 lightSpaceMatrices[8]; // this array maps to shadowMaps

// camera uniforms
uniform vec3 viewPos;

// texture maps
uniform sampler2D albedoMap;
uniform bool hasAlbedoMap = false;
uniform sampler2D normalMap;
uniform bool hasNormalMap = false;
uniform sampler2D metRoughMap;
uniform bool hasMetRoughMap = false;
uniform sampler2D aoMap;
uniform bool hasAOMap = false;

// imgui stuff
uniform vec4 p_albedo; // rgb

// attributes from vertex shader
in vec3 vFragPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec2 vTexCoords;

// MAIN
// -------------------------------------------------
void main() {
    // fetch data
    vec3 albedo = vec3(1.0);
    float alpha = 1.0;
    if (hasAlbedoMap) {
        // use the texture map
        albedo = pow(texture(albedoMap, vTexCoords).rgb, vec3(2.2));
        alpha = texture(albedoMap, vTexCoords).a;
    } else {
        // use p_albedo
        albedo = p_albedo.rgb;
        alpha = p_albedo.a;
    }

    // the TBN matrix is a transformation that converts tangentspace to worldspace
    // this is needed to be able to apply object transformations to the normal map
    mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
    vec3 tangentNormal;
    if (hasNormalMap) {
        // sample and remap to -1,1
        tangentNormal = texture(normalMap, vTexCoords).rgb * 2.0 - 1.0;
    } else {
        // 001
        tangentNormal = vec3(0.0, 0.0, 1.0);
    }

//    // transform to world space
//    vec3 N = normalize(TBN * tangentNormal);
//    vec3 V = normalize(viewPos - vFragPos);
//
//    // diffuse component
//    vec3 Lo = vec3(0.0);
//    for (int i = 0; i < numDirLights; i++) {
//        vec3 L = normalize(-dirLights[i].direction);
//        float diff = max(dot(N, L), 0.0);
//        vec3 radiance = dirLights[i].color;
//
//        Lo += (albedo) * radiance * diff;
//    }

    // combine
//    vec3 ambient = vec3(0.08) * albedo;
//    vec3 color = ambient + Lo;

    FragColor = vec4(albedo, alpha);
}