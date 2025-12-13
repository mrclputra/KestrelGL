#version 460 core
out vec4 FragColor;

// from the vertex shader
in vec3 vFragPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec2 vTexCoords;

// TODO: modify main loop to handle multiple lights
uniform vec3 lightDir;

// light struct
struct Light {
	vec3 position;
	vec3 direction;
	vec3 color;
	int type; // 0-directional, 1-point, 2-spot
};

#define MAX_LIGHTS 8
uniform Light lights[MAX_LIGHTS];
uniform int numLights; // active light count

// TODO: add more follow gltf convention, also wondering if I should make these arrays
// textures
uniform sampler2D albedoMap;
uniform sampler2D normalMap;

void main() {
	vec3 albedo = texture(albedoMap, vTexCoords).rgb;
	vec3 normal = texture(normalMap, vTexCoords).rgb;
	normal = normal * 2.0 - 1.0; // to (-1, 1)

	// TBN matrix
	vec3 T = normalize(vTangent);
	vec3 B = normalize(vBitangent);
	vec3 N = normalize(vNormal);
	mat3 TBN = mat3(T, B, N);

	normal = normalize(TBN * normal); // order matters

	// phong-thing lighting below

	vec3 ambient = 0.17 * albedo;
	vec3 result = ambient;

	// for every light
	for (int i = 0; i < numLights; i++) {
		vec3 lightDir;
        if (lights[i].type == 0) { // directional
            lightDir = normalize(-lights[i].direction); // reverse
        }

		float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * albedo * lights[i].color;

        result += diffuse;
	}

	// to display the normal map in rgb, remap back to 0-1
//	FragColor = vec4(normal * 0.5 + 0.5, 1.0);


//	FragColor = vec4(albedo, 1.0);
	FragColor = vec4(result, 1.0);
}