#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "shader.h"

class Material {
public:
	Material() = default;
	~Material() = default;

	// set shader uniforms
	[[deprecated("This class is incompatible with current engine architecture")]]
	void apply(Shader& shader) const;

	// material properties
	glm::vec3 albedo = glm::vec3(1.0f); // rgb base color
};