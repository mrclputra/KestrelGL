#version 460 core
out vec4 FragColor;

// lights
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

// camera position
uniform vec3 viewPos;

// texture maps
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicRoughnessMap;
uniform sampler2D aoMap;

// attributes from the vertex shader
in vec3 vFragPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec2 vTexCoords;

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

    // shadow check
//    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
//    if(projCoords.z > 1.0) shadow = 0.0;
//
//    return shadow;

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
	vec3 albedo = texture(albedoMap, vTexCoords).rgb;
	vec3 normal = texture(normalMap, vTexCoords).rgb;
	vec3 metallicRoughness = texture(metallicRoughnessMap, vTexCoords).rgb;
	normal = normal * 2.0 - 1.0; // map to (-1, 1)

	vec3 viewDir = normalize(viewPos - vFragPos);

	// TBN matrix
	vec3 T = normalize(vTangent);
	vec3 B = normalize(vBitangent);
	vec3 N = normalize(vNormal);
	mat3 TBN = mat3(T, B, N);

	normal = normalize(TBN * normal);

	// phong lighting
    vec3 ambient = 0.1 * albedo;
    vec3 result = ambient;

    // directional lights
    for (int i = 0; i < numDirLights; i++) {
        // diffuse
        vec3 L = normalize(-dirLights[i].direction);
        float NdotL = max(dot(normal, L), 0.0);
        vec3 diffuse = NdotL * albedo * dirLights[i].color;

        // specular
        vec3 halfwayDir = normalize(L + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
        vec3 specular = spec * 0.3 * dirLights[i].color;

        // shadows
        float shadow = shadowCalculation(i, N, L);

        result += (1.0 - shadow) * (diffuse + specular);
    }

    // point lights
    for (int i = 0; i < numPointLights; i++) {
        // diffuse
        vec3 toLight = pointLights[i].position - vFragPos;
        float distance = length(toLight);
        vec3 L = toLight / distance;

        float att = clamp(1.0 - distance / pointLights[i].radius, 0.0, 1.0);
        float NdotL = max(dot(normal, L), 0.0);
        vec3 diffuse = NdotL * albedo * pointLights[i].color * att;

        // specular
        vec3 halfwayDir = normalize(L + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
        vec3 specular = spec * 0.3 * pointLights[i].color * att;

        result += diffuse + specular;
    }

    // to display the models normals in rgb, we need to remap to 0-1
    //	FragColor = vec4(normal * 0.5 + 0.5, 1.0);

    //	FragColor = vec4(albedo, 1.0);
    //	FragColor = vec4(metallicRoughness, 1.0);
    FragColor = vec4(result, 1.0);
}