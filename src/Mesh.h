#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>
#include <memory>

#include <shader.h>

class Mesh {
public:
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
        glm::vec2 uv;
    };

    struct Texture {
        unsigned int id;
        std::string type;
        std::string path;
    };
    
    // constructors
    Mesh() = default;
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
    ~Mesh();

    void upload();
    void render(const Shader& shader);

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
};