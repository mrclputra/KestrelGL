#pragma once

#include "Light.h"
#include <glm/glm.hpp>

class DirectionalLight : public Light {
public:
	explicit DirectionalLight(const glm::vec3& dir,
		const glm::vec3& col = glm::vec3(1.0f))
		: direction(glm::normalize(dir))
	{
		color = col;
	}

	void upload(Shader& shader, int index) const override {
		const std::string base = "dirLights[" + std::to_string(index) + "]";
		shader.setVec3(base + ".direction", direction);
		shader.setVec3(base + ".color", color);
	}

private:
	glm::vec3 direction;
};