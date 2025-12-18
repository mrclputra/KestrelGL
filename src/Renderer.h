// ALL rendering operations should be done in this class
// The Renderer() class reads the Scene tree and performs rendering operations
#pragma once

#include "Scene.h"
#include "lights/DirectionalLight.h"
#include "lights/PointLight.h"

class Renderer {
public:
	void render(const Scene& scene);

private:
	// TODO: refactor this custom datatype
	struct ShaderLight {
		glm::vec3 position;
		glm::vec3 direction;
		glm::vec3 color;
		int type;
	};

	void uploadLights(const Scene& scene, Shader& shader);
	void renderObject(const Scene& scene, const Object& object);
};