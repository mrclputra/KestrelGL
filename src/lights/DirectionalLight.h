#pragma once

#include <glm/glm.hpp>
#include "components/Transform.h"
#include "lights/Light.h"

// directional light
class DirectionalLight : public Light {
public:
	DirectionalLight(glm::vec3 pos, glm::vec3 dir)
		: Light(pos), direction(dir) { }

	glm::vec3 direction;
};