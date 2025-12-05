#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "mesh.h"

class Entity {
public:
    Entity(const std::string& name = "Entity");
    virtual ~Entity();

    virtual void update(float deltaTime);
    virtual void render();

    // transform methods
    void translate(const glm::vec3& translation);
    void rotate(float angle, const glm::vec3& axis);
    void scale(const glm::vec3& scale);

    glm::mat4 getTransform() const

        std::string name;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(0.0f);

    // mesh
    std::shared_ptr<Mesh> mesh;

private:
    // prevent copying
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
};