#version 460 core
out vec4 FragColor;

// from the vertex shader
in vec3 vFragPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec2 vTexCoords;

// TODO: scene lighting input vectors here

// TODO: modify to follow gltf convention 
// textures; TODO: should I make this an array
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
	FragColor = vec4(normal * 0.5 + 0.5, 1.0); // to display, remap back to 0-1

//	FragColor = vec4(albedo, 1.0);
}