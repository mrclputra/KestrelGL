#include "debug.h"
#include "ModelLoader.h"

#include <string>
#include <shader.h>

// TODO: should I create some kind of scene factory?
//	it would be responsible for instantiating and setting up different scenes

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
//				scene.addEntity(object);
//			}
//		}
//	}
//}

void debugScene(Scene& scene) {
	logger.info("creating debug test scene");

	auto shader = std::make_shared<Shader>(SHADER_DIR "model.vert", SHADER_DIR "model.frag");

	auto object = ModelLoader::load("assets/models/pbr/stanford_dragon_pbr/scene.gltf");
	object->shader = shader;
	if (!object) return;

	// set transforms
	object->transform.scale = glm::vec3(0.08f, 0.08f, 0.08f);
	object->transform.position = glm::vec3(0,-3,0);
	//object->transform.rotation = glm::vec3(-90, 0, 0);

	scene.addEntity(object);
}