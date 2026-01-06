#version 460 core

out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;
uniform float roughness;

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

// GGX importance sampling
vec3 ImportanceSampleGGX(vec2 xi, vec3 N, float roughness)
{
    float a = roughness * roughness;
	
    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a*a - 1.0) * xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // spherical to cartesian
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // tangentspace to worldspace
    vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent     = normalize(cross(up, N));
    vec3 bitangent   = cross(N, tangent);
	
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

void main() {
    vec3 N = normalize(vec3(WorldPos.x, -WorldPos.y, WorldPos.z));
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 2048u;
    float totalWeight = 0.0;
    vec3 color = vec3(0.0);

    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H  = ImportanceSampleGGX(xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float nDotL = max(dot(N, L), 0.0);
        if(nDotL > 0.0)
        {
            vec3 sampleColor = texture(environmentMap, L).rgb;
            sampleColor = clamp(sampleColor, 0.0, 1000.0); // CLAMP

            // compression
            // https://graphicrants.blogspot.com/2013/12/tone-mapping.html?m=1
            float luma = dot(sampleColor, vec3(0.2126, 0.7152, 0.0722));
            float weight = 1.0 / (1.0 + luma);

            color       += sampleColor * weight * nDotL;
            totalWeight += weight * nDotL;
        }
    }
    color = color / totalWeight;

//    FragColor = vec4(color, 1.0);

    if (any(isnan(color)) || any(isinf(color))) {
        FragColor = vec4(1.0, 0.0, 1.0, 1.0);
    } else {
        FragColor = vec4(color, 1.0);
    }
}