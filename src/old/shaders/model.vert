# version 330 core
layout (location = 0) in vec3 aPos;       // vertex position
layout (location = 1) in vec3 aNormal;    // vertex normals
layout (location = 2) in vec2 aTexCoords; // vertex texture coordinates
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// stuff to pass to the fragment shader
out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  TexCoords = aTexCoords;
  FragPos = vec3(model * vec4(aPos, 1.0));
  Normal = mat3(transpose(inverse(model))) * aNormal;
  Tangent = mat3(model) * aTangent; // transform to world space
  Bitangent = mat3(model) * aBitangent;

  gl_Position = projection * view * model * vec4(aPos, 1.0); // to NDC
}