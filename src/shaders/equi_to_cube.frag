#version 460 core
out vec4 FragColor;
in vec3 LocalPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 sampleSphere(vec3 v) {
	// atan2 and asin convert the 3D direction to spherical angles
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan; // scale to 0,1
	uv += 0.5;
	return uv;
}

void main() {
	vec2 uv = sampleSphere(normalize(LocalPos));
    vec3 color = texture(equirectangularMap, uv).rgb;

    FragColor = vec4(color, 1.0);
}