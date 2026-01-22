// ALL rendering operations should be done in this class
// The Renderer() class reads the Scene tree and performs rendering operations

#pragma once

#include <map>

#include "Scene.h"
//#include "lights/DirectionalLight.h"

class Renderer {
public:
	void init(const Scene& scene);
	void render(const Scene& scene);

	//unsigned int renderMode;
	std::multimap<float, std::shared_ptr<Object>, std::greater<float>> drawOrder;

	// todo: shadow rendering parameters

private:
	void renderObject(const Scene& scene, const Object& transform);
	void renderSkybox(const Scene& scene);

	// to bake a depth map, we need to render a quad that fills the screen, 
	// will need to figure out how to better do this in the future
	//void renderQuad();
};