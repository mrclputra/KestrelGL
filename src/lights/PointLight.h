#pragma once

#include <glm/glm.hpp>
#include "lights/Light.h"

// point light
class PointLight : public Light {
public:
	PointLight(glm::vec3 pos, glm::vec3 color) : Light(pos) {
		this->color = color;
	}
};