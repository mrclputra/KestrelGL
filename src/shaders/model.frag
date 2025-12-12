#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// material properties
//uniform vec3 albedo; // base color

uniform sampler2D albedoMap;
uniform sampler2D normalMap;

void main() {
    // FragColor = vec4(albedo, 1.0); // yay
    
    // FragColor = vec4(vec3(TexCoords, 1.0), 1.0);
    // FragColor = vec4(fract(TexCoords * 5.0), 0.0, 1.0);

    //FragColor = vec4(Normal * 0.5 + 0.5, 1.0);

    vec3 albedo = texture(albedoMap, TexCoords).rgb;
    vec3 N = normalize(Normal);

    FragColor = vec4(albedo, 1.0);
}