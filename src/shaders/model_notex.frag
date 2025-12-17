#version 460 core
out vec4 FragColor;

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoords;

struct Light {
	vec3 position;
	vec3 direction;
	vec3 color;
	int type; // 0-directional, 1-point, 2-spot
};

#define MAX_LIGHTS 8
uniform Light lights[MAX_LIGHTS];
uniform int numLights;

uniform vec3 viewPos;
uniform vec3 albedo = vec3(0.98, 0.98, 0.98); // default color
uniform vec3 nullPink = vec3(1, 0, 1); 

void main() {
	vec3 normal = normalize(vNormal);
	vec3 viewDir = normalize(viewPos - vFragPos);
	
	vec3 ambient = 0.1 * albedo;
	vec3 result = ambient;
	
	// for every light
	for (int i = 0; i < numLights; i++) {
		vec3 lightDir = vec3(0.0);
		vec3 diffuse = vec3(0.0);
		
		if (lights[i].type == 0) {
			// directional
			lightDir = normalize(-lights[i].direction);
			float diff = max(dot(normal, lightDir), 0.0);
			diffuse = diff * albedo * lights[i].color;
		} else if (lights[i].type == 1) {
			// point
			lightDir = normalize(lights[i].position - vFragPos);
			float diff = max(dot(normal, lightDir), 0.0);
			float distance = length(lights[i].position - vFragPos);
			float attenuation = 1.0 / distance * 2.2;
			diffuse = diff * albedo * lights[i].color * attenuation;
		}
		
		// specular
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
		vec3 specular = 0.3 * spec * lights[i].color;
		
		result += diffuse + specular;
	}
	
//	FragColor = vec4(result, 1.0);
	FragColor = vec4(nullPink, 1.0);
}