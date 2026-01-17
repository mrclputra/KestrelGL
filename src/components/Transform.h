#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform {
public:
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f); // degrees
	glm::vec3 scale = glm::vec3(1.0f);

    // cache
    mutable glm::mat4 modelMatrix = glm::mat4(1.0f);
    mutable bool isDirty = true;

    void translate(const glm::vec3& delta) { position += delta; isDirty = true; }
	void rotate(const glm::vec3& delta) { rotation += delta; isDirty = true; }
	void rescale(const glm::vec3& factor) { scale *= factor; isDirty = true; }

    // object space -> world space
    glm::mat4 getModelMatrix() const {
        if (isDirty) {
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, position);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0, 0.0, 0.0));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0, 1.0, 0.0));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0, 0.0, 1.0));
            modelMatrix = glm::scale(modelMatrix, scale);

            isDirty = false;
        }
        return modelMatrix;
    }
};