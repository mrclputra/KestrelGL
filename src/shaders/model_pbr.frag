#version 460 core
out vec4 FragColor;

// lights
struct DirectionalLight { vec3 direction; vec3 color; };
struct PointLight { vec3 position; vec3 color; float radius; };
struct SpotLight { vec3 position; vec3 direction; vec3 color; float innerCos; float outerCos; };

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

// shadows
uniform sampler2D shadowMaps[MAX_LIGHTS];
uniform mat4 lightSpaceMatrices[MAX_LIGHTS];

// irradiance test
uniform vec3 shCoefficients[9];

// attributes from the vertex shader
in vec3 vFragPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec2 vTexCoords;

const float PI = 3.14159265359;

// normal distribution function; Trowbridge-Reitz GGX
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return num / (PI * denom * denom);
}

// geometry function; Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// fresnel equation; Schlick's Approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// the pbr calculation
// I put it in a function so that I dont have to repeat code for the multiple light types
vec3 calculatePBR(vec3 L, vec3 V, vec3 N, vec3 radiance, vec3 albedo, float roughness, float metallic, vec3 F0) {
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
    vec3 specular     = numerator / denominator;
    
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

// shadows!!
float shadowCalculation(int lightIdx, vec3 normal, vec3 lightDir) {
    vec4 fragPosLightSpace = lightSpaceMatrices[lightIdx] * vec4(vFragPos, 1.0);
    vec3 projCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;

    float bias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.0005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMaps[lightIdx], 0);

    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMaps[lightIdx], projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += (projCoords.z - bias) > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    return shadow / 9.0;
}

uniform float metalnessFac;
uniform float roughnessFac;

// irradiance calculation
vec3 evaluateSHIrradiance(vec3 n)
{
    float x = n.x, y = n.y, z = n.z;

    // Lambert convolution constants
    const float A0 = 3.14159265;   // pi
    const float A1 = 2.09439510;   // 2pi/3
    const float A2 = 0.78539816;   // pi/4

    // SH basis (real, orthonormal)
    float b0 = 0.282095;
    float b1 = 0.488603 * y;
    float b2 = 0.488603 * z;
    float b3 = 0.488603 * x;

    float b4 = 1.092548 * x * y;
    float b5 = 1.092548 * y * z;
    float b6 = 0.315392 * (3.0 * z * z - 1.0);
    float b7 = 1.092548 * x * z;
    float b8 = 0.546274 * (x * x - y * y);

    vec3 L0 = shCoefficients[0] * b0;
    vec3 L1 =
          shCoefficients[1] * b1
        + shCoefficients[2] * b2
        + shCoefficients[3] * b3;
    vec3 L2 =
          shCoefficients[4] * b4
        + shCoefficients[5] * b5
        + shCoefficients[6] * b6
        + shCoefficients[7] * b7
        + shCoefficients[8] * b8;

    return max(L0 * A0 + L1 * A1 + L2 * A2, vec3(0.0));
}

// ENTRY POINT HERE
void main() {
    // fetch data
    vec3 texAlbedo      = pow(texture(albedoMap, vTexCoords).rgb, vec3(2.2));
    vec3 texNormal      = texture(normalMap, vTexCoords).rgb * 2.0 - 1.0;
    vec2 metRough       = texture(metallicRoughnessMap, vTexCoords).gb; // G=Rough, B=Met
    float texAO         = texture(aoMap, vTexCoords).r;

    // TOLOOKUP: did they change the color convention for metallic roughness maps? gltf 2.0
    // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#metallic-roughness-material

//    float roughness     = metRough.x;
//    float metallic      = metRough.y;

    float metallic      = metalnessFac;
    float roughness     = roughnessFac;

    // world space conversion
//    mat3 TBN    = mat3(normalize(vNormal), normalize(vNormal), normalize(vNormal));
    mat3 TBN    = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
    vec3 N      = normalize(TBN * texNormal);
    vec3 V      = normalize(viewPos - vFragPos);
    vec3 F0     = mix(vec3(0.04), texAlbedo, metallic);

    // lighting accm
    vec3 Lo = vec3(0.0);
   
    // Directional
    for (int i = 0; i < numDirLights; i++) {
        vec3 L = normalize(-dirLights[i].direction);
        float shadow = shadowCalculation(i, N, L);
        Lo += calculatePBR(L, V, N, dirLights[i].color, texAlbedo, roughness, metallic, F0) * (1.0 - shadow);
    }

    // Point
    for (int i = 0; i < numPointLights; i++) {
        vec3 L = normalize(pointLights[i].position - vFragPos);
        float dist = length(pointLights[i].position - vFragPos);
        float atten = clamp(1.0 - (dist / pointLights[i].radius), 0.0, 1.0);
        Lo += calculatePBR(L, V, N, pointLights[i].color * atten, texAlbedo, roughness, metallic, F0);
    }

    // now I put it all together :)
    vec3 irradiance = evaluateSHIrradiance(N);
    vec3 ambient = irradiance * texAlbedo * (1.0 - metallic) * texAO;
    vec3 color = ambient + Lo;

    // tonemapping
    color = color / (color + vec3(1.0));
    // gamma correction
    color = pow(color, vec3(1.0/2.2));

    // debug conditional thingy
    int mode = 0;
    if      (mode == 1) FragColor = vec4(texAlbedo, 1.0);
    else if (mode == 2) FragColor = vec4(N * 0.5 + 0.5, 1.0);
    else if (mode == 3) FragColor = vec4(vec3(0.0, metRough), 1.0);
    else if (mode == 4) FragColor = vec4(vec3(roughness), 1.0);
    else if (mode == 5) FragColor = vec4(vec3(metallic), 1.0);
    else if (mode == 6) FragColor = vec4(vec3(ambient), 1.0);
    else                FragColor = vec4(color, 1.0);
}