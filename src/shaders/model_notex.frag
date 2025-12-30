// this shader is to be used in the event the object has no base textures

#version 460 core
out vec4 FragColor;

uniform vec3 albedo = vec3(1.0, 1.0, 1.0);

void main()
{
    FragColor = vec4(albedo, 1.0);
}
