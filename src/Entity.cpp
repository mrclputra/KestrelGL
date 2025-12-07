#include "Entity.h"

Entity::Entity(const std::string& name) : name(name) {}
Entity::~Entity() {}

void Entity::update(float deltaTime) {

}

void Entity::render(const glm::mat4& view, const glm::mat4& projection) {
    glm::mat4 model = getModelMatrix();

	for (auto& m : meshes) {
		if (m) {
			// each mesh should handle its own opengl draw calls
			m->render(model, view, projection); 
		}
	}
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