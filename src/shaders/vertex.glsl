# version 330 core
layout (location = 0) in vec3 aPos;       // vertex position
layout (location = 1) in vec3 aNormal;    // vertex normals
layout (location = 2) in vec2 aTexCoords; // vertex texture coordinates

// stuff to pass to the fragment shader
out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  TexCoords = aTexCoords;
  FragPos = vec3(model * vec4(aPos, 1.0));
  Normal = mat3(transpose(inverse(model))) * aNormal;

  gl_Position = projection * view * model * vec4(aPos, 1.0); // to NDC
}