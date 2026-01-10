#version 460 core
layout (location = 0) in vec3 aPos;         // vertex position
layout (location = 1) in vec3 aNormal;      // vertex normal
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aBitangent;
layout (location = 4) in vec2 aTexCoords;   // vertex texture coordinates

// stuff to pass to the fragment shader
out vec3 vFragPos;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;
out vec2 vTexCoords;

// transformation matrices
uniform mat4 model; // object to world space
uniform mat4 view;  // world to view space
uniform mat4 projection; // view to clip space

// TODO: handle multiple lights
// shadow map
//uniform mat4 lightSpaceMatrix;
//out vec4 vFragPosLightSpace;

void main() {
    mat3 M = mat3(model);

    // transform these attributes to world space
    // we can do this by using the model matrix
    vFragPos   = vec3(model * vec4(aPos, 1.0));
    vNormal    = M * aNormal;
    vTangent   = M * aTangent;
    vBitangent = M * aBitangent;
    vTexCoords = aTexCoords;

    // ORDER MATTERS
    gl_Position = projection * view * vec4(vFragPos, 1.0); // TO NDC
}