#include "Scene.h"
#include "ModelLoader.h"

#include <string>
#include <logger.h>

#include "lights/DirectionalLight.h"
#include "lights/PointLight.h"

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
	sphere0->material->albedo = glm::vec3(0.56f, 0.5f, 0.19f);
	sphere0->material->roughness = 0.27f;
	sphere0->material->metalness = 1.0f;
	scene.addObject(sphere0);

	auto sphere1 = ModelLoader::load("assets/models/cubeSphere.obj", "sphere_titanium");
	sphere1->transform.position = glm::vec3(2.0f, 0.0f, 0.0f);
	sphere1->material->albedo = glm::vec3(0.62f);
	sphere1->material->roughness = 0.07f;
	sphere1->material->metalness = 1.0f;
	scene.addObject(sphere1);

	auto sphere2 = ModelLoader::load("assets/models/cubeSphere.obj", "sphere_base");
	sphere2->transform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
	sphere2->material->albedo = glm::vec3(0.98f);
	sphere2->material->roughness = 0.12f;
	sphere2->material->metalness = 0.0f;
	scene.addObject(sphere2);

	//auto object = ModelLoader::load("assets/models/lion_crushing_a_serpent/scene.gltf", "lion");
	//object->transform.scale = glm::vec3(0.6f);
	//object->transform.position = glm::vec3(1.0f, 0.0f, -20.0f);
	//object->material->albedo = glm::vec3(0.98f);
	//scene.addObject(object);

	//auto cube = ModelLoader::load("assets/models/cube.obj", "cube");
	//cube->transform.scale = glm::vec3(0.7f);
	//cube->transform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
	//cube->material->albedo = glm::vec3(0.98f);
	//scene.addObject(cube);

	//auto dragon = ModelLoader::load("assets/models/stanford_dragon_pbr/scene.gltf", "stanford_dragon");
	//dragon->transform.scale = glm::vec3(0.07);
	//dragon->transform.position = glm::vec3(1.0f, -2.0f, -3.0f);
	//scene.addObject(dragon);
}