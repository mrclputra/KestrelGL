#include "Entity.h"

Entity::Entity(const std::string& name) : name(name) {
    logger.info("entity created: " + name);
}
Entity::~Entity() {}

void Entity::update(float deltaTime) {
    // DEBUG
    rotation += glm::vec3(.5f, .5f, .5f) * deltaTime;
}

void Entity::render(const glm::mat4& view, const glm::mat4& projection) {
    // render own mesh
    mesh->render(getModelMatrix(), view, projection);
}

// relative transforms
void Entity::translate(const glm::vec3& delta) { position += delta; }
void Entity::rotate(const glm::vec3& delta) { rotation += delta; }
void Entity::rescale(const glm::vec3& factor) { scale *= factor; }

glm::mat4 Entity::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);
    return model;
}