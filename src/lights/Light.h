#pragma once

#include <glm/glm.hpp>
#include "components/Transform.h"

// light abstract base class
class Light {
public:
	Light(glm::vec3 pos) {
		transform.position = pos;
	}
	virtual ~Light() = default;

	Transform transform;
	glm::vec3 color{1.0f}; // RGB light color
};