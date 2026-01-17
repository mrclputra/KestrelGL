#version 460 core
out vec4 FragColor;

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

    FragColor = vec4(albedo, alpha);
}