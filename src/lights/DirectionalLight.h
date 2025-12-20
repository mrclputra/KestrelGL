#pragma once

#include <glm/glm.hpp>
#include "lights/Light.h"

// directional light
class DirectionalLight : public Light {
public:
	DirectionalLight(glm::vec3 dir, glm::vec3 color) 
		: Light(), direction(dir), nearPlane(1.0f), farPlane(50.0f),
		SHADOW_WIDTH(2048), SHADOW_HEIGHT(2048) 
	{
		this->color = glm::normalize(color);
		this->transform.position = glm::vec3(0.0f); // no position
		initShadowMap();
	}

	glm::vec3 direction;
	glm::mat4 lightSpaceMatrix;

	// shadow map parameters
	GLuint depthMapFBO;
	GLuint depthMap;
	const unsigned int SHADOW_WIDTH;
	const unsigned int SHADOW_HEIGHT;
	float nearPlane;
	float farPlane;

	void updateLightSpaceMatrix() {
		glm::mat4 lightProj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, nearPlane, farPlane);
		glm::vec3 lightPos = -direction * 30.0f;
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lightSpaceMatrix = lightProj * lightView;
	}

private:
	// TODO: move this to the renderer
	// TODO: make use of GL_RENDER_2D_ARRAY to reduce vram usage
	void initShadowMap() {
		glGenFramebuffers(1, &depthMapFBO);

		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};