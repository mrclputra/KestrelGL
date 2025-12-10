#include "Entity.h"
#include <random> // temp

// constructor
Entity::Entity(const std::string& name) {
    this->name = name;
}

void Entity::update(float deltaTime) {
    // DEBUG rotation
    transform.rotate(glm::vec3(.5f, .5f, .5f) * deltaTime);
}

void Entity::render(const glm::mat4& view, const glm::mat4& projection) {

    shader->use();

    // TODO: bind textures

    shader->setMat4("model", transform.getModelMatrix());
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    shader->setVec3("color", glm::vec3(0.9f, 0.9f, 0.9f));

    // render mesh
    for (auto& mesh : meshes) {
        mesh.render();
    }

    // TODO: unbind textures
    // TODO: cleanup resources? this could be handled inside each respective component classes instead
}