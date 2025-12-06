#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <shader.h>
#include <memory>

// placeholder, ASSIMP logic goes here
// or maybe put it in a factory?

class Mesh {
public:
    Mesh();
    virtual ~Mesh();

    virtual void render(const glm::mat4& model);

    // each mesh has its own shader
    std::shared_ptr<Shader> shader;

    // OpenGL stuff here, 
    // write to VBO, VAO, EBO from some kind of ASSIMP loader

private:
    unsigned int VAO = 0;
    unsigned int VBO = 0;

    void triangle(); // debug

    // prevent copying
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
};