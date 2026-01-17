#include "Scene.h"
#include "ModelLoader.h"

#include <string>
#include <logger.h>

// TODO: should I create some kind of scene factory?
//	it would be responsible for instantiating and setting up different scenes

static void loadScene01(Scene& scene) {
	logger.info("loading debug scene 01");

	// OBJECTS
	auto sphere0 = ModelLoader::load("assets/models/cubeSphere.obj", scene.textures, "sphere0");
	sphere0->transform.position = glm::vec3(0.0f);
	sphere0->material->albedo = glm::vec4(0.56f, 0.5f, 0.19f, 1.0f);
	sphere0->material->roughness = 0.27f;
	sphere0->material->metalness = 1.0f;
	scene.addObject(sphere0);

	auto sphere1 = ModelLoader::load("assets/models/cubeSphere.obj", scene.textures, "sphere1");
	sphere1->transform.position = glm::vec3(2.0f, 0.0f, 0.0f);
	sphere1->material->albedo = glm::vec4(0.62f, 0.62f, 0.62f, 1.0f);
	sphere1->material->roughness = 0.07f;
	sphere1->material->metalness = 1.0f;
	scene.addObject(sphere1);

	auto sphere2 = ModelLoader::load("assets/models/cubeSphere.obj", scene.textures, "sphere2");
	sphere2->transform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
	sphere2->material->albedo = glm::vec4(0.98f, 0.98f, 0.98f, 0.5f);
	sphere2->material->roughness = 0.12f;
	sphere2->material->metalness = 0.0f;
	scene.addObject(sphere2);

	auto dragon = ModelLoader::load("assets/models/stanford_dragon_pbr/scene.gltf", scene.textures, "dragon");
	dragon->transform.scale = glm::vec3(0.07);
	dragon->transform.position = glm::vec3(1.0f, -2.0f, -3.0f);
	scene.addObject(dragon);

	//auto sponza = ModelLoader::load("assets/models/Sponza/sponza.obj", scene.textures);
	//sponza->transform.scale = glm::vec3(0.05);
	//sponza->transform.position = glm::vec3(0.0, -2.0, 0.0);
	//scene.addObject(sponza);

	auto sponza = ModelLoader::loadAsMultiple("assets/models/Sponza/sponza.obj", scene.textures);
	for (auto& part : sponza) {
		part->transform.scale = glm::vec3(0.05);
		part->transform.position = glm::vec3(0.0, -2.0, 0.0);
		scene.addObject(part);
	}
}