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

// shadows
uniform sampler2D shadowMaps[MAX_LIGHTS];
uniform mat4 lightSpaceMatrices[MAX_LIGHTS]; // this array maps to shadowMaps[]

// camera position
uniform vec3 viewPos;

// texture maps
// note that not all of these may be bound
uniform sampler2D albedoMap;
uniform bool hasAlbedoMap = false;
uniform sampler2D normalMap;
uniform bool hasNormalMap = false;
uniform sampler2D metRoughMap;
uniform bool hasMetRoughMap = false;
uniform sampler2D aoMap;
uniform bool hasAOMap = false;

// todo: external flags to determine if said textures should be used

// irradiance
uniform vec3 shCoefficients[9];

// other maps
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// imgui stuff
uniform int mode = 0;
uniform bool normalEnabled = true;
// to be deprecated?: 
uniform vec3 p_albedo;
uniform float p_metalness;
uniform float p_roughness;

// attributes from vertex shader
in vec3 vFragPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec2 vTexCoords;

const float PI = 3.14159265359;

// PBR FUNCTIONS
// -------------------------------------------------

// normal distribution function
// approximates the relative suface area of microfacets aligned to the halfway vector (H)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    
    // Trowbridge-Reitz GGX
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// geometry function
// approximates the relative surface area in which microfacets overshadow each other, causing light rays to be occluded
float GeometrySchlickGGX(float NdotV, float k) {
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k) {
    float NdotV = max(dot(N, V), 0.0); // shadows from view vector
    float NdotL = max(dot(N, L), 0.0); // shadows from light vector
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

// fresnel
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// SHADOWS
// -------------------------------------------------
float calculateShadow(int lightIdx, vec3 normal, vec3 lightDir) {
    // convert to light space
    vec4 fragPosLightSpace = lightSpaceMatrices[lightIdx] * vec4(vFragPos, 1.0);
    vec3 projCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;

    float bias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.0005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMaps[lightIdx], 0);

    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            // 3x3 PCF sample grid
            float pcfDepth = texture(shadowMaps[lightIdx], projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += (projCoords.z - bias) > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    // grab average
    return shadow / 9.0;
}

// OTHER
// -------------------------------------------------

// irradiance calculation
vec3 evaluateSHIrradiance(vec3 n)
{
    float x = n.x, y = -n.y, z = n.z;

    // Lambert convolution constants
    const float A0 = PI;
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

// MAIN
// -------------------------------------------------

void main() {
    // fetch data
    vec3 texAlbedo = hasAlbedoMap ? pow(texture(albedoMap, vTexCoords).rgb, vec3(2.2)) : p_albedo;
    vec3 rawNormal = hasNormalMap ? texture(normalMap, vTexCoords).rgb * 2.0 - 1.0 : vec3(0.0, 0.0, 1.0); // fallback to vertex normal in tangent space
    float metallic = hasMetRoughMap ? texture(metRoughMap, vTexCoords).b : p_metalness;
    float roughness = hasMetRoughMap ? texture(metRoughMap, vTexCoords).g : p_roughness;
    float texAO = hasAOMap ? texture(aoMap, vTexCoords).r : 1.0;

    // the TBN matrix is a transformation that converts tangentspace to worldspace
    // this is needed to be able to apply object transformations to the normal map
    mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
    vec3 tangentNormal;
    if (hasNormalMap && normalEnabled) {
        // sample and remap to -1,1
        tangentNormal = texture(normalMap, vTexCoords).rgb * 2.0 - 1.0;
    } else {
        // 001
        tangentNormal = vec3(0.0, 0.0, 1.0);
    }

    // transform to world space
    vec3 N = normalize(TBN * tangentNormal);
    vec3 V = normalize(viewPos - vFragPos);
    vec3 F0 = mix(vec3(0.04), texAlbedo, metallic); // todo: replace with BRDF lut

    // IBL :)

    // fresnel component
    // kS represents the amount of light that gets reflected; specular
    float NdotV = max(dot(N, V), 0.0);
    // sample the BRDF LUT, red = roughness, green = bias
    vec2 envBRDF = texture(brdfLUT, vec2(NdotV, roughness)).rg;

    // specular component
    vec3 R = reflect(-V, N); 
    const float MAX_REFLECTION_LOD = 8.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec3 specularIBL = prefilteredColor * (F0 * envBRDF.x + envBRDF.y);

    vec3 F = fresnelSchlick(NdotV, F0); 
    vec3 kD = (1.0 - F) * (1.0 - metallic);

    // diffuse component
    vec3 irradiance = evaluateSHIrradiance(N);
    vec3 diffuseIBL = (irradiance / PI) * texAlbedo;

    // combine!
    vec3 ambient = (kD * diffuseIBL + specularIBL) * texAO;

    // CUSTOM LIGHTING
    vec3 Lo = vec3(0.0); // accumulated lighting
    // Directional
    for (int i = 0; i < numDirLights; i++) {
        vec3 L = normalize(-dirLights[i].direction);
        float shadow = calculateShadow(i, N, L);

        // simple lambertian diffuse
        float diff = max(dot(N, L), 0.0);
        vec3 radiance = dirLights[i].color;

        Lo += (kD * texAlbedo / PI) * radiance * diff * (1.0 - shadow);
    }
    // Point
    for (int i = 0; i < numPointLights; i++) {
        vec3 L = normalize(pointLights[i].position - vFragPos);
        float distance = length(pointLights[i].position - vFragPos);

        // todo: calculate shadow

        float attenuation = 1.0 / (distance * distance);
        
        float diff = max(dot(N, L), 0.0);
        vec3 radiance = pointLights[i].color * attenuation;
        
        Lo += (kD * texAlbedo / PI) * radiance * diff;
    }

    // combine
    vec3 color = ambient + Lo;

    // tonemapping
    color = color / (color + vec3(1.0));
    // gamma correction
    color = pow(color, vec3(1.0/2.2));

    // output
    if      (mode == 1) FragColor = vec4(texAlbedo, 1.0);
    else if (mode == 2) FragColor = vec4(N * 0.5 + 0.5, 1.0);
    else if (mode == 3) FragColor = vec4(texture(metRoughMap, vTexCoords).rgb, 1.0);
    else if (mode == 4) FragColor = vec4(vec3(diffuseIBL), 1.0);
    else if (mode == 5) FragColor = vec4(vec3(specularIBL), 1.0);
    else if (mode == 6) FragColor = vec4(vec3(prefilteredColor), 1.0);
    else                FragColor = vec4(color, 1.0);

//    if (any(isnan(color)) || any(isinf(color))) {
//        FragColor = vec4(1.0, 0.0, 1.0, 1.0);
//    } else {
//        FragColor = vec4(color, 1.0);
//    }
}