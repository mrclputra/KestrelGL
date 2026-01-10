#include "Scene.h"
#include "ModelLoader.h"

#include <string>
#include <logger.h>

#include "lights/DirectionalLight.h"

// TODO: should I create some kind of scene factory?
//	it would be responsible for instantiating and setting up different scenes

// this is my test scene configuration
static void sphereScene(Scene& scene) {
	logger.info("creating spheres scene");

	// LIGHTS
	scene.addLight(std::make_shared<DirectionalLight>(glm::vec3(-1), glm::vec3(255, 254, 247)));

	// OBJECTS
	auto sphere0 = ModelLoader::load("assets/models/cubeSphere.obj", "sphere_gold");
	sphere0->transform.position = glm::vec3(0.0f);
	sphere0->material->albedo = glm::vec4(0.56f, 0.5f, 0.19f, 1.0f);
	sphere0->material->roughness = 0.27f;
	sphere0->material->metalness = 1.0f;
	scene.addObject(sphere0);

	auto sphere1 = ModelLoader::load("assets/models/cubeSphere.obj", "sphere_titanium");
	sphere1->transform.position = glm::vec3(2.0f, 0.0f, 0.0f);
	sphere1->material->albedo = glm::vec4(0.62f, 0.62f, 0.62f, 1.0f);
	sphere1->material->roughness = 0.07f;
	sphere1->material->metalness = 1.0f;
	scene.addObject(sphere1);

	auto sphere2 = ModelLoader::load("assets/models/cubeSphere.obj", "sphere_base");
	sphere2->transform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
	sphere2->material->albedo = glm::vec4(0.98f, 0.98f, 0.98f, 0.5f);
	sphere2->material->roughness = 0.12f;
	sphere2->material->metalness = 0.0f;
	scene.addObject(sphere2);

	auto dragon = ModelLoader::load("assets/models/stanford_dragon_pbr/scene.gltf", "stanford_dragon");
	dragon->transform.scale = glm::vec3(0.07);
	dragon->transform.position = glm::vec3(1.0f, -2.0f, -3.0f);
	scene.addObject(dragon);
}

static void khronos_spheres(Scene& scene) {
	logger.info("loading khronos spheres scene...");

	// light

	// model
	auto spheres = ModelLoader::load("assets/models/khronos_test_spheres/MetalRoughSpheres.gltf");
	scene.addObject(spheres);
}

static void sponzaScene(Scene& scene) {
	logger.info("loading sponza...");

	auto sponza = ModelLoader::load("assets/models/main_sponza/NewSponza_Main_glTF_003.gltf", "sponza");
	scene.addObject(sponza);
}