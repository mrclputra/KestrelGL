#pragma once

#include <glm/glm.hpp>
#include "lights/Light.h"

// directional light
class DirectionalLight : public Light {
public:
	DirectionalLight(glm::vec3 dir, glm::vec3 color) 
		: Light(), direction(dir), nearPlane(1.0f), farPlane(60.0f)
	{
		this->color = glm::normalize(color);
		this->transform.position = glm::vec3(0.0f); // no position
		updateLightSpaceMatrix();
	}

	glm::vec3 direction;
	glm::mat4 lightSpaceMatrix;
	int shadowArrayLayer = -1;

	// shadow map parameters
	float nearPlane;
	float farPlane;

	void updateLightSpaceMatrix() {
		// projection matrix controls the extents of the light map,
		// todo: will need to expose this as a parameter 
		const float extents = 10.0f;
		glm::mat4 lightProj = glm::ortho(-extents, extents, -extents, extents, nearPlane, farPlane);
		glm::vec3 lightPos = -direction * 40.0f;
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lightSpaceMatrix = lightProj * lightView;
	}
};