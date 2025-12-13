#pragma once

#include <glm/glm.hpp>
#include "shader.h"
//#include "Transform.h"

// light abstract base class
class Light {
public:
	virtual ~Light() = default;

	glm::vec3 color{1.0f}; // RGB

	// push this light into shader uniforms
	virtual void upload(Shader& shader, int index) const = 0;
};