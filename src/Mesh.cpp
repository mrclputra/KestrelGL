#include "Mesh.h"

// triangle :D
static float triangleVertices[] = {
     0.0f,  0.5f, 0.0f,  // top
    -0.5f, -0.5f, 0.0f,  // bottom left
     0.5f, -0.5f, 0.0f   // bottom right
};

Mesh::Mesh() {
    triangle();
    
    // initialize shader instance
    shader = std::make_shared<Shader>(SHADER_DIR "/model.vert", SHADER_DIR "/model.frag");
}

Mesh::~Mesh() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}

void Mesh::triangle() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::render(const glm::mat4& model) {
    // call shader and pass uniforms
    if (shader) {
        shader->use();
        shader->setMat4("model", model);
        // TODO: pass more uniforms here

        shader->setVec3("color", glm::vec3(1.0f, 1.5f, 0.8f));
    }
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}