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
//uniform vec3 albedo = vec3(0.98);
uniform vec3 albedo = vec3(0.08);
uniform vec3 nullPink = vec3(1, 0, 1);

// shadows
uniform sampler2D shadowMaps[MAX_LIGHTS];
uniform mat4 lightSpaceMatrices[MAX_LIGHTS];

float shadowCalculation(int lightIdx, vec3 normal, vec3 lightDir) {
    // we calculate light space position on the fly
    vec4 fragPosLightSpace = lightSpaceMatrices[lightIdx] * vec4(vFragPos, 1.0);

    // perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;

    // get closest depth value from light
    float currentDepth = projCoords.z;

    // acne thing
    float bias = max(0.02 * (1.0 - dot(normal, lightDir)), 0.0005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMaps[lightIdx], 0);

    // PCF
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMaps[lightIdx], projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    return shadow / 9.0;
}


void main() {
    vec3 normal = normalize(vNormal);
    vec3 result = 0.1 * albedo; // ambient

    // directional lights
    for (int i = 0; i < numDirLights; i++) {
        vec3 L = normalize(-dirLights[i].direction);
        float NdotL = max(dot(normal, L), 0.0);

        float shadow = shadowCalculation(i, normal, L);

        result += (1.0 - shadow) * NdotL * albedo * dirLights[i].color;
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