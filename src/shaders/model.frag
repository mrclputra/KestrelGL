#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// material properties
uniform vec3 albedo;

void main() {
    //FragColor = vec4(albedo, 1.0); // yay
    FragColor = vec4(1.0); // yay
}