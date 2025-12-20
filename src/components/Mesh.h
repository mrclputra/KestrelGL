#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>
#include <memory>

#include <logger.h>

class Mesh {
public:
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
        glm::vec2 uv;
    };

    // which textures from the object's texture vector this mesh uses
    std::vector<int> textureIndices;

    // constructors
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
    ~Mesh();

    void upload();
    void render();

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
};