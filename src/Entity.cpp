#include "Entity.h"
#include <random> // temp

// constructor
Entity::Entity(const std::string& name, std::shared_ptr<Shader> shaderPtr)
    : name(name), shader(shaderPtr) {
    static std::mt19937 rng{ std::random_device{}() };
    static std::uniform_real_distribution<float> dist(0.0f, glm::two_pi<float>());
    rotation = glm::vec3(dist(rng), dist(rng), dist(rng));
}

void Entity::update(float deltaTime) {
    // DEBUG rotation
    rotation += glm::vec3(.5f, .5f, .5f) * deltaTime;
}

void Entity::render(const glm::mat4& view, const glm::mat4& projection) {

    shader->use();

    // TODO: bind textures if available

    shader->setMat4("model", getModelMatrix());
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    shader->setVec3("color", glm::vec3(0.9f, 0.9f, 0.9f));

    // maybe not need to pass shaders if mesh doesnt modify shader
    for (auto& mesh : meshes) {
        mesh->render(*shader);
    }

    // TODO: unbind textures
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