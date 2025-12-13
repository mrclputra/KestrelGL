#include "debug.h"
#include "ModelLoader.h"

#include <string>
#include <shader.h>

#include "lights/DirectionalLight.h"

// TODO: should I create some kind of scene factory?
//	it would be responsible for instantiating and setting up different scenes

void debugScene(Scene& scene) {
	logger.info("creating debug test scene");

	// ADD LIGHTS
	auto light = std::make_shared<DirectionalLight>(glm::vec3(0, 0, 0), glm::vec3(-1, -1, -2));
	scene.addLight(light);

	// ADD ENTITY
	auto shader = std::make_shared<Shader>(SHADER_DIR "model.vert", SHADER_DIR "model.frag");

	auto dragon = ModelLoader::load("assets/models/pbr/stanford_dragon_pbr/scene.gltf");
	dragon->shader = shader;
	if (!dragon) return;

	// set initial dragon transforms
	dragon->transform.scale = glm::vec3(0.08f, 0.08f, 0.08f);
	dragon->transform.position = glm::vec3(0, -3, 0);
	//dragon->transform.rotation = glm::vec3(0, 90, 0);

	scene.addObject(dragon);
}


// // OLD TESTING STUFF
//void debugScene(Scene& scene) {
//	logger.info("creating debug test scene");
//
//	auto shader = std::make_shared<Shader>(SHADER_DIR "model.vert", SHADER_DIR "model.frag");
//
//	const int cX = 3;
//	const int cY = 3;
//	const int cZ = 3;
//	const float spacing = 2.0f;
//
//	for (int x = 0; x < cX; ++x) {
//		for (int y = 0; y < cY; ++y) {
//			for (int z = 0; z < cZ; ++z) {
//				auto object = ModelLoader::load("assets/models/sphere.obj");
//				object->shader = shader;
//				if (!object) continue;
//
//				// set position
//				object->transform.position = glm::vec3(
//					x * spacing - ((cX - 1.0f) * spacing * 0.5f),
//					y * spacing - ((cY - 1.0f) * spacing * 0.5f),
//					z * spacing - ((cZ - 1.0f) * spacing * 0.5f)
//				);
//
//				scene.addObject(object);
//			}
//		}
//	}
//}