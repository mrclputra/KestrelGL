// ALL rendering operations should be done in this class
// The Renderer() class reads the Scene tree and performs rendering operations
#pragma once

#include "Scene.h"
#include <map>
#include "lights/DirectionalLight.h"

class Renderer {
public:
	void init(const Scene& scene);
	void render(const Scene& scene);

	unsigned int renderMode;
	std::multimap<float, std::shared_ptr<Object>, std::greater<float>> drawOrder;

	GLuint shadowArrayTexture;
	GLuint shadowArrayFBO;
	const unsigned int SHADOW_HEIGHT = 2048;
	const unsigned int SHADOW_WIDTH = 2048;

private:
	void renderObject(const Scene& scene, const Object& object);
	void renderShadowPass(const Scene& scene);
	void renderSkybox(const Scene& scene);

	Shader* depthShader = nullptr;

	//void initShadowArray(int numLayers);

	unsigned int brdfLUT = 0;
	unsigned int generateBRDFLUT();

	// todo: move this to a utility functions class or something
	void renderQuad();
};