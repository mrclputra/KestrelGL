#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <shader.h>
#include <memory>

// placeholder, ASSIMP logic goes here
// or maybe put it in a factory?

class Mesh {
public:
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
        glm::vec2 uv;
    };
    
    // constructors
    Mesh();
    //Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
    //Mesh(Mesh&& other) noexcept;
    //Mesh& operator=(Mesh&& other) noexcept;
    ~Mesh();

    void render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);

    // each mesh has its own shader
    std::shared_ptr<Shader> shader;

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;

    void cube(); // debug

    //// prevent copying
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
};