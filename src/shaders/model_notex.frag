#version 460 core
out vec4 FragColor;

in vec3 vFragPos;
in vec3 vNormal;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};
struct PointLight {
    vec3 position;
    vec3 color;
    float radius;
};
struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float innerCos;
    float outerCos;
};

#define MAX_LIGHTS 8
uniform DirectionalLight dirLights[MAX_LIGHTS];
uniform int numDirLights;

uniform PointLight pointLights[MAX_LIGHTS];
uniform int numPointLights;

uniform SpotLight spotLights[MAX_LIGHTS];
uniform int numSpotLights;

uniform vec3 viewPos;
uniform vec3 albedo = vec3(0.98, 0.98, 0.98);
uniform vec3 nullPink = vec3(1, 0, 1);

void main() {
    vec3 normal = normalize(vNormal);
    vec3 result = 0.1 * albedo; // ambient

    // directional lights
    for (int i = 0; i < numDirLights; i++) {
        vec3 L = normalize(-dirLights[i].direction);
        float NdotL = max(dot(normal, L), 0.0);
        result += NdotL * albedo * dirLights[i].color;
    }

    // point lights
    for (int i = 0; i < numPointLights; i++) {
        vec3 toLight = pointLights[i].position - vFragPos;
        float dist = length(toLight);
        vec3 L = toLight / dist;
        float att = clamp(1.0 - dist / pointLights[i].radius, 0.0, 1.0);
        float NdotL = max(dot(normal, L), 0.0);
        result += NdotL * albedo * pointLights[i].color * att;
    }

    FragColor = vec4(result, 1.0);
}