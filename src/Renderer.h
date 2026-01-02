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

	// TODO: check if storage is necessary
	//int screenWidth, screenHeight;

	unsigned int renderMode;

private:
	void uploadLights(const Scene& scene, Shader& shader);
	void renderObject(const Scene& scene, const Object& object);

	Shader* depthShader = nullptr;
};