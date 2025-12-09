#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <string>

#include <logger.h>
#include <shader.h>

#include "mesh.h"

class Entity {
public:
    Entity() = default;
    Entity(const std::string& name = "Entity", std::shared_ptr<Shader> shaderPtr = nullptr);
    ~Entity() = default;

    // per-frame logic
    void update(float deltaTime);
    void render(const glm::mat4& view, const glm::mat4& projection);

    // relative transformations
    void translate(const glm::vec3& delta);
    void rotate(const glm::vec3& delta);
    void rescale(const glm::vec3& factor);
    glm::mat4 getModelMatrix() const;

    // public attributes
    std::string name;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    std::vector<std::shared_ptr<Mesh>> meshes;
    std::shared_ptr<Shader> shader = nullptr;
    //std::vector<std::shared_ptr<Texture>> textures;

private:
    // prevent copying
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
};