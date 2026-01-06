// ALL rendering operations should be done in this class
// The Renderer() class reads the Scene tree and performs rendering operations
#pragma once

#include "Scene.h"
#include "lights/DirectionalLight.h"
#include "lights/PointLight.h"

class Renderer {
public:
	void init(const Scene& scene);
	void render(const Scene& scene);
	void renderLightPass(const Scene& scene);

	unsigned int renderMode;

private:
	void uploadLights(const Scene& scene, Shader& shader);
	void renderObject(const Scene& scene, const Object& object);

	Shader* depthShader = nullptr;

	unsigned int brdfLUT = 0;
	unsigned int generateBRDFLUT();

	// todo: move this to a utility functions class or something
	void renderQuad();
};