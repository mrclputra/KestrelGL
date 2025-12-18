#pragma once

#include <glm/glm.hpp>
#include "lights/Light.h"

// directional light
class DirectionalLight : public Light {
public:
	DirectionalLight(glm::vec3 dir, glm::vec3 color) : Light(), direction(dir) {
		this->color = color;
		this->transform.position = glm::vec3(0.0f); // no position
	}

	glm::vec3 direction;
};