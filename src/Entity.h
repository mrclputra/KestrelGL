#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <string>

#include <logger.h>

#include "mesh.h"

class Entity {
public:
    Entity(const std::string& name = "Entity");
    ~Entity();

    // per-frame logic
    void update(float deltaTime);
    void render(const glm::mat4& view, const glm::mat4& projection);

    // relative transformations
    void translate(const glm::vec3& delta);
    void rotate(const glm::vec3& delta);
    void rescale(const glm::vec3& factor);

    //
    glm::mat4 getModelMatrix() const;

    // public attributes
    std::string name;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    // meshes
    std::shared_ptr<Mesh> mesh;

private:
    // prevent copying
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
};